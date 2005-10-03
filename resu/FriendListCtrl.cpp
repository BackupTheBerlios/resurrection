//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "emule.h"
#include "FriendListCtrl.h"
#include "friend.h"
#include "ClientDetailDialog.h"
#include "Addfriend.h"
#include "FriendList.h"
#include "emuledlg.h"
#include "ClientList.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "ListenSocket.h"
#include "MenuCmds.h"
#include "ChatWnd.h"
#include "log.h"
// MORPH START - Added by Commander, Friendlinks [emulEspa�a]
#include "ED2KLink.h"
// MORPH END - Added by Commander, Friendlinks [emulEspa�a]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CFriendListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CFriendListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
END_MESSAGE_MAP()

CFriendListCtrl::CFriendListCtrl()
{
}

CFriendListCtrl::~CFriendListCtrl()
{
}

void CFriendListCtrl::Init()
{
	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	SetName(_T("FriendListCtrl"));
// MORPH START - Added by Commander, Friendlinks [emulEspa�a]
	ModifyStyle(LVS_SINGLESEL,0);
	// MORPH END - Added by Commander, Friendlinks [emulEspa�a]
	RECT rcWindow;
	GetWindowRect(&rcWindow);
	InsertColumn(0, GetResString(IDS_QL_USERNAME), LVCFMT_LEFT, rcWindow.right - rcWindow.left - 4, 0);
	SetAllIcons();
	theApp.friendlist->SetWindow(this);
	LoadSettings();
}

void CFriendListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CFriendListCtrl::SetAllIcons()
{
	CImageList iml;
	iml.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	iml.SetBkColor(CLR_NONE);
	iml.Add(CTempIconLoader(_T("FriendNoClient")));
	iml.Add(CTempIconLoader(_T("FriendWithClient")));
	iml.Add(CTempIconLoader(_T("FriendConnected")));
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	HIMAGELIST himlOld = ApplyImageList(iml.Detach());
	if (himlOld)
		ImageList_Destroy(himlOld);
}

void CFriendListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_QL_USERNAME);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(0, &hdi);
	strRes.ReleaseBuffer();

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		UpdateFriend(i, (CFriend*)GetItemData(i));
}

void CFriendListCtrl::UpdateFriend(int iItem, const CFriend* pFriend)
{
	SetItemText(iItem, 0, pFriend->m_strName.IsEmpty() ? _T('(') + GetResString(IDS_UNKNOWN) + _T(')') : pFriend->m_strName);

	int iImage;
    if (!pFriend->GetLinkedClient())
		iImage = 0;
	else if (pFriend->GetLinkedClient()->socket && pFriend->GetLinkedClient()->socket->IsConnected())
		iImage = 2;
	else
		iImage = 1;
	SetItem(iItem,0,LVIF_IMAGE,0,iImage,0,0,0,0);
}

void CFriendListCtrl::AddFriend(const CFriend* pFriend)
{
	// avoid crash at shutdown 
           if (!theApp.emuledlg->IsRunning()) 
                   return; 
    // avoid crash at shutdown 

	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,GetItemCount(),pFriend->m_strName,0,0,0,(LPARAM)pFriend);
	if (iItem >= 0)
		UpdateFriend(iItem, pFriend);
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
}

void CFriendListCtrl::RemoveFriend(const CFriend* pFriend)
{
	// avoid crash at shutdown 
           if (!theApp.emuledlg->IsRunning()) 
                   return; 
    // avoid crash at shutdown 

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFriend;
	int iItem = FindItem(&find);
	if (iItem != -1)
		DeleteItem(iItem);
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
}

void CFriendListCtrl::RefreshFriend(const CFriend* pFriend)
{
	// avoid crash at shutdown 
           if (!theApp.emuledlg->IsRunning()) 
                   return; 
    // avoid crash at shutdown 

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFriend;
	int iItem = FindItem(&find);
	if (iItem != -1)
		UpdateFriend(iItem, pFriend);
}

void CFriendListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_FRIENDLIST), true);

	const CFriend* cur_friend = NULL;
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		cur_friend = (CFriend*)GetItemData(iSel);
		ClientMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
		ClientMenu.SetDefaultItem(MP_DETAIL);
	}

	ClientMenu.AppendMenu(MF_STRING, MP_ADDFRIEND, GetResString(IDS_ADDAFRIEND), _T("ADDFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
	ClientMenu.AppendMenu(MF_STRING | ((cur_friend==NULL || (cur_friend && cur_friend->GetLinkedClient() && !cur_friend->GetLinkedClient()->GetViewSharedFilesSupport())) ? MF_GRAYED : MF_ENABLED), MP_SHOWLIST, GetResString(IDS_VIEWFILES) , _T("VIEWFILES"));
		//KTS+
    CUpDownClient* pClientLinked = (cur_friend?cur_friend->GetLinkedClient():0);//LSD UNBAN
	ClientMenu.AppendMenu(MF_STRING | ((pClientLinked && pClientLinked->IsEd2kClient() && pClientLinked->IsBanned()) ? MF_ENABLED : MF_GRAYED), MP_UNBAN, GetResString(IDS_UNBAN));//LSD UNBAN
	pClientLinked = 0;//LSD UNBAN
	//KTS-
ClientMenu.AppendMenu(MF_SEPARATOR);
	ClientMenu.AppendMenu(MF_STRING | (theApp.IsEd2kFriendLinkInClipboard() ? MF_ENABLED : MF_GRAYED), MP_PASTE, GetResString(IDS_PASTE), _T("PASTELINK"));
	ClientMenu.AppendMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), MP_GETFRIENDED2KLINK, GetResString(IDS_GETFRIENDED2KLINK), _T("ED2KLINK"));
	ClientMenu.AppendMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), MP_GETHTMLFRIENDED2KLINK, GetResString(IDS_GETHTMLFRIENDED2KLINK), _T("ED2KLINK"));
	ClientMenu.AppendMenu(MF_SEPARATOR);
	ClientMenu.AppendMenu(MF_STRING, MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT"));
    ClientMenu.EnableMenuItem(MP_FRIENDSLOT, (cur_friend)?MF_ENABLED : MF_GRAYED);
	ClientMenu.CheckMenuItem(MP_FRIENDSLOT, (cur_friend && cur_friend->GetFriendSlot()) ? MF_CHECKED : MF_UNCHECKED);

	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CFriendListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	CFriend* cur_friend = NULL;
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) 
		cur_friend = (CFriend*)GetItemData(iSel);
	
	switch (wParam){
		case MP_MESSAGE:{
			if (cur_friend){
				if (cur_friend->GetLinkedClient())
					theApp.emuledlg->chatwnd->StartSession(cur_friend->GetLinkedClient());
				else{
					CUpDownClient* chatclient = new CUpDownClient(0,cur_friend->m_nLastUsedPort,cur_friend->m_dwLastUsedIP,0,0,true);
					chatclient->SetUserName(cur_friend->m_strName);
					chatclient->SetUserHash(cur_friend->m_abyUserhash);
					theApp.clientlist->AddClient(chatclient);
					theApp.emuledlg->chatwnd->StartSession(chatclient);
				}
			}
			break;
		}
		case MP_REMOVEFRIEND:{
			if (cur_friend){
				theApp.friendlist->RemoveFriend(cur_friend);
				// auto select next item after deleted one.
				if (iSel < GetItemCount()){
					SetSelectionMark(iSel);
					SetItemState(iSel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
			}
			break;
		}
		case MP_ADDFRIEND:{
			CAddFriend dialog2; 
			dialog2.DoModal();
			break;
		}
		case MPG_ALTENTER:
		case MP_DETAIL:
			if (cur_friend)
				ShowFriendDetails(cur_friend);
			break;
		case MP_SHOWLIST:
		{
			if (cur_friend){
				if (cur_friend->GetLinkedClient())
					cur_friend->GetLinkedClient()->RequestSharedFileList();
				else{
					CUpDownClient* newclient = new CUpDownClient(0,cur_friend->m_nLastUsedPort,cur_friend->m_dwLastUsedIP,0,0,true);
					newclient->SetUserName(cur_friend->m_strName);
					theApp.clientlist->AddClient(newclient);
					newclient->RequestSharedFileList();
				}
			}
			break;
		}
		case MP_FRIENDSLOT:
		{
			if (cur_friend){
				bool IsAlready;
                IsAlready = cur_friend->GetFriendSlot();
				theApp.friendlist->RemoveAllFriendSlots();
				if( !IsAlready )
                    cur_friend->SetFriendSlot(true);
                else
                    cur_friend->SetFriendSlot(false);
			
			break;
		}

case MP_PASTE:
			{
				CString link = theApp.CopyTextFromClipboard();
				link.Trim();
				if ( link.IsEmpty() )
					break;

				try{
					CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(link);

					if (pLink && pLink->GetKind() == CED2KLink::kFriend )
					{
						// Better with dynamic_cast, but no RTTI enabled in the project
						CED2KFriendLink* pFriendLink = static_cast<CED2KFriendLink*>(pLink);
						uchar userHash[16];
						pFriendLink->GetUserHash(userHash);

						if ( ! theApp.friendlist->IsAlreadyFriend(userHash) )
							theApp.friendlist->AddFriend(userHash, 0U, 0U, 0U, 0U, pFriendLink->GetUserName(), 1U);
						else
						{
							CString msg;
							msg.Format(GetResString(IDS_USER_ALREADY_FRIEND), pFriendLink->GetUserName());
							AddLogLine(true, msg);
						}
					}
				}
				catch(CString strError){
					AfxMessageBox(strError);
				}
			}
			break;
		case MP_GETFRIENDED2KLINK:
			{
				CString sCompleteLink;
				if ( cur_friend && cur_friend->m_dwHasHash )
				{
					CString sLink;
					CED2KFriendLink friendLink(cur_friend->m_strName, cur_friend->m_abyUserhash);
					friendLink.GetLink(sLink);
					if ( !sCompleteLink.IsEmpty() )
						sCompleteLink.Append(_T("\r\n"));
					sCompleteLink.Append(sLink);
				}

				if ( !sCompleteLink.IsEmpty() )
					theApp.CopyTextToClipboard(sCompleteLink);
			
			break;
			}
		case MP_GETHTMLFRIENDED2KLINK:
			{
				CString sCompleteLink;

				if ( cur_friend && cur_friend->m_dwHasHash )
				{
					CString sLink;
					CED2KFriendLink friendLink(cur_friend->m_strName, cur_friend->m_abyUserhash);
					friendLink.GetLink(sLink);
					sLink = _T("<a href=\"") + sLink + _T("\">") + StripInvalidFilenameChars(cur_friend->m_strName, true) + _T("</a>");
					if ( !sCompleteLink.IsEmpty() )
						sCompleteLink.Append(_T("\r\n"));
					sCompleteLink.Append(sLink);
				}

				if ( !sCompleteLink.IsEmpty() )
					theApp.CopyTextToClipboard(sCompleteLink);
			}
			break;
		}
	}
	return true;
}

void CFriendListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) 
		ShowFriendDetails((CFriend*)GetItemData(iSel));
	*pResult = 0;
}

void CFriendListCtrl::ShowFriendDetails(const CFriend* pFriend)
{
	if (pFriend){
		if (pFriend->GetLinkedClient()){
			CClientDetailDialog dialog(pFriend->GetLinkedClient());
			dialog.DoModal();
		}
		else{
			CAddFriend dlg;
			dlg.m_pShowFriend = const_cast<CFriend*>(pFriend);
			dlg.DoModal();
		}
	}
}

BOOL CFriendListCtrl::PreTranslateMessage(MSG* pMsg) 
{
   	if ( pMsg->message == 260 && pMsg->wParam == 13 && GetAsyncKeyState(VK_MENU)<0 ) {
		PostMessage(WM_COMMAND, MPG_ALTENTER, 0);
		return TRUE;
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE)
		PostMessage(WM_COMMAND, MP_REMOVEFRIEND, 0);
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_INSERT)
		PostMessage(WM_COMMAND, MP_ADDFRIEND, 0);

	return CMuleListCtrl::PreTranslateMessage(pMsg);
}

void CFriendListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Determine ascending based on whether already sorted on this column
	int iSortItem = GetSortItem();
	bool bOldSortAscending = GetSortAscending();
	bool bSortAscending = (iSortItem != pNMListView->iSubItem) ? true : !bOldSortAscending;

	// Item is column clicked
	iSortItem = pNMListView->iSubItem;

	// Sort table
	SetSortArrow(iSortItem, bSortAscending);
	SortItems(SortProc, MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));

	*pResult = 0;
}

int CFriendListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CFriend* item1 = (CFriend*)lParam1;
	CFriend* item2 = (CFriend*)lParam2; 
	if (item1 == NULL || item2 == NULL)
		return 0;

	int iResult;
	switch (LOWORD(lParamSort))
	{
		case 0:
			iResult = CompareLocaleStringNoCase(item1->m_strName, item2->m_strName);
			break;
		default:
			return 0;
	}
	if (HIWORD(lParamSort))
		iResult = -iResult;
	return iResult;
}

void CFriendListCtrl::UpdateList()
{
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
	SortItems(SortProc, MAKELONG(GetSortItem(), (GetSortAscending() ? 0 : 0x0001)));
}
// MORPH START - Added by Commander, Friendlinks [emulEspa�a]
bool CFriendListCtrl::AddEmfriendsMetToList(const CString& strFile)
{
	ShowWindow(SW_HIDE);
	bool ret = theApp.friendlist->AddEmfriendsMetToList(strFile);
	theApp.friendlist->ShowFriends();
	UpdateList();
	ShowWindow(SW_SHOW);
	return ret;
}
// MORPH END - Added by Commander, Friendlinks [emulEspa�a]


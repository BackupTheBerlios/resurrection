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
#include "ClientListCtrl.h"
#include "otherfunctions.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "KademliaWnd.h"
#include "ClientList.h"
#include "emuledlg.h"
#include "FriendList.h"
#include "TransferWnd.h"
#include "MemDC.h"
#include "UpDownClient.h"
#include "ClientCredits.h"
#include "ListenSocket.h"
#include "ChatWnd.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/net/KademliaUDPListener.h"
// IP-to-Country +
#include "IP2Country.h" 
// IP-to-Country -

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CClientListCtrl

IMPLEMENT_DYNAMIC(CClientListCtrl, CMuleListCtrl)

CClientListCtrl::CClientListCtrl()
	: CListCtrlItemWalk(this)
{
}

void CClientListCtrl::Init()
{
	SetName(_T("ClientListCtrl"));
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	InsertColumn(0,GetResString(IDS_QL_USERNAME),LVCFMT_LEFT,150,0);
	InsertColumn(1,GetResString(IDS_CL_UPLOADSTATUS),LVCFMT_LEFT,150,1);
	InsertColumn(2,GetResString(IDS_CL_TRANSFUP),LVCFMT_LEFT,150,2);
	InsertColumn(3,GetResString(IDS_CL_DOWNLSTATUS),LVCFMT_LEFT,150,3);
	InsertColumn(4,GetResString(IDS_CL_TRANSFDOWN),LVCFMT_LEFT,150,4);
	InsertColumn(5,GetResString(IDS_CD_CSOFT),LVCFMT_LEFT,150,5);
	InsertColumn(6,GetResString(IDS_CONNECTED),LVCFMT_LEFT,150,6);
	CString coltemp;
	coltemp=GetResString(IDS_CD_UHASH);coltemp.Remove(':');
	InsertColumn(7,coltemp,LVCFMT_LEFT,150,7);
	InsertColumn(8,GetResString(IDS_MODVERSION_LABEL),LVCFMT_LEFT,100,8); //Spe64 Modversion
	
	SetAllIcons();
	Localize();
	LoadSettings();
	SetSortArrow();
	SortItems(SortProc, GetSortItem()+ (GetSortAscending()? 0:100));
}

CClientListCtrl::~CClientListCtrl()
{
}

void CClientListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CClientListCtrl::SetAllIcons()
{
	imagelist.DeleteImageList();
	imagelist.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	imagelist.SetBkColor(CLR_NONE);
	imagelist.Add(CTempIconLoader(_T("ClientEDonkey")));
	imagelist.Add(CTempIconLoader(_T("ClientCompatible")));
	imagelist.Add(CTempIconLoader(_T("Friend")));
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkey")));
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	imagelist.Add(CTempIconLoader(_T("ClientShareaza")));
	imagelist.Add(CTempIconLoader(_T("Server")));
	imagelist.Add(CTempIconLoader(_T("ClientAMule")));
	imagelist.Add(CTempIconLoader(_T("ClientLPhant")));
   // Mondgott :: Show RedSmurfIconOnClientDetect
	imagelist.Add(CTempIconLoader(_T("RedSmurf"))); //icon  9
    // Mondgott :: Show RedSmurfIconOnClientDetect
	imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
}

void CClientListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

	if(pHeaderCtrl->GetItemCount() != 0) {
		CString strRes;

		strRes = GetResString(IDS_QL_USERNAME);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(0, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_CL_UPLOADSTATUS);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(1, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_CL_TRANSFUP);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(2, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_CL_DOWNLSTATUS);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(3, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_CL_TRANSFDOWN);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(4, &hdi);
		strRes.ReleaseBuffer();

		strRes=GetResString(IDS_CD_CSOFT);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(5, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_CONNECTED);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(6, &hdi);
		strRes.ReleaseBuffer();

		strRes=GetResString(IDS_CD_UHASH);strRes.Remove(':');
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(7, &hdi);
		strRes.ReleaseBuffer();
//Spe6+4 Modversion
		strRes=GetResString(IDS_MODVERSION_LABEL);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(8, &hdi);
		strRes.ReleaseBuffer();
	}
}

void CClientListCtrl::ShowKnownClients()
{
	DeleteAllItems();
	int iItemCount = 0;
	for(POSITION pos = theApp.clientlist->list.GetHeadPosition(); pos != NULL;){
		const CUpDownClient* cur_client = theApp.clientlist->list.GetNext(pos);
		int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,iItemCount,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)cur_client);
		Update(iItem);
		iItemCount++;
	}
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Clients, iItemCount);
}

void CClientListCtrl::AddClient(const CUpDownClient* client)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (thePrefs.IsKnownClientListDisabled())
		return;

	int iItemCount = GetItemCount();
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,iItemCount,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)client);
	Update(iItem);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Clients, iItemCount+1);
}

void CClientListCtrl::RemoveClient(const CUpDownClient* client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	sint32 result = FindItem(&find);
	if (result != -1){
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Clients);
	}
}

void CClientListCtrl::RefreshClient(const CUpDownClient* client)
{
	// There is some type of timing issue here.. If you click on item in the queue or upload and leave
	// the focus on it when you exit the cient, it breaks on line 854 of emuleDlg.cpp.. 
	// I added this IsRunning() check to this function and the DrawItem method and
	// this seems to keep it from crashing. This is not the fix but a patch until
	// someone points out what is going wrong.. Also, it will still assert in debug mode..
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	sint16 result = FindItem(&find);
	if (result != -1)
		Update(result);
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CClientListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	//MORPH START - Added by SiRoB, Don't draw hidden Rect
	RECT clientRect;
	GetClientRect(&clientRect);
	RECT cur_rec = lpDrawItemStruct->rcItem;
	if (cur_rec.top >= clientRect.bottom || cur_rec.bottom <= clientRect.top)
		return;
	//MORPH END   - Added by SiRoB, Don't draw hidden Rect

	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		if (bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());
	const CUpDownClient* client = (CUpDownClient*)lpDrawItemStruct->itemData;
	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	//CRect cur_rec(lpDrawItemStruct->rcItem);
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;
	CString Sbuffer;
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			cur_rec.right += GetColumnWidth(iColumn);
			switch(iColumn){
				case 0:{
					uint8 image;
					if (client->IsFriend())
						image = 2;
                            // Mondgott :: Show RedSmurfIconOnClientDetect
					        else if (client->GetRedSmurfClient())
					            image = 9;
                            // Mondgott :: Show RedSmurfIconOnClientDetect
					else if (client->GetClientSoft() == SO_EDONKEYHYBRID)
						image = 4;
					else if (client->GetClientSoft() == SO_MLDONKEY)
						image = 3;
					else if (client->GetClientSoft() == SO_SHAREAZA)
						image = 5;
					else if (client->GetClientSoft() == SO_URL)
						image = 6;
					else if (client->GetClientSoft() == SO_AMULE)
						image = 7;
					else if (client->GetClientSoft() == SO_LPHANT)
						image = 8;
					else if (client->ExtProtocolAvailable())
						image = 1;
					else
						image = 0;

					POINT point = {cur_rec.left, cur_rec.top+1};
					imagelist.Draw(dc,image, point, ILD_NORMAL | ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED) ? INDEXTOOVERLAYMASK(1) : 0));
					if (client->GetUserName()==NULL)
						Sbuffer.Format(_T("<%s> (%s)"), client->GetCountryName(), GetResString(IDS_UNKNOWN));
					else
						Sbuffer.Format(_T("<%s> %s"), client->GetCountryName(), client->GetUserName()); 
// IP-to-Country +
					if(theApp.ip2country->ShowCountryFlag()){
						cur_rec.left+=20;
						POINT point2= {cur_rec.left,cur_rec.top+1};
						int index = client->GetCountryFlagIndex();
						theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, index , point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
					}
					// IP-to-Country -
					cur_rec.left +=20;
					dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
					cur_rec.left -=20;
// IP-to-Country +
					if(theApp.ip2country->ShowCountryFlag()){
						cur_rec.left-=20;
					}
					// IP-to-Country -
					break;
				}
				case 1:{
					Sbuffer = client->GetUploadStateDisplayString();
					break;
				}
				case 2:{
					if(client->credits)
						Sbuffer = CastItoXBytes(client->credits->GetUploadedTotal(), false, false);
					else
						Sbuffer.Empty();
					break;
				}
				case 3:{
					Sbuffer = client->GetDownloadStateDisplayString();
					break;
				}
				case 4:{
					if(client->credits)
						Sbuffer = CastItoXBytes(client->credits->GetDownloadedTotal(), false, false);
					else
						Sbuffer.Empty();
					break;
				}
				case 5:{
					//Sbuffer = client->GetClientSoftVer();
					Sbuffer = client->DbgGetFullClientSoftVer(); // Maella -Support for tag ET_MOD_VERSION 0x55
					if (Sbuffer.IsEmpty())
						Sbuffer = GetResString(IDS_UNKNOWN);
					break;
				}
				case 6:{
					if(client->socket){
						if(client->socket->IsConnected()){
							Sbuffer = GetResString(IDS_YES);
							break;
						}
					}
					Sbuffer = GetResString(IDS_NO);
					break;
				}
				case 7:
					Sbuffer = md4str(client->GetUserHash());
					break;
               //Spe64 ModVersion
 case 8:
                    Sbuffer = client->GetClientModVer();
                    break;
			}
			if( iColumn != 0)
				dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}

	// draw rectangle around selected item(s)
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc.FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if(bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}

	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

BEGIN_MESSAGE_MAP(CClientListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
END_MESSAGE_MAP()

// CClientListCtrl message handlers
	
void CClientListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	const CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
	ClientMenu.SetDefaultItem(MP_DETAIL);
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
			//-->Spe64	
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED),MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND")); // Remove Friend
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED),MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT")); //-->FriendSlot

		if (GetSelectionMark() != (-1))	{ 
		CUpDownClient* client = (CUpDownClient*)GetItemData(GetSelectionMark());
		if (client && client->IsFriend()) {
			ClientMenu.CheckMenuItem(MP_FRIENDSLOT, ((client->GetFriendSlot())?MF_CHECKED : MF_UNCHECKED) );
			ClientMenu.EnableMenuItem(MP_ADDFRIEND,MF_GRAYED); 
		}
		else {
			ClientMenu.EnableMenuItem(MP_FRIENDSLOT,MF_GRAYED);
			ClientMenu.EnableMenuItem(MP_REMOVEFRIEND,MF_GRAYED); 
		}
	}
	//--->Spe64
	if (Kademlia::CKademlia::isRunning() && !Kademlia::CKademlia::isConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CClientListCtrl::OnCommand(WPARAM wParam,LPARAM lParam )
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		switch (wParam){
			case MP_SHOWLIST:
				client->RequestSharedFileList();
				break;
			case MP_MESSAGE:
				theApp.emuledlg->chatwnd->StartSession(client);
				break;
			case MP_ADDFRIEND:
				if (theApp.friendlist->AddFriend(client))
					Update(iSel);
				break;
			case MP_UNBAN:
				if (client->IsBanned()){
					client->UnBan();
					Update(iSel);
				}
//Spe64 		
	case MP_REMOVEFRIEND: {
				if (client && client->IsFriend())
					theApp.friendlist->RemoveFriend(client->m_Friend);
				break;
				}
				break;
			case MPG_ALTENTER:
			case MP_DETAIL:{
				CClientDetailDialog dialog(client, this);
				dialog.DoModal();
				break;
			}
			case MP_BOOT:
				if (client->GetKadPort())
					Kademlia::CKademlia::bootstrap(ntohl(client->GetIP()), client->GetKadPort());
				break;
 //Spe64           
case MP_FRIENDSLOT: 
				{
					if (client)
					{
					bool IsAlready;				
					IsAlready = client->GetFriendSlot();
					//theApp.friendlist->RemoveAllFriendSlots();//LSD
					if( !IsAlready )
							client->SetFriendSlot(true);
					else
							client->SetFriendSlot(false);
					}
					break;
}
		}
	}
	return true;
} 

void CClientListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	bool sortAscending = (GetSortItem()!= pNMListView->iSubItem) ? true : !GetSortAscending();

	// Sort table
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0:100), 100);
	SetSortArrow(pNMListView->iSubItem, sortAscending);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0:100));

	*pResult = 0;
}

int CClientListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CUpDownClient* item1 = (CUpDownClient*)lParam1;
	const CUpDownClient* item2 = (CUpDownClient*)lParam2;
	
	int iResult=0;
	switch (lParamSort) {
	    case 0:
			if (item1->GetUserName() && item2->GetUserName())
				iResult=CompareLocaleStringNoCase(item1->GetUserName(), item2->GetUserName());
			else if (!item1->GetUserName() && !item2->GetUserName())
				iResult=0;
			else {
				// place clients with no usernames at bottom
				if (!item1->GetUserName())
					iResult=1;
				else
					iResult=-1;
			}
			break;
	    case 100:
			if (item1->GetUserName() && item2->GetUserName())
				iResult=CompareLocaleStringNoCase(item2->GetUserName(), item1->GetUserName());
			else if (!item1->GetUserName() && !item2->GetUserName())
				iResult=0;
			else {
				// place clients with no usernames at bottom
				if (!item1->GetUserName())
					iResult=1;
				else
					iResult=-1;
			}
			break;

		case 1:
		    iResult=item1->GetUploadState() - item2->GetUploadState();
			break;
	    case 101:
		    iResult=item2->GetUploadState() - item1->GetUploadState();
			break;

		case 2:
			if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item1->credits->GetUploadedTotal(), item2->credits->GetUploadedTotal());
			else if (!item1->credits)
			    iResult=1;
			else
				iResult=-1;
			break;
	    case 102:
			if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item2->credits->GetUploadedTotal(), item1->credits->GetUploadedTotal());
			else if (!item1->credits)
				iResult=1;
			else
			    iResult=-1;
			break;

		case 3:
		    if (item1->GetDownloadState() == item2->GetDownloadState()) {
			    if (item1->IsRemoteQueueFull() && item2->IsRemoteQueueFull())
				    iResult=0;
			    else if (item1->IsRemoteQueueFull())
				    iResult=1;
			    else if (item2->IsRemoteQueueFull())
				    iResult=-1;
			    else
				    iResult=0;
		    } else
				iResult=item1->GetDownloadState() - item2->GetDownloadState();
			break;
	    case 103:
		    if (item2->GetDownloadState() == item1->GetDownloadState()) {
			    if (item2->IsRemoteQueueFull() && item1->IsRemoteQueueFull())
				    iResult=0;
			    else if (item2->IsRemoteQueueFull())
				    iResult=1;
			    else if (item1->IsRemoteQueueFull())
				    iResult=-1;
			    else
				    iResult=0;
		    } else
				iResult=item2->GetDownloadState() - item1->GetDownloadState();
			break;

		case 4:
		    if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item1->credits->GetDownloadedTotal(), item2->credits->GetDownloadedTotal());
		    else if (!item1->credits)
			    iResult=1;
		    else
				iResult=-1;
			break;
	    case 104:
		    if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item2->credits->GetDownloadedTotal(), item1->credits->GetDownloadedTotal());
		    else if (!item1->credits)
			    iResult=1;
		    else
			    iResult=-1;
			break;

		case 5:
		    if (item1->GetClientSoft() == item2->GetClientSoft())
			    iResult=item2->GetVersion() - item1->GetVersion();
		    else 
				iResult=item1->GetClientSoft() - item2->GetClientSoft();
			break;
	    case 105:
		    if (item1->GetClientSoft() == item2->GetClientSoft())
			    iResult=item1->GetVersion() - item2->GetVersion();
		    else
				iResult=item2->GetClientSoft() - item1->GetClientSoft();
			break;

		case 6:
		    if (item1->socket && item2->socket)
			    iResult=item1->socket->IsConnected() - item2->socket->IsConnected();
		    else if (!item1->socket)
			    iResult=-1;
		    else
			    iResult=1;
			break;
	    case 106:
		    if (item1->socket && item2->socket)
			    iResult=item2->socket->IsConnected() - item1->socket->IsConnected();
		    else if (!item2->socket)
			    iResult=-1;
		    else
			    iResult=1;
			break;

		case 7:
		    iResult=memcmp(item1->GetUserHash(), item2->GetUserHash(), 16);
			break;
	    case 107:
		    iResult=memcmp(item2->GetUserHash(), item1->GetUserHash(), 16);
			break;
//Spe64 ModVersion
        case 8:
		case 108: 
		    if(item1->GetClientModVer() && item2->GetClientModVer())
			    iResult = _tcsicmp(item1->GetClientModVer(), item2->GetClientModVer());
                if(lParamSort == 108) {
                    iResult = -iResult;
                }
		    else if(item1->GetClientSoftVer())
			    iResult =  1;
		    else
			    iResult =  -1;
            break;

		default:
			iResult=0;
	}

	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->clientlistctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}

	return iResult;
}

void CClientListCtrl::ShowSelectedUserDetails()
{
	POINT point;
	::GetCursorPos(&point);
	CPoint p = point; 
    ScreenToClient(&p); 
    int it = HitTest(p); 
    if (it == -1)
		return;

	SetItemState(-1, 0, LVIS_SELECTED);
	SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	SetSelectionMark(it);   // display selection mark correctly!

	CUpDownClient* client = (CUpDownClient*)GetItemData(GetSelectionMark());
	if (client){
		CClientDetailDialog dialog(client, this);
		dialog.DoModal();
	}
}

void CClientListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult) {
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) {
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		if (client){
			CClientDetailDialog dialog(client, this);
			dialog.DoModal();
		}
	}
	*pResult = 0;
}

void CClientListCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if (theApp.emuledlg->IsRunning()){
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, no *NOT* put any time consuming code here in.

		if (pDispInfo->item.mask & LVIF_TEXT){
			const CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(pDispInfo->item.lParam);
			if (pClient != NULL){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (pClient->GetUserName() != NULL && pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, pClient->GetUserName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
		}
	}
	*pResult = 0;
}

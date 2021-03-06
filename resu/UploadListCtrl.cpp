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
#include "UploadListCtrl.h"
#include "TransferWnd.h"
#include "otherfunctions.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "KademliaWnd.h"
#include "emuledlg.h"
#include "friendlist.h"
#include "MemDC.h"
#include "KnownFile.h"
#include "SharedFileList.h"
#include "UpDownClient.h"
#include "ClientCredits.h"
#include "ChatWnd.h"
#include "kademlia/kademlia/Kademlia.h"
#include "kademlia/net/KademliaUDPListener.h"
#include "UploadQueue.h"
#include "ToolTipCtrlX.h"
// IP-to-Country +
#include "IP2Country.h"
// IP-to-Country -
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CUploadListCtrl

IMPLEMENT_DYNAMIC(CUploadListCtrl, CMuleListCtrl)

CUploadListCtrl::CUploadListCtrl()
	: CListCtrlItemWalk(this)
{
	m_tooltip = new CToolTipCtrlX;
}

void CUploadListCtrl::Init()
{
	SetName(_T("UploadListCtrl"));
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		m_tooltip->SubclassWindow(tooltip->m_hWnd);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}

	InsertColumn(0,GetResString(IDS_QL_USERNAME),LVCFMT_LEFT,150,0);
	InsertColumn(1,GetResString(IDS_FILE),LVCFMT_LEFT,275,1);
	InsertColumn(2,GetResString(IDS_DL_SPEED),LVCFMT_LEFT,60,2);
	InsertColumn(3,GetResString(IDS_DL_TRANSF),LVCFMT_LEFT,65,3);
	InsertColumn(4,GetResString(IDS_WAITED),LVCFMT_LEFT,60,4);
	InsertColumn(5,GetResString(IDS_UPLOADTIME),LVCFMT_LEFT,60,5);
	InsertColumn(6,GetResString(IDS_STATUS),LVCFMT_LEFT,110,6);
	InsertColumn(7,GetResString(IDS_UPSTATUS),LVCFMT_LEFT,100,7);
	InsertColumn(8,GetResString(IDS_SOFTWARE_LABEL),LVCFMT_LEFT,100,9);
    InsertColumn(9,_T("RQR/DL"),LVCFMT_LEFT,60,10);	// bobo RQR DL
	InsertColumn(10,GetResString(IDS_LSD_TOTAL_UP_DL),LVCFMT_LEFT,100,9); //LSD Total UP-DL
	SetAllIcons();
	Localize();
	LoadSettings();

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100));
}

CUploadListCtrl::~CUploadListCtrl()
{
	delete m_tooltip;
}

void CUploadListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CUploadListCtrl::SetAllIcons()
{
	imagelist.DeleteImageList();
	imagelist.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	imagelist.SetBkColor(CLR_NONE);
	imagelist.Add(CTempIconLoader(_T("ClientEDonkey")));
	imagelist.Add(CTempIconLoader(_T("ClientCompatible")));
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));
	imagelist.Add(CTempIconLoader(_T("ClientCompatiblePlus")));
	imagelist.Add(CTempIconLoader(_T("Friend")));
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkey")));
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));
	imagelist.Add(CTempIconLoader(_T("ClientShareaza")));
	imagelist.Add(CTempIconLoader(_T("ClientShareazaPlus")));
	imagelist.Add(CTempIconLoader(_T("ClientAMule")));
	imagelist.Add(CTempIconLoader(_T("ClientAMulePlus")));
	imagelist.Add(CTempIconLoader(_T("ClientLPhant")));
	imagelist.Add(CTempIconLoader(_T("ClientLPhantPlus")));
	// Mondgott :: Show RedSmurfIconOnClientDetect
	imagelist.Add(CTempIconLoader(_T("RedSmurf"))); //15
    // Mondgott :: Show RedSmurfIconOnClientDetect
	imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
}

void CUploadListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_QL_USERNAME);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(0, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_FILE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(1, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_SPEED);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(2, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_TRANSF);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(3, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_WAITED);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(4, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_UPLOADTIME);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(5, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_STATUS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(6, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_UPSTATUS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(7, &hdi);
	strRes.ReleaseBuffer();


	// >>> bobo RQR DL
	strRes = _T("RQR/Speed"); 
	hdi.pszText = strRes.GetBuffer(); 
	pHeaderCtrl->SetItem(9, &hdi); 
	strRes.ReleaseBuffer();
	// <<< bobo RQR DL

	//LSD Total UP-DL
	strRes = GetResString(IDS_LSD_TOTAL_UP_DL);//LSD
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(10, &hdi);//LSD
	strRes.ReleaseBuffer();
	//LSD Total UP-DL


}

void CUploadListCtrl::AddClient(const CUpDownClient* client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	int iItemCount = GetItemCount();
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,iItemCount,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)client);
	Update(iItem);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Uploading, iItemCount+1);
}

void CUploadListCtrl::RemoveClient(const CUpDownClient* client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	sint32 result = FindItem(&find);
	if (result != -1){
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Uploading);
	}
}

void CUploadListCtrl::RefreshClient(const CUpDownClient* client)
{
	// There is some type of timing issue here.. If you click on item in the queue or upload and leave
	// the focus on it when you exit the cient, it breaks on line 854 of emuleDlg.cpp
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

void CUploadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	//MORPH START - Added by SiRoB, Don't draw hidden Rect
	/*RECT clientRect;
	GetClientRect(&clientRect);
	RECT cur_rec = lpDrawItemStruct->rcItem;
	if (cur_rec.top >= clientRect.bottom || cur_rec.bottom <= clientRect.top)
		return;*/
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
	CRect cur_rec(lpDrawItemStruct->rcItem);
	COLORREF crOldTextColor = dc.SetTextColor(m_crWindowText);

// upload colors
	if(thePrefs.IsUploadColor()) //2bc-colors-check
	{
		int qr1 = client->GetRemoteQueueRank();

		if ( qr1 > 0 )
		{ 
			if ( qr1 < 101 ) //Green 				
				crOldTextColor = dc.SetTextColor(RGB(2,185,12)); 
			else if ( qr1 < 501 ) //Bright green 				
				crOldTextColor = dc.SetTextColor(RGB(121,209,33)); 
			else if ( qr1 < 1001 ) //Yellow 				
				crOldTextColor = dc.SetTextColor(RGB(169,192,16));// [Aireoreion] 
			else if ( qr1 < 2001 ) //Orange 				
				crOldTextColor = dc.SetTextColor(RGB(255,178,0)); 
			else if ( qr1 < 3001 ) //Dark orange 				
				crOldTextColor = dc.SetTextColor(RGB(216,145,48)); 
			else //Red 				
				crOldTextColor = dc.SetTextColor(RGB(208,13,2)); 
		}
		else
		{ 
			if ( client->IsRemoteQueueFull() ) //Dark blue 				
				crOldTextColor = dc.SetTextColor(RGB(71,94,95)); 
			else if ( client->GetDownloadDatarate() > 0 ) //Blue 
				crOldTextColor = dc.SetTextColor(RGB(0,0,211)); 
			else //Black 
				crOldTextColor = dc.SetTextColor(RGB(0,0,0));          
		} 
    }
// upload colors
	else if(client->GetSlotNumber() > theApp.uploadqueue->GetActiveUploadsCount()) 
		dc.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;
	CString Sbuffer;


	for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if (!IsColumnHidden(iColumn))
		{
			cur_rec.right += GetColumnWidth(iColumn);
			switch (iColumn)
			{
				case 0:{
					uint8 image;
					if (client->IsFriend())
						image = 4;
					    // Mondgott :: Show RedSmurfIconOnClientDetect
					    else if (client->GetRedSmurfClient())
						        image = 15;
					    // Mondgott :: Show RedSmurfIconOnClientDetect
					else if (client->GetClientSoft() == SO_EDONKEYHYBRID){
						if (client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 8;
						else
							image = 7;
					}
					else if (client->GetClientSoft() == SO_MLDONKEY){
						if (client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 6;
						else
							image = 5;
					}
					else if (client->GetClientSoft() == SO_SHAREAZA){
						if(client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 10;
						else
							image = 9;
					}
					else if (client->GetClientSoft() == SO_AMULE){
						if(client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 12;
						else
							image = 11;
					}
					else if (client->GetClientSoft() == SO_LPHANT){
						if(client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 14;
						else
							image = 13;
					}
					else if (client->ExtProtocolAvailable()){
						if(client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 3;
						else
							image = 1;
					}
					else{
						if (client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 2;
						else
							image = 0;
					}

					POINT point = {cur_rec.left, cur_rec.top+1};
					imagelist.Draw(dc,image, point, ILD_NORMAL | ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED) ? INDEXTOOVERLAYMASK(1) : 0));
					Sbuffer = client->GetUserName();
// IP-to-Country +
					CString tempStr;
					tempStr.Format(_T("%s%s"), client->GetCountryName(), Sbuffer);
					Sbuffer = tempStr;

					if(theApp.ip2country->ShowCountryFlag() ){
						cur_rec.left+=20;
						POINT point2= {cur_rec.left,cur_rec.top+1};
						theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, client->GetCountryFlagIndex(), point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
					}
					// IP-to-Country -
					cur_rec.left += 20;
					dc.DrawText(Sbuffer, Sbuffer.GetLength(), &cur_rec, DLC_DT_TEXT);
					cur_rec.left -= 20;
// IP-to-Country +
					if(theApp.ip2country->ShowCountryFlag() ){
						cur_rec.left-=20;
					}
					// IP-to-Country -
					break;
				}
				case 1:
					if (file)
						Sbuffer = file->GetFileName();
					else
						Sbuffer = _T("?");
					break;
				case 2:
					Sbuffer = CastItoXBytes(client->GetDatarate(), false, true);
					break;
				case 3:
					// NOTE: If you change (add/remove) anything which is displayed here, update also the sorting part..
					if (thePrefs.m_bExtControls)
						Sbuffer.Format( _T("%s (%s)"), CastItoXBytes(client->GetSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionPayloadUp(), false, false));
					else
						Sbuffer = CastItoXBytes(client->GetSessionUp(), false, false);
					break;
				case 4:
					if (client->HasLowID())
						Sbuffer.Format(_T("%s (%s)"), CastSecondsToHM(client->GetWaitTime()/1000), GetResString(IDS_IDLOW));
					else
						Sbuffer = CastSecondsToHM(client->GetWaitTime()/1000);
					break;
				case 5:
					Sbuffer = CastSecondsToHM(client->GetUpStartTimeDelay()/1000);
					break;
				case 6:
					Sbuffer = client->GetUploadStateDisplayString();
					break;
				case 7:
					cur_rec.bottom--;
					cur_rec.top++;
					client->DrawUpStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
					cur_rec.bottom++;
					cur_rec.top--;
					break;
				case 8:
					Sbuffer = client->DbgGetFullClientSoftVer();
						//Telp Super Release
					if (file && file->IsReleaseFile())
						Sbuffer += _T(" [") + GetResString(IDS_RELEASEFILE) + _T("]");
					//Telp Super Release
                                        break;
					// >>> bobo RQR DL

				case 9: 
					if (client->GetDownloadDatarate() > 0)
						Sbuffer.Format(_T("%.2f KB/s"),(float)client->GetDownloadDatarate()/1024);
					else if (client->GetRemoteQueueRank()) 
						Sbuffer.Format(_T("QR: %u"),client->GetRemoteQueueRank()); 
					else 
						Sbuffer.Format(_T("Unknown")); 
					break;
			// <<< bobo RQR DL
	case 10: //LSD Total UP/DL
					if (client->Credits()){
						Sbuffer.Format( _T("%s/%s"),// %.1f",
							CastItoXBytes((float)client->Credits()->GetUploadedTotal()),
							CastItoXBytes((float)client->Credits()->GetDownloadedTotal()));
					}
					else{
						Sbuffer = _T("?/?");
					}
					break;

			}
			if (iColumn != 7 && iColumn != 0)
				dc.DrawText(Sbuffer, Sbuffer.GetLength(), &cur_rec, DLC_DT_TEXT);
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}

	//draw rectangle around selected item(s)
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

		if (bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}

	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

BEGIN_MESSAGE_MAP(CUploadListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
END_MESSAGE_MAP()

// CUploadListCtrl message handlers
void CUploadListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	const CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
//Telp Start Slot Focus
	ClientMenu.AppendMenu(MF_STRING , MP_SLOTFOCUS, _T("Disable Slotfocus"), _T("CANCEL"));
	if(thePrefs.GetSlotFocus())
		ClientMenu.CheckMenuItem(MP_SLOTFOCUS,MF_CHECKED);
	else
		ClientMenu.CheckMenuItem(MP_SLOTFOCUS,MF_UNCHECKED);
	ClientMenu.SetDefaultItem(MP_SLOTFOCUS);
ClientMenu.AppendMenu(MF_SEPARATOR);
//Telp End Slot Focus
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
	ClientMenu.SetDefaultItem(MP_DETAIL);
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
		//KTS+ Whois
			ClientMenu.AppendMenu(MF_SEPARATOR);
			ClientMenu.AppendMenu(MF_STRING,MP_WHOIS2, _T("WHOIS query (basic)"), _T("SEARCHPARAMS") );
			ClientMenu.SetDefaultItem(MP_WHOIS2);
			ClientMenu.AppendMenu(MF_STRING,MP_WHOIS, _T("WHOIS query (more info)"), _T("SEARCHRESULTS") );
			ClientMenu.SetDefaultItem(MP_WHOIS);
			//KTS- Whois
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
	//Spe64	
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED),MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND")); //RemoveFriend
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED),MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT")); //FreindSlot

	if (GetSelectionMark() != (-1))	{ 
		CUpDownClient* client = (CUpDownClient*)GetItemData(GetSelectionMark());
		if (client && client->IsFriend()) {
			ClientMenu.CheckMenuItem(MP_FRIENDSLOT, ((client->GetFriendSlot())?MF_CHECKED : MF_UNCHECKED) );
			ClientMenu.EnableMenuItem(MP_ADDFRIEND,MF_GRAYED); 
		}
		else {
			ClientMenu.EnableMenuItem(MP_FRIENDSLOT,MF_GRAYED);
			ClientMenu.EnableMenuItem(MP_REMOVEFRIEND,MF_GRAYED); //-->Matrik [LSD]
		}
	}
//Spe64	
	if (Kademlia::CKademlia::isRunning() && !Kademlia::CKademlia::isConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CUploadListCtrl::OnCommand(WPARAM wParam,LPARAM lParam ){
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		switch (wParam){
			case MP_SHOWLIST:
				client->RequestSharedFileList();
				break;
			   //KTS+ Whois
			case MP_WHOIS:{
				TCHAR address[256];
				_tcscpy(address, _T("http://www.whois.sc/"));
				_tcscat(address, ipstr(client->GetConnectIP()));
				ShellExecute(NULL, NULL, address, NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
				break;
						  }
			case MP_WHOIS2:{
				TCHAR address[256];
				_tcscpy(address, _T("http://www.searchbug.com/peoplefinder/location-by-ip-address.aspx?ipaddress="));
				_tcscat(address, ipstr(client->GetConnectIP()));
				ShellExecute(NULL, NULL, address, NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
				break;
						   }
			case MP_MESSAGE:
				theApp.emuledlg->chatwnd->StartSession(client);
				break;
			case MP_ADDFRIEND:
				if (theApp.friendlist->AddFriend(client))
					Update(iSel);
				break;
//Spe64		
	case MP_REMOVEFRIEND: {
				if (client && client->IsFriend())
					theApp.friendlist->RemoveFriend(client->m_Friend);
				break;
				}
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
		//Telp Start Slot Focus
	if(wParam == MP_SLOTFOCUS){
		if(thePrefs.GetSlotFocus())
			thePrefs.SetSlotFocus(false);
		else
			thePrefs.SetSlotFocus(true);
	}
	//Telp End Slot Focus
	return true;
}


void CUploadListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// if it's a second click on the same column then reverse the sort order,
	// otherwise sort the new column in ascending order.

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	bool sortAscending = (GetSortItem() != pNMListView->iSubItem) ? true : !GetSortAscending();

	// Sort table
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0:100));
	SetSortArrow(pNMListView->iSubItem, sortAscending);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0:100));

	*pResult = 0;
}

int CUploadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CUpDownClient* item1 = (CUpDownClient*)lParam1;
	const CUpDownClient* item2 = (CUpDownClient*)lParam2;

	int iResult=0;
	switch(lParamSort){
		case 0: 
			if(item1->GetUserName() && item2->GetUserName())
				iResult=CompareLocaleStringNoCase(item1->GetUserName(), item2->GetUserName());
			else if(item1->GetUserName())
				iResult=1;
			else
				iResult=-1;
			break;
		case 100:
			if(item1->GetUserName() && item2->GetUserName())
				iResult=CompareLocaleStringNoCase(item2->GetUserName(), item1->GetUserName());
			else if(item2->GetUserName())
				iResult=1;
			else
				iResult=-1;
			break;
		case 1: {
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
				iResult=CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}
		case 101:{
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
				iResult=CompareLocaleStringNoCase(file2->GetFileName(), file1->GetFileName());
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}
		case 2: 
			iResult=CompareUnsigned(item1->GetDatarate(), item2->GetDatarate());
			break;
		case 102:
			iResult=CompareUnsigned(item2->GetDatarate(), item1->GetDatarate());
			break;

		case 3: 
			iResult=CompareUnsigned(item1->GetSessionUp(), item2->GetSessionUp());
			if (iResult == 0 && thePrefs.m_bExtControls) {
				iResult = CompareUnsigned(item1->GetQueueSessionPayloadUp(), item2->GetQueueSessionPayloadUp());
			}
			break;
		case 103: 
			iResult=CompareUnsigned(item2->GetSessionUp(), item1->GetSessionUp());
			if (iResult == 0 && thePrefs.m_bExtControls) {
				iResult = CompareUnsigned(item2->GetQueueSessionPayloadUp(), item1->GetQueueSessionPayloadUp());
			}
			break;

		case 4: 
			iResult=item1->GetWaitTime() - item2->GetWaitTime();
			break;
		case 104: 
			iResult=item2->GetWaitTime() - item1->GetWaitTime();
			break;

		case 5: 
			iResult=item1->GetUpStartTimeDelay() - item2->GetUpStartTimeDelay();
			break;
		case 105: 
			iResult=item2->GetUpStartTimeDelay() - item1->GetUpStartTimeDelay();
			break;

		case 6: 
			iResult=item1->GetUploadState() - item2->GetUploadState();
			break;
		case 106: 
			iResult=item2->GetUploadState() - item1->GetUploadState();
			break;

		case 7:
			iResult=item1->GetUpPartCount() - item2->GetUpPartCount();
			break;
		case 107: 
			iResult=item2->GetUpPartCount() - item1->GetUpPartCount();
			break;
//Spe64+ add column
        case 8:
			iResult=CompareUnsigned(item1->GetSlotNumber(), item2->GetSlotNumber());
            break;
        case 108:
			iResult=CompareUnsigned(item2->GetSlotNumber(), item1->GetSlotNumber());
            break;

        case 9:
		case 109: 
			if(item1->DbgGetFullClientSoftVer() && item2->DbgGetFullClientSoftVer()) {
				iResult = _tcsicmp(item1->DbgGetFullClientSoftVer(), item2->DbgGetFullClientSoftVer());
				if(lParamSort == 109) {
						iResult = -iResult;
				}
			} else if(item1->DbgGetFullClientSoftVer())
				iResult =  1;
			else
				iResult =  -1;

			break;

		// >>> bobo RQR DL
		case 10: 
			return item1->GetDownloadDatarate() - item2->GetRemoteQueueRank(); 
		case 110: 
			return item2->GetRemoteQueueRank() - item1->GetDownloadDatarate();
		// <<< bobo RQR DL
	case 11:  //LSD Total UP-DL
			if (item2->Credits() && item1->Credits())
				iResult = item2->Credits()->GetUploadedTotal() - item1->Credits()->GetUploadedTotal();
			else if (!item2->Credits())
				iResult = -1;
			else if (!item1->Credits())
				iResult = 1;
			break;
		case 11+100:  //LSD Total UP-DL
			if (item2->Credits() && item1->Credits())
				iResult = item2->Credits()->GetDownloadedTotal() - item1->Credits()->GetDownloadedTotal();
			else if (!item2->Credits())
				iResult = -1;
			else if (!item1->Credits())
				iResult = 1;
			break;
		default:
			iResult=0;
			break;
	}
	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->uploadlistctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}

	return iResult;

}

void CUploadListCtrl::ShowSelectedUserDetails()
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

void CUploadListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		if (client){
			CClientDetailDialog dialog(client, this);
			dialog.DoModal();
		}
	}
	*pResult = 0;
}

void CUploadListCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
						if (pClient->GetUserName() && pDispInfo->item.cchTextMax > 0){
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

void CUploadListCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);

	if (pGetInfoTip->iSubItem == 0)
	{
		LVHITTESTINFO hti = {0};
		::GetCursorPos(&hti.pt);
		ScreenToClient(&hti.pt);
		if (SubItemHitTest(&hti) == -1 || hti.iItem != pGetInfoTip->iItem || hti.iSubItem != 0){
			// don' show the default label tip for the main item, if the mouse is not over the main item
			if ((pGetInfoTip->dwFlags & LVGIT_UNFOLDED) == 0 && pGetInfoTip->cchTextMax > 0 && pGetInfoTip->pszText[0] != '\0')
				pGetInfoTip->pszText[0] = '\0';
			return;
		}

		const CUpDownClient* client = (CUpDownClient*)GetItemData(pGetInfoTip->iItem);
		if (client && pGetInfoTip->pszText && pGetInfoTip->cchTextMax > 0)
		{
			CString info;

			CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			// build info text and display it
			info.Format(GetResString(IDS_USERINFO), client->GetUserName());
			if (file)
			{
				info += GetResString(IDS_SF_REQUESTED) + _T(" ") + CString(file->GetFileName()) + _T("\n");
				CString stat;
				stat.Format(GetResString(IDS_FILESTATS_SESSION)+GetResString(IDS_FILESTATS_TOTAL),
							file->statistic.GetAccepts(), file->statistic.GetRequests(), CastItoXBytes(file->statistic.GetTransferred(), false, false),
							file->statistic.GetAllTimeAccepts(), file->statistic.GetAllTimeRequests(), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false) );
				info += stat;
			}
			else
			{
				info += GetResString(IDS_REQ_UNKNOWNFILE);
			}

			_tcsncpy(pGetInfoTip->pszText, info, pGetInfoTip->cchTextMax);
			pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
		}
	}
	*pResult = 0;
}

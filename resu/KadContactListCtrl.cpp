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
#include "KademliaWnd.h"
#include "KadContactListCtrl.h"
#include "Ini2.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
//KTS+ IP to Country
#include "IP2Country.h" 
#include "MemDC.h"
#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)
//KTS- IP to Country
//KTS+ Whois
#include "MenuCmds.h"
//KTS- Whois

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CONContactListCtrl

enum ECols
{
	colIP=0,//KTS IP to Country
	colID,
	colType,
	colDistance
};

IMPLEMENT_DYNAMIC(CKadContactListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CKadContactListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_DESTROY()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CKadContactListCtrl::CKadContactListCtrl()
{
	SetGeneralPurposeFind(true);
	SetName(_T("ONContactListCtrl"));
}

CKadContactListCtrl::~CKadContactListCtrl()
{
}

void CKadContactListCtrl::Init()
{
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	InsertColumn(colIP,_T("IP"),LVCFMT_LEFT,100);//KTS IP to Country
	InsertColumn(colID,GetResString(IDS_ID),LVCFMT_LEFT,100);
	InsertColumn(colType,GetResString(IDS_TYPE) ,LVCFMT_LEFT,50);
	InsertColumn(colDistance,GetResString(IDS_KADDISTANCE),LVCFMT_LEFT,50);
	SetAllIcons();
	Localize();

	LoadSettings();
	int iSortItem = GetSortItem();
	bool bSortAscending = GetSortAscending();

	SetSortArrow(iSortItem, bSortAscending);
	SortItems(SortProc, MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));
}

void CKadContactListCtrl::SaveAllSettings()
{
	SaveSettings();
}

void CKadContactListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CKadContactListCtrl::SetAllIcons()
{
	//KTS+ IP to Country
	imagelist.DeleteImageList();
	imagelist.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	imagelist.SetBkColor(CLR_NONE);
	imagelist.Add(CTempIconLoader(_T("Contact0")));
	imagelist.Add(CTempIconLoader(_T("Contact1")));
	imagelist.Add(CTempIconLoader(_T("Contact2")));
	imagelist.Add(CTempIconLoader(_T("Contact3")));
	imagelist.Add(CTempIconLoader(_T("Contact4")));
	//KTS- IP to Country
}

void CKadContactListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	for (int icol=0;icol< pHeaderCtrl->GetItemCount() ;icol++) {
		switch (icol) {
			//KTS+ IP to Country
			case colIP: strRes = _T("IP");break;//KTS IP to Country
			case colID: strRes = GetResString(IDS_ID); break;
			case colType: strRes = GetResString(IDS_TYPE); break;
			case colDistance: strRes = GetResString(IDS_KADDISTANCE); break;
			//KTS+ IP to Country
		}
	
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(icol, &hdi);
		strRes.ReleaseBuffer();
	}

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		UpdateContact(i, (Kademlia::CContact*)GetItemData(i), true);
}
	
void CKadContactListCtrl::UpdateContact(int iItem, const Kademlia::CContact* contact, bool bLocalize)
{
	CString id;
	if (!bLocalize) // update the following fields only if really needed (it's quite expensive to always update them)
	{
		contact->getClientID(&id);
		SetItemText(iItem,colID,id);

		id.Format(_T("%i"),contact->getType());
		SetItemText(iItem,colType,id);

		contact->getDistance(&id);
		SetItemText(iItem,colDistance,id);

		SetItem(iItem,0,LVIF_IMAGE,0,contact->getType()>4?4:contact->getType(),0,0,0,0);
	}
}

void CKadContactListCtrl::UpdateKadContactCount()
{
	CString id;
	id.Format(_T("%s (%i)"), GetResString(IDS_KADCONTACTLAB), GetItemCount());
	theApp.emuledlg->kademliawnd->GetDlgItem(IDC_KADCONTACTLAB)->SetWindowText(id);
}

bool CKadContactListCtrl::ContactAdd(const Kademlia::CContact* contact)
{
	bool bResult = false;
	try
	{
		ASSERT( contact != NULL );
		int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,GetItemCount(),NULL,0,0,0,(LPARAM)contact);
		if (iItem >= 0)
		{
			bResult = true;
	//		Trying to update all the columns causes one of the connection freezes in win98
	//		ContactRef(contact);
			// If it still doesn't work under Win98, uncomment the '!afxData.bWin95' term
			if (!afxData.bWin95 && iItem >= 0)
				UpdateContact(iItem, contact);
			UpdateKadContactCount();
		}
	}
	catch(...){ASSERT(0);}
	return bResult;
}

void CKadContactListCtrl::ContactRem(const Kademlia::CContact* contact)
{
	try
	{
		ASSERT( contact != NULL );
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)contact;
		int iItem = FindItem(&find);
		if (iItem != -1)
		{
			DeleteItem(iItem);
			UpdateKadContactCount();
		}
	}
	catch(...){ASSERT(0);}
}

void CKadContactListCtrl::ContactRef(const Kademlia::CContact* contact)
{
	try
	{
		ASSERT( contact != NULL );
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)contact;
		int iItem = FindItem(&find);
		if (iItem != -1)
			UpdateContact(iItem, contact);
	}
	catch(...){ASSERT(0);}
}

BOOL CKadContactListCtrl::OnCommand(WPARAM wParam,LPARAM lParam)
{
int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		Kademlia::CContact* client = (Kademlia::CContact*)GetItemData(iSel);
		switch (wParam){
			//KTS+ Whois
			case MP_WHOIS:{
				TCHAR address[256];
				_tcscpy(address, _T("http://www.whois.sc/"));
				_tcscat(address, ipstr(client->getIPAddress()));
				ShellExecute(NULL, NULL, address, NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
				break;
						  }
			case MP_WHOIS2:{
				TCHAR address[256];
				_tcscpy(address, _T("http://www.searchbug.com/peoplefinder/location-by-ip-address.aspx?ipaddress="));
				_tcscat(address, ipstr(client->getIPAddress()));
				ShellExecute(NULL, NULL, address, NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
				break;
						   }
						   //KTS- Whois
		}
	}
	return TRUE;
}

void CKadContactListCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Determine ascending based on whether already sorted on this column
	int iSortItem = GetSortItem();
	bool bOldSortAscending = GetSortAscending();
	bool bSortAscending = (iSortItem != pNMListView->iSubItem) ? true : !bOldSortAscending;

	// Item is column clicked
	iSortItem = pNMListView->iSubItem;

	// Sort table
	UpdateSortHistory(MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));
	SetSortArrow(iSortItem, bSortAscending);
	SortItems(SortProc, MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));

	*pResult = 0;
}

int CKadContactListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	Kademlia::CContact* item1 = (Kademlia::CContact*)lParam1;
	Kademlia::CContact* item2 = (Kademlia::CContact*)lParam2; 
	if((item1 == NULL) || (item2 == NULL))
		return 0;

	int iResult;
	switch(LOWORD(lParamSort))
	{
		case colID:
		{
			Kademlia::CUInt128 i1;
			Kademlia::CUInt128 i2;
			item1->getClientID(&i1);
			item2->getClientID(&i2);
			iResult = i1.compareTo(i2);
			break;
		}
		case colType:
			iResult = item1->getType() - item2->getType();
			break;
		case colDistance:
		{
			Kademlia::CUInt128 distance1, distance2;
			item1->getDistance(&distance1);
			item2->getDistance(&distance2);
			iResult = distance1.compareTo(distance2);
			break;
		}
		default:
			return 0;
	}
	if (HIWORD(lParamSort))
		iResult = -iResult;
	return iResult;
}//KTS+ IP to Country
void CKadContactListCtrl::UpdateContact(int iItem, Kademlia::CContact* contact)
{
		if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)contact;
	sint16 result = FindItem(&find);
	if (result != -1)
		Update(result);
}

void CKadContactListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if( !theApp.emuledlg->IsRunning() )
		return;
	if (!lpDrawItemStruct->itemData)
		return;
	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this ) || (GetStyle() & LVS_SHOWSELALWAYS));
	if( (lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED )){
		if(bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());
	Kademlia::CContact* contact = (Kademlia::CContact*)lpDrawItemStruct->itemData;
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	RECT cur_rec = lpDrawItemStruct->rcItem;
	COLORREF crOldTextColor = dc.SetTextColor(m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
}	else
		iOldBkMode = OPAQUE;

	CString Sbuffer;
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;

	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			cur_rec.right += GetColumnWidth(iColumn);
			switch(iColumn){
				case colIP:{
					uint8 image;
					image = 0;
					//Tant que les contactes sont pas en liste l'initialisation de l'IPCountry se fera à l'affichage
					//Quand ils seront en liste : 
					// - Initialisation dans le(s) constructeur(s)
					// - Implementation de UpdateAllContact (cf refresh all server)
					// - Implementation de ResetIP2Country qui evidera le refreshwindows "hard" dans iptocountry
					contact->m_structServerCountry = theApp.ip2country->GetCountryFromIP(contact->m_ip); //KTS
					contact->getIPAddress(&Sbuffer);
					CString tempStr;
					tempStr.Format(_T("%s %s"), contact->GetCountryName(), Sbuffer);
					Sbuffer = tempStr;
					if(theApp.ip2country->ShowCountryFlag()){
						POINT point2= {cur_rec.left,cur_rec.top+1};
						theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, contact->GetCountryFlagIndex(), point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
					}
					cur_rec.left +=20;
					dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
					cur_rec.left -=20;
					break;
				}
				case colID:{
					POINT point = {cur_rec.left, cur_rec.top+1};
					switch (contact->getType()) {
						case 0:{
							imagelist.Draw(dc, 0, point, ILD_NORMAL);
							break;
}						case 1:{
							imagelist.Draw(dc, 1, point, ILD_NORMAL);
							break;
						}
						case 2:{
							imagelist.Draw(dc, 2, point, ILD_NORMAL);
							break;
						}
						case 3:{
							imagelist.Draw(dc, 3, point, ILD_NORMAL);
							break;
						}
						case 4:{
							imagelist.Draw(dc, 4, point, ILD_NORMAL);
							break;
						}
					}
					cur_rec.left +=20;
					contact->getClientID(&Sbuffer);
					break;
			    }
				case colType:{
					Sbuffer.Format(_T("%i"),contact->getType());
					break;
			   }
			case colDistance:{
					contact->getDistance(&Sbuffer);
					break;
				}
			}
			if(iColumn != 0 )
				dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec, DT_LEFT);
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}
	//draw rectangle around selected item(s)
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;
		outline_rec.top--;
		outline_rec.bottom++;
		dc->FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;
		if(bCtrlFocused)
			dc->FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc->FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}
//KTS- IP to Country
//KTS+ Whois
void CKadContactListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	//Kademlia::CContact* client = (iSel != -1) ? (Kademlia::CContact*)GetItemData(iSel) : NULL;//fix

	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(_T("KadContact"), true);
	ClientMenu.AppendMenu(MF_STRING,MP_WHOIS2, _T("WHOIS query (basic)"), _T("SEARCHPARAMS") );
	ClientMenu.SetDefaultItem(MP_WHOIS2);
	ClientMenu.AppendMenu(MF_STRING,MP_WHOIS, _T("WHOIS query (more info)"), _T("SEARCHRESULTS") );
	ClientMenu.SetDefaultItem(MP_WHOIS);

	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}
//KTS- Whois

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
#include "emuledlg.h"
#include "SharedFilesCtrl.h"
#include "OtherFunctions.h"
#include "ResizableLib/ResizableSheet.h"
#include "KnownFile.h"
#include "MapKey.h"
#include "SharedFileList.h"
#include "MemDC.h"
#include "PartFile.h"
#include "MenuCmds.h"
#include "SharedFilesWnd.h"
#include "Opcodes.h"
#include "InputBox.h"
#include "WebServices.h"
#include "TransferWnd.h"
#include "ClientList.h"
#include "UpDownClient.h"
#include "Collection.h"
#include "CollectionCreateDialog.h"
#include "CollectionViewDialog.h"
#include "FileDetailDialog.h"
#include "SharedDirsTreeCtrl.h"
#include "SearchParams.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
//KTS-
// WebCache ////////////////////////////////////////////////////////////////////////////////////
#include "Preferences.h" //JP webcache release



//////////////////////////////////////////////////////////////////////////////
// CSharedFilesCtrl

IMPLEMENT_DYNAMIC(CSharedFilesCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CSharedFilesCtrl, CMuleListCtrl)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CSharedFilesCtrl::CSharedFilesCtrl()
	: CListCtrlItemWalk(this)
{
	MEMSET(&sortstat, 0, sizeof(sortstat));
	nAICHHashing = 0;
	m_pDirectoryFilter = NULL;
}

CSharedFilesCtrl::~CSharedFilesCtrl()
{
}

void CSharedFilesCtrl::Init()
{
	SetName(_T("SharedFilesCtrl"));
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	ModifyStyle(LVS_SINGLESEL,0);

	InsertColumn(0, GetResString(IDS_DL_FILENAME) ,LVCFMT_LEFT,250,0);
	InsertColumn(1,GetResString(IDS_DL_SIZE),LVCFMT_LEFT,100,1);
	InsertColumn(2,GetResString(IDS_TYPE),LVCFMT_LEFT,50,2);
	InsertColumn(3,GetResString(IDS_PRIORITY),LVCFMT_LEFT,70,3);
	InsertColumn(4,GetResString(IDS_FILEID),LVCFMT_LEFT,220,4);
	InsertColumn(5,GetResString(IDS_SF_REQUESTS),LVCFMT_LEFT,100,5);
	InsertColumn(6,GetResString(IDS_SF_ACCEPTS),LVCFMT_LEFT,100,6);
	InsertColumn(7,GetResString(IDS_SF_TRANSFERRED),LVCFMT_LEFT,120,7);
	InsertColumn(8,GetResString(IDS_UPSTATUS),LVCFMT_LEFT,100,8);
	InsertColumn(9,GetResString(IDS_FOLDER),LVCFMT_LEFT,200,9);
	InsertColumn(10,GetResString(IDS_COMPLSOURCES),LVCFMT_LEFT,100,10);
	InsertColumn(11,GetResString(IDS_SHAREDTITLE),LVCFMT_LEFT,200,11);
//<<-- ADDED STORMIT - Morph: PowerShare //
//<<-- ADDED STORMIT - ZZ Upload System //
	InsertColumn(12,GetResString(IDS_POWERSHARE_COLUMN_LABEL),LVCFMT_LEFT,70,13);
//<<-- ADDED STORMIT - ZZ Upload System //

//<<-- ADDED STORMIT - SLUGFILLER: Spreadbars - //
	InsertColumn(13,GetResString(IDS_SF_UPLOADED_PARTS),LVCFMT_LEFT,170,14); // SF
	InsertColumn(14,GetResString(IDS_SF_TURN_PART),LVCFMT_LEFT,100,15); // SF
	InsertColumn(15,GetResString(IDS_SF_TURN_SIMPLE),LVCFMT_LEFT,100,16); // VQB
	InsertColumn(16,GetResString(IDS_SF_FULLUPLOAD),LVCFMT_LEFT,100,17); // SF
//<<-- ADDED STORMIT - SLUGFILLER: Spreadbars - //

//<<-- ADDED STORMIT - SLUGFILLER: HIDEOS //
	InsertColumn(17,GetResString(IDS_HIDEOS),LVCFMT_LEFT,100,18);
//<<-- ADDED STORMIT - SLUGFILLER: HIDEOS //

//<<-- ADDED STORMIT - SHARE_ONLY_THE_NEED //
	InsertColumn(18,GetResString(IDS_SHAREONLYTHENEED),LVCFMT_LEFT,100,19);
//<<-- ADDED STORMIT - SHARE_ONLY_THE_NEED //
//<<-- ADDED STORMIT - Morph: PowerShare //
	

	SetAllIcons();
	CreateMenues();
	LoadSettings();

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:20));
}

void CSharedFilesCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
	CreateMenues();
}

void CSharedFilesCtrl::SetAllIcons()
{
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);
	m_ImageList.Add(CTempIconLoader(_T("EMPTY")));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedServer"), 16, 16));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedKad"), 16, 16));
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("FileCommentsOvl"))), 1);
}

void CSharedFilesCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_DL_FILENAME);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(0, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_SIZE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(1, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_TYPE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(2, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_PRIORITY);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(3, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_FILEID);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(4, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SF_REQUESTS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(5, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SF_ACCEPTS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(6, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SF_TRANSFERRED);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(7, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SHARED_STATUS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(8, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_FOLDER);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(9, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_COMPLSOURCES);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(10, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SHAREDTITLE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(11, &hdi);
	strRes.ReleaseBuffer();

//<<-- ADDED STORMIT - Morph: PowerShare //
//<<-- ADDED STORMIT - ZZ Upload System //
	strRes = GetResString(IDS_POWERSHARE_COLUMN_LABEL);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(12, &hdi);
	strRes.ReleaseBuffer();
//<<-- ADDED STORMIT - Morph: PowerShare //

//<<-- ADDED STORMIT - SLUGFILLER: Spreadbars - //
	strRes = GetResString(IDS_SF_UPLOADED_PARTS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(13, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SF_TURN_PART);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(14, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SF_TURN_SIMPLE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(15, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SF_FULLUPLOAD);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(16, &hdi);

	strRes.ReleaseBuffer();
	//<<-- ADDED STORMIT - SLUGFILLER: Spreadbars - //

	//<<-- ADDED STORMIT - SLUGFILLER:  HIDEOS - //
    strRes = GetResString(IDS_HIDEOS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(17, &hdi);
	strRes.ReleaseBuffer();
	//<<-- ADDED STORMIT - SLUGFILLER:  HIDEOS - //

	//<<-- ADDED STORMIT - SHARE ONLY THE NEED - //
	strRes = GetResString(IDS_SHAREONLYTHENEED);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(18, &hdi);
	strRes.ReleaseBuffer();
	//<<-- ADDED STORMIT - SHARE ONLY THE NEED - //
	// <--- Morph: PowerShare
	//<<-- ADDED STORMIT - Morph: PowerShare //

	//FRTK(kts)+
	// xMule_MOD: showSharePermissions
	strRes = GetResString(IDS_PERMISSION);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(19, &hdi);
	strRes.ReleaseBuffer();
	// xMule_MOD: showSharePermissions
	//FRTK(kts)-

	CreateMenues();

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		Update(i);
}

void CSharedFilesCtrl::AddFile(const CKnownFile* file)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	// check filter conditions if we should show this file right now
	if (m_pDirectoryFilter != NULL){
		CString strFilePath = file->GetPath();
		if (strFilePath.Right(1) == "\\"){
			strFilePath = strFilePath.Left(strFilePath.GetLength()-1);
		}
		switch(m_pDirectoryFilter->m_eItemType){
			case SDI_ALL:
				// No filter
				break;
			case SDI_FILESYSTEMPARENT:
				return; // no files
				break;
			case SDI_NO:
				// some shared directory
			case SDI_CATINCOMING:
				// Categories with special incoming dirs
			case SDI_UNSHAREDDIRECTORY:
				// Items from the whole filesystem tree
				if (strFilePath.CompareNoCase(m_pDirectoryFilter->m_strFullPath) != 0)
					return;
				break;
			case SDI_TEMP:
				// only tempfiles
				if (!file->IsPartFile())
					return;
				else if (m_pDirectoryFilter->m_nCatFilter != -1 && m_pDirectoryFilter->m_nCatFilter != ((CPartFile*)file)->GetCategory())
					return;
				break;
			case SDI_DIRECTORY:
				// any userselected shared dir but not incoming or temp
				if (file->IsPartFile())
					return;
				if (strFilePath.CompareNoCase(thePrefs.GetIncomingDir()) == 0)
					return;
				break;
			case SDI_INCOMING:
				// Main incoming directory
				if (strFilePath.CompareNoCase(thePrefs.GetIncomingDir()) != 0)
					return;
				// Hmm should we show all incoming files dirs or only those from the main incoming dir here?
				// hard choice, will only show the main for now
				break;

		}
	}
	if (FindFile(file) != -1)
		return;
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)file);
	if (iItem >= 0)
		Update(iItem);
}

void CSharedFilesCtrl::RemoveFile(const CKnownFile* file)
{
	int iItem = FindFile(file);
	if (iItem != -1)
	{
		DeleteItem(iItem);
		ShowFilesCount();
	}
}

void CSharedFilesCtrl::UpdateFile(const CKnownFile* file)
{
	if (!file || !theApp.emuledlg->IsRunning())
		return;
	int iItem = FindFile(file);
	if (iItem != -1)
	{
		Update(iItem);
		if (GetItemState(iItem, LVIS_SELECTED))
			theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
	}
}

int CSharedFilesCtrl::FindFile(const CKnownFile* pFile)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFile;
	return FindItem(&find);
}

void CSharedFilesCtrl::ReloadFileList()
{
	DeleteAllItems();
	theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
	
	CCKey bufKey;
	CKnownFile* cur_file;
	for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition(); pos != 0; ){
		theApp.sharedfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
		AddFile(cur_file);
	}
	ShowFilesCount();
}

void CSharedFilesCtrl::ShowFilesCount()
{
	CString str;
	if (theApp.sharedfiles->GetHashingCount() + nAICHHashing)
		str.Format(_T(" (%i, %s %i)"), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing);
	else
		str.Format(_T(" (%i)"), theApp.sharedfiles->GetCount());
	theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES) + str);
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CSharedFilesCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;
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

//<<-- ADDED STORMIT - Morph: PowerShare //
	CKnownFile* file = (CKnownFile*)lpDrawItemStruct->itemData; // Morph: PowerShare
////	const CKnownFile* file = (CKnownFile*)lpDrawItemStruct->itemData;
//<<-- ADDED STORMIT - Morph: PowerShare //


	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	CRect cur_rec(lpDrawItemStruct->rcItem);
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
	const int iMarginX = 4;
	cur_rec.right = cur_rec.left - iMarginX*2;
	cur_rec.left += iMarginX;
	CString buffer;
	int iIconDrawWidth = theApp.GetSmallSytemIconSize().cx + 3;
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			UINT uDTFlags = DLC_DT_TEXT;
			cur_rec.right += GetColumnWidth(iColumn);
			switch(iColumn){
				case 0:{
					int iImage = theApp.GetFileTypeSystemImageIdx(file->GetFileName());
					if (theApp.GetSystemImageList() != NULL)
						::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc.GetSafeHdc(), cur_rec.left, cur_rec.top, ILD_NORMAL|ILD_TRANSPARENT);
					if (!file->GetFileComment().IsEmpty() || file->GetFileRating())
						m_ImageList.Draw(dc, 0, CPoint(cur_rec.left, cur_rec.top), ILD_NORMAL | ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
					cur_rec.left += (iIconDrawWidth - 3);

					if (thePrefs.ShowRatingIndicator() && (file->HasComment() || file->HasRating()))
					{
						m_ImageList.Draw(dc, file->UserRating()+3, CPoint(cur_rec.left, cur_rec.top), ILD_NORMAL);
						cur_rec.left += 16;
						iIconDrawWidth += 16;
					}

					cur_rec.left += 3;
	// xMule_MOD: showSharePermissions, modified by itsonlyme
					// display not finished files in navy, blocked files in red and friend-only files in orange
					if (file->GetPermissions() == PERM_NOONE)
						dc->SetTextColor((COLORREF)RGB(240,0,0));
					else if (file->GetPermissions() == PERM_FRIENDS)
						dc->SetTextColor((COLORREF)RGB(208,128,0));
					else if (file->IsPartFile())
						dc->SetTextColor((COLORREF)RGB(0,0,192));
					// xMule_MOD: showSharePermissions
					//FRTK(kts)-
					buffer = file->GetFileName();
					break;
				}
				case 1:
					buffer = CastItoXBytes(file->GetFileSize(), false, false);
					uDTFlags |= DT_RIGHT;
					break;
				case 2:
					buffer = file->GetFileTypeDisplayStr();
					break;
				case 3:{
					switch (file->GetUpPriority()) {
						case PR_VERYLOW :
							buffer = GetResString(IDS_PRIOVERYLOW);
							break;
						case PR_LOW :
							if( file->IsAutoUpPriority() )
								buffer = GetResString(IDS_PRIOAUTOLOW);
							else
								buffer = GetResString(IDS_PRIOLOW);
							break;
						case PR_NORMAL :
							if( file->IsAutoUpPriority() )
								buffer = GetResString(IDS_PRIOAUTONORMAL);
							else
								buffer = GetResString(IDS_PRIONORMAL);
							break;
						case PR_HIGH :
							if( file->IsAutoUpPriority() )
								buffer = GetResString(IDS_PRIOAUTOHIGH);
							else
								buffer = GetResString(IDS_PRIOHIGH);
							break;
						case PR_VERYHIGH :
							buffer = GetResString(IDS_PRIORELEASE);
							break;
						default:
							buffer.Empty();
					}
//<<-- ADDED STORMIT - Morph: PowerShare //
						if(file->GetPowerShared()) {
                            CString tempString = GetResString(IDS_POWERSHARE_PREFIX);
                            tempString.Append(_T(" "));
                            tempString.Append(buffer);
                            buffer.Empty();
                            buffer = tempString;
					} 
									// <--- Morph: PowerShare
//<<-- ADDED STORMIT - Morph: PowerShare //
						//Telp Super Release
					if (file->IsReleaseFile())
						buffer += _T(" [") + GetResString(IDS_RELEASEFILE) + _T("]");
					//Telp Super Release

					break;
				}
				case 4:
					buffer = md4str(file->GetFileHash());
					break;
				case 5:
                    buffer.Format(_T("%u (%u)"), file->statistic.GetRequests(), file->statistic.GetAllTimeRequests());
					break;
				case 6:
					buffer.Format(_T("%u (%u)"), file->statistic.GetAccepts(), file->statistic.GetAllTimeAccepts());
					break;
				case 7:
					buffer.Format(_T("%s (%s)"), CastItoXBytes(file->statistic.GetTransferred(), false, false), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false));
					break;
				case 8:
					if (file->GetPartCount()){
						cur_rec.bottom--;
						cur_rec.top++;
						file->DrawShareStatusBar(dc,&cur_rec,false,thePrefs.UseFlatBar());
						cur_rec.bottom++;
						cur_rec.top--;
					}
					break;
				case 9:
					buffer = file->GetPath();
					PathRemoveBackslash(buffer.GetBuffer());
					buffer.ReleaseBuffer();
					break;
				case 10:{

                    if (file->m_nCompleteSourcesCountLo == file->m_nCompleteSourcesCountHi){
						buffer.Format(_T("%u"), file->m_nCompleteSourcesCountLo);
}
                    else if (file->m_nCompleteSourcesCountLo == 0)
						buffer.Format(_T("< %u"), file->m_nCompleteSourcesCountHi);
					else
						buffer.Format(_T("%u - %u"), file->m_nCompleteSourcesCountLo, file->m_nCompleteSourcesCountHi);

//<<-- ADDED STORMIT - Morph: PowerShare //
					CString buffer2;
					buffer2.Format(_T(" (%u)"),file->m_nVirtualCompleteSourcesCount);
					buffer.Append(buffer2);
					// <--- Morph: PowerShare
//<<-- ADDED STORMIT - Morph: PowerShare //

					break;
						}
				case 11:{
					CPoint pt(cur_rec.left, cur_rec.top);
					m_ImageList.Draw(dc, file->GetPublishedED2K() ? 1 : 0, pt, ILD_NORMAL | ILD_TRANSPARENT);
					pt.x += 16;
					bool bSharedInKad;
					if ((uint32)time(NULL) < file->GetLastPublishTimeKadSrc())
					{
						if (theApp.IsFirewalled() && theApp.IsConnected())
						{
							if (theApp.clientlist->GetBuddy() && (file->GetLastPublishBuddy() == theApp.clientlist->GetBuddy()->GetIP()))
								bSharedInKad = true;
							else
								bSharedInKad = false;
						}
						else
							bSharedInKad = true;
					}
					else
						bSharedInKad = false;
					m_ImageList.Draw(dc, bSharedInKad ? 2 : 0, pt, ILD_NORMAL | ILD_TRANSPARENT);
					buffer.Empty();
					break;
				}
			//<<-- ADDED STORMIT - Morph: PowerShare //
				case 12:{
					int powersharemode;
					bool powershared = file->GetPowerShared();
					buffer = _T("[") + GetResString((powershared)?IDS_POWERSHARE_ON_LABEL:IDS_POWERSHARE_OFF_LABEL) + _T("] ");
					if (file->GetPowerSharedMode()>=0)
						powersharemode = file->GetPowerSharedMode();
					else {
						powersharemode = thePrefs.GetPowerShareMode();
						buffer.Append(_T(" ") + ((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". "));
						}
					if(powersharemode == 2)
						buffer.Append(GetResString(IDS_POWERSHARE_AUTO_LABEL));
					else if (powersharemode == 1)
						buffer.Append(GetResString(IDS_POWERSHARE_ACTIVATED_LABEL));
					else if (powersharemode == 3) {
						buffer.Append(GetResString(IDS_POWERSHARE_LIMITED));
						if (file->GetPowerShareLimit()<0)
							buffer.AppendFormat(_T(" %s. %i"), ((CString)GetResString(IDS_DEFAULT)).Left(1), thePrefs.GetPowerShareLimit());
						else
							buffer.AppendFormat(_T(" %i"), file->GetPowerShareLimit());
					}
					else
						buffer.Append(GetResString(IDS_POWERSHARE_DISABLED_LABEL));
					buffer.Append(_T(" ("));
					if (file->GetPowerShareAuto())
						buffer.Append(GetResString(IDS_POWERSHARE_ADVISED_LABEL));
					else if (file->GetPowerShareLimited() && (powersharemode == 3))
						buffer.Append(GetResString(IDS_POWERSHARE_LIMITED));
					else if (file->GetPowerShareAuthorized())
						buffer.Append(GetResString(IDS_POWERSHARE_AUTHORIZED_LABEL));
					else
						buffer.Append(GetResString(IDS_POWERSHARE_DENIED_LABEL));
					buffer.Append(_T(")"));
					break;
			}
//<<-- ADDED STORMIT - SLUGFILLER: Spreadbars //
				case 13:
						cur_rec.bottom--;
						cur_rec.top++;
						((CKnownFile*)lpDrawItemStruct->itemData)->statistic.DrawSpreadBar(dc,&cur_rec,thePrefs.UseFlatBar());
						cur_rec.bottom++;
						cur_rec.top--;
						break;
				case 14:
						buffer.Format(_T("%.2f"),((CKnownFile*)lpDrawItemStruct->itemData)->statistic.GetSpreadSortValue());
						break;
				case 15:
						if (file->GetFileSize())
							buffer.Format(_T("%.2f"),((float)file->statistic.GetAllTimeTransferred())/((float)file->GetFileSize()));
						else
							buffer.Format(_T("%.2f"),0.0f);
						break;
				case 16:
						buffer.Format(_T("%.2f"),((CKnownFile*)lpDrawItemStruct->itemData)->statistic.GetFullSpreadCount());
						break;
//<<-- ADDED STORMIT - SLUGFILLER: Spreadbars //

//<<-- ADDED STORMIT - SLUGFILLER: HIDEOS //
				case 17:
					{
						uint8 hideOSInWork = file->HideOSInWork();
						buffer = _T("[") + GetResString((hideOSInWork>0)?IDS_POWERSHARE_ON_LABEL:IDS_POWERSHARE_OFF_LABEL) + _T("] ");
						if(file->GetHideOS()<0)
							buffer.Append(_T(" ") + ((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". "));
						hideOSInWork = (file->GetHideOS()>=0)?file->GetHideOS():thePrefs.GetHideOvershares();
						if (hideOSInWork>0)
							buffer.AppendFormat(_T("%i"), hideOSInWork);
						//MORPH	Start	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
						else if(file->GetSpreadbarSetStatus() == 0 || (file->GetSpreadbarSetStatus() == -1 && thePrefs.GetSparsePartFiles() == 0))
							buffer.AppendFormat(_T("%s"), GetResString(IDS_SPREADBAR) + _T(" ") + GetResString(IDS_DISABLED));
						//MORPH	End	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
						else
							buffer.AppendFormat(_T("%s"), GetResString(IDS_DISABLED));
						if (file->GetSelectiveChunk()>=0){
							if (file->GetSelectiveChunk())
								buffer.Append(_T(" + S"));
						}else
							if (thePrefs.IsSelectiveShareEnabled())
								buffer.Append(_T(" + ") + ((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". S"));
						break;
					}
				case 18:
						if(file->GetShareOnlyTheNeed()>=0)
							if (file->GetShareOnlyTheNeed())
								buffer.Format(_T("%i") ,file->GetShareOnlyTheNeed());
							else
								buffer = GetResString(IDS_DISABLED);
						else
							buffer = GetResString(IDS_DEFAULT);
						break;
					// <--- Morph: PowerShare
//<<-- ADDED STORMIT - Morph: PowerShare //
			//FRTK(kts)+
			// xMule_MOD: showSharePermissions
			case 19:
				switch (file->GetPermissions())
				{
					case PERM_NOONE: 
						buffer = GetResString(IDS_HIDDEN); 
						break;
					case PERM_FRIENDS: 
						buffer = GetResString(IDS_FSTATUS_FRIENDSONLY); 
						break;
					default: 
						buffer = GetResString(IDS_FSTATUS_PUBLIC); 
						break;
				}
				break;
			// xMule_MOD: showSharePermissions
			//FRTK(kts)-
			}

//<<-- ADDED STORMIT - Morph: PowerShare //
			if( iColumn != 8 && iColumn!=13) // Morph: PowerShare
//<<-- ADDED STORMIT - Morph: PowerShare //

				dc.DrawText(buffer, buffer.GetLength(), &cur_rec, uDTFlags);
			if (iColumn == 0)
				
				cur_rec.left -= iIconDrawWidth;
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}
	ShowFilesCount();
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc.FrameRect(&outline_rec, &CBrush(m_crWindow));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if (lpDrawItemStruct->itemID > 0 && GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))
			outline_rec.top--;

		if (lpDrawItemStruct->itemID + 1 < (UINT)GetItemCount() && GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))
			outline_rec.bottom++;

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

void CSharedFilesCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// get merged settings
	bool bFirstItem = true;
	int iSelectedItems = GetSelectedCount();
	int iCompleteFileSelected = -1;
CString buffer;
//KTS+webcache
	bool uWCReleaseItem = true; //JP webcache release
	bool uGreyOutWCRelease = true; //JP webcache release
	//KTS- webcache
//<<-- ADDED STORMIT - Morph: PowerShare //
	int iPowerShareLimit = -1; //<<-- ADDED STORMIT - POWERSHARE Limit //
	int iHideOS = -1; //<<-- ADDED STORMIT - HIDEOS //
	UINT uPowershareMenuItem = 0; //<<-- ADDED STORMIT - Powershare //
	UINT uPowerShareLimitMenuItem = 0; //<<-- ADDED STORMIT - POWERSHARE Limit //
	UINT uSpreadbarMenuItem = 0;
	UINT uHideOSMenuItem = 0; //<<-- ADDED STORMIT - HIDEOS //
	UINT uSelectiveChunkMenuItem = 0; //<<-- ADDED STORMIT - HIDEOS //
	UINT uShareOnlyTheNeedMenuItem = 0; //<<-- ADDED STORMIT - SHARE_ONLY_THE_NEED //
	// <--- Morph: PowerShare
//Telp Super Release
	bool bReleaseFileSelected = false;
	bool bNonReleaseFileSelected = false;
	//Telp Super Release
//<<-- ADDED STORMIT - Morph: PowerShare //
	UINT uPrioMenuItem = 0;
	//FRTK(kts)+
	UINT uPermMenuItem = 0;	// xMule_MOD: showSharePermissions
	//FRTK(kts)-
	const CKnownFile* pSingleSelFile = NULL;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		const CKnownFile* pFile = (CKnownFile*)GetItemData(GetNextSelectedItem(pos));
		if (bFirstItem)
			pSingleSelFile = pFile;
		else
			pSingleSelFile = NULL;

		int iCurCompleteFile = pFile->IsPartFile() ? 0 : 1;
		if (bFirstItem)
			iCompleteFileSelected = iCurCompleteFile;
		else if (iCompleteFileSelected != iCurCompleteFile)
			iCompleteFileSelected = -1;

		UINT uCurPrioMenuItem = 0;
		if (pFile->IsAutoUpPriority())
			uCurPrioMenuItem = MP_PRIOAUTO;
		else if (pFile->GetUpPriority() == PR_VERYLOW)
			uCurPrioMenuItem = MP_PRIOVERYLOW;
		else if (pFile->GetUpPriority() == PR_LOW)
			uCurPrioMenuItem = MP_PRIOLOW;
		else if (pFile->GetUpPriority() == PR_NORMAL)
			uCurPrioMenuItem = MP_PRIONORMAL;
		else if (pFile->GetUpPriority() == PR_HIGH)
			uCurPrioMenuItem = MP_PRIOHIGH;
		else if (pFile->GetUpPriority() == PR_VERYHIGH)
			uCurPrioMenuItem = MP_PRIOVERYHIGH;
		else
		ASSERT(0);

		if (bFirstItem)
			uPrioMenuItem = uCurPrioMenuItem;
		else if (uPrioMenuItem != uCurPrioMenuItem)
			uPrioMenuItem = 0;
		//Telp Super Release
		if (pFile->IsReleaseFile())
			bReleaseFileSelected = true;
		else
			bNonReleaseFileSelected = true;
		//Telp Super Release
		//<<-- ADDED STORMIT - Morph: PowerShare //
		UINT uCurPowershareMenuItem = 0;
		if (pFile->GetPowerSharedMode()==-1)
			uCurPowershareMenuItem = MP_POWERSHARE_DEFAULT;
		else
			uCurPowershareMenuItem = MP_POWERSHARE_DEFAULT+1 + pFile->GetPowerSharedMode();

		if (bFirstItem)
			uPowershareMenuItem = uCurPowershareMenuItem;
		else if (uPowershareMenuItem != uCurPowershareMenuItem)
			uPowershareMenuItem = 0;

		UINT uCurPowerShareLimitMenuItem = 0;
		int iCurPowerShareLimit = pFile->GetPowerShareLimit();
		if (iCurPowerShareLimit==-1)
			uCurPowerShareLimitMenuItem = MP_POWERSHARE_LIMIT;
		else
			uCurPowerShareLimitMenuItem = MP_POWERSHARE_LIMIT_SET;

		if (bFirstItem)
		{
			uPowerShareLimitMenuItem = uCurPowerShareLimitMenuItem;
			iPowerShareLimit = iCurPowerShareLimit;
		}
		else if (uPowerShareLimitMenuItem != uCurPowerShareLimitMenuItem || iPowerShareLimit != iCurPowerShareLimit)
		{
			uPowerShareLimitMenuItem = 0;
			iPowerShareLimit = -1;
	}
//FRTK(kts)+
		// xMule_MOD: showSharePermissions
		UINT uCurPermMenuItem = 0;
		if (pFile->GetPermissions() == PERM_ALL)
			uCurPermMenuItem = MP_PERMALL;
		else if (pFile->GetPermissions() == PERM_FRIENDS)
			uCurPermMenuItem = MP_PERMFRIENDS;
		else if (pFile->GetPermissions() == PERM_NOONE)
			uCurPermMenuItem = MP_PERMNONE;
		else
			ASSERT(0);

		if (bFirstItem)
			uPermMenuItem = uCurPermMenuItem;
		else if (uPermMenuItem != uCurPermMenuItem)
			uPermMenuItem = 0;
		// xMule_MOD: showSharePermissions
		//FRTK(kts)-
		//FRTK(kts)+ Hideos
		UINT uCurSpreadbarMenuItem = 0;
		if(pFile->GetSpreadbarSetStatus() == -1)
			uCurSpreadbarMenuItem = MP_SPREADBAR_DEFAULT;
		else if(pFile->GetSpreadbarSetStatus() == 0)
			uCurSpreadbarMenuItem = MP_SPREADBAR_OFF;
		else if(pFile->GetSpreadbarSetStatus() == 1)
			uCurSpreadbarMenuItem = MP_SPREADBAR_ON;
		else
			ASSERT(0);

		if (bFirstItem)
			uSpreadbarMenuItem = uCurSpreadbarMenuItem;
		else if (uSpreadbarMenuItem != uCurSpreadbarMenuItem)
			uSpreadbarMenuItem = 0;

		UINT uCurHideOSMenuItem = 0;
		int iCurHideOS = pFile->GetHideOS();
		if (iCurHideOS == -1)
			uCurHideOSMenuItem = MP_HIDEOS_DEFAULT;
		else
			uCurHideOSMenuItem = MP_HIDEOS_SET;
		if (bFirstItem)
		{
			uHideOSMenuItem = uCurHideOSMenuItem;
			iHideOS = iCurHideOS;
		}
		else if (uHideOSMenuItem != uCurHideOSMenuItem || iHideOS != iCurHideOS)
		{
			uHideOSMenuItem = 0;
			iHideOS = -1;
	}
UINT uCurSelectiveChunkMenuItem = 0;
		if (pFile->GetSelectiveChunk() == -1)
			uCurSelectiveChunkMenuItem = MP_SELECTIVE_CHUNK;
		else
			uCurSelectiveChunkMenuItem = MP_SELECTIVE_CHUNK+1 + pFile->GetSelectiveChunk();
		if (bFirstItem)
			uSelectiveChunkMenuItem = uCurSelectiveChunkMenuItem;
		else if (uSelectiveChunkMenuItem != uCurSelectiveChunkMenuItem)
			uSelectiveChunkMenuItem = 0;

		UINT uCurShareOnlyTheNeedMenuItem = 0;
		if (pFile->GetShareOnlyTheNeed() == -1)
			uCurShareOnlyTheNeedMenuItem = MP_SHAREONLYTHENEED;
		else
			uCurShareOnlyTheNeedMenuItem = MP_SHAREONLYTHENEED+1 + pFile->GetShareOnlyTheNeed();
		if (bFirstItem)
			uShareOnlyTheNeedMenuItem = uCurShareOnlyTheNeedMenuItem ;
		else if (uShareOnlyTheNeedMenuItem != uCurShareOnlyTheNeedMenuItem)
			uShareOnlyTheNeedMenuItem = 0;
		// <--- Morph: PowerShare
//<<-- ADDED STORMIT - Morph: PowerShare //
	bFirstItem = false;
	}
//FRTK(kts)+
	// xMule_MOD: showSharePermissions
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PermMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_PermMenu.CheckMenuRadioItem(MP_PERMALL, MP_PERMNONE, uPermMenuItem, 0);
	// xMule_MOD: showSharePermissions
	//FRTK(kts)-
//KTS+ webcache
	//jp webcache release START
	m_PrioMenu.EnableMenuItem(MP_PRIOWCRELEASE, (thePrefs.UpdateWebcacheReleaseAllowed() && !uGreyOutWCRelease) ? MF_ENABLED : MF_GRAYED);
	if (uWCReleaseItem && thePrefs.IsWebcacheReleaseAllowed()) //JP webcache release
		m_PrioMenu.CheckMenuItem(MP_PRIOWCRELEASE, MF_CHECKED);
	else
		m_PrioMenu.CheckMenuItem(MP_PRIOWCRELEASE, MF_UNCHECKED);
	//jp webcache relesae END
	//KTS- webcache
//Telp Super Release
	m_PrioMenu.EnableMenuItem(MP_RELEASESET, bNonReleaseFileSelected ? MF_ENABLED : MF_GRAYED);
	m_PrioMenu.EnableMenuItem(MP_RELEASEREMOVE, bReleaseFileSelected ? MF_ENABLED : MF_GRAYED);
	//Telp Super Release
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOAUTO, uPrioMenuItem, 0);
	bool bSingleCompleteFileSelected = (iSelectedItems == 1 && iCompleteFileSelected == 1);
	m_SharedFilesMenu.EnableMenuItem(MP_OPEN, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	UINT uInsertedMenuItem = 0;
	static const TCHAR _szSkinPkgSuffix[] = _T(".") EMULSKIN_BASEEXT _T(".zip");
	if (bSingleCompleteFileSelected 
		&& pSingleSelFile 
		&& pSingleSelFile->GetFilePath().Right(ARRSIZE(_szSkinPkgSuffix)-1).CompareNoCase(_szSkinPkgSuffix) == 0)
	{
		MENUITEMINFO mii = {0};
		mii.cbSize = sizeof mii;
		mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.fState = MFS_ENABLED;
		mii.wID = MP_INSTALL_SKIN;
		CString strBuff(GetResString(IDS_INSTALL_SKIN));
		mii.dwTypeData = const_cast<LPTSTR>((LPCTSTR)strBuff);
		if (::InsertMenuItem(m_SharedFilesMenu, MP_OPENFOLDER, FALSE, &mii))
			uInsertedMenuItem = mii.wID;
	}
	m_SharedFilesMenu.EnableMenuItem(MP_OPENFOLDER, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_RENAME, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_REMOVE, iCompleteFileSelected > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.SetDefaultItem(bSingleCompleteFileSelected ? MP_OPEN : -1);
	m_SharedFilesMenu.EnableMenuItem(MP_CMT, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_DETAIL, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_SpreadbarMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	switch (thePrefs.GetSpreadbarSetStatus()){
		case 0:
			buffer.Format(_T(" (%s)"),GetResString(IDS_DISABLED));
			break;
		case 1:
			buffer.Format(_T(" (%s)"),GetResString(IDS_ENABLED));
			break;
		default:
			buffer = _T(" (?)");
			break;
	}
	m_SharedFilesMenu.ModifyMenu(MP_SPREADBAR_DEFAULT, MF_STRING, MP_SPREADBAR_DEFAULT, GetResString(IDS_DEFAULT) + buffer);
	m_SharedFilesMenu.CheckMenuRadioItem(MP_SPREADBAR_DEFAULT, MP_SPREADBAR_ON, uSpreadbarMenuItem, 0);

//<<-- ADDED STORMIT - Morph: PowerShare //
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_HideOSMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_SelectiveChunkMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	if (thePrefs.GetHideOvershares()==0)
		buffer.Format(_T(" (%s)"),GetResString(IDS_DISABLED));
	else
		buffer.Format(_T(" (%u)"),thePrefs.GetHideOvershares());
	m_HideOSMenu.ModifyMenu(MP_HIDEOS_DEFAULT, MF_STRING,MP_HIDEOS_DEFAULT, GetResString(IDS_DEFAULT) + buffer);
	if (iHideOS==-1)
		buffer = _T("Set");
	else if (iHideOS==0)
		buffer = GetResString(IDS_DISABLED);
	else
		buffer.Format(_T("%i"), iHideOS);
	m_HideOSMenu.ModifyMenu(MP_HIDEOS_SET, MF_STRING,MP_HIDEOS_SET, buffer);
	m_HideOSMenu.CheckMenuRadioItem(MP_HIDEOS_DEFAULT, MP_HIDEOS_SET, uHideOSMenuItem, 0);
	buffer.Format(_T(" (%s)"),thePrefs.IsSelectiveShareEnabled()?GetResString(IDS_ENABLED):GetResString(IDS_DISABLED));
	m_SelectiveChunkMenu.ModifyMenu(MP_SELECTIVE_CHUNK, MF_STRING, MP_SELECTIVE_CHUNK, GetResString(IDS_DEFAULT) + buffer);
	m_SelectiveChunkMenu.CheckMenuRadioItem(MP_SELECTIVE_CHUNK, MP_SELECTIVE_CHUNK_1, uSelectiveChunkMenuItem, 0);

	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_ShareOnlyTheNeedMenu.m_hMenu, iSelectedItems > 0 && iCompleteFileSelected ==1 ? MF_ENABLED : MF_GRAYED);
	buffer.Format(_T(" (%s)"),thePrefs.GetShareOnlyTheNeed()?GetResString(IDS_ENABLED):GetResString(IDS_DISABLED));
	m_ShareOnlyTheNeedMenu.ModifyMenu(MP_SHAREONLYTHENEED, MF_STRING, MP_SHAREONLYTHENEED, GetResString(IDS_DEFAULT) + buffer);
	m_ShareOnlyTheNeedMenu.CheckMenuRadioItem(MP_SHAREONLYTHENEED, MP_SHAREONLYTHENEED_1, uShareOnlyTheNeedMenuItem, 0);

	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PowershareMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	switch (thePrefs.GetPowerShareMode()){
case 0:
	buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_DISABLED));
	break;
case 1:
	buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_ACTIVATED));
	break;
case 2:
	buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_AUTO));
	break;
case 3:
	buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_LIMITED));
	break;
default:
	buffer = _T(" (?)");
	break;
	}
	m_PowershareMenu.ModifyMenu(MP_POWERSHARE_DEFAULT, MF_STRING,MP_POWERSHARE_DEFAULT, GetResString(IDS_DEFAULT) + buffer);
	m_PowershareMenu.CheckMenuRadioItem(MP_POWERSHARE_DEFAULT, MP_POWERSHARE_LIMITED, uPowershareMenuItem, 0);
	
	m_PowershareMenu.EnableMenuItem((UINT_PTR)m_PowerShareLimitMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	if (iPowerShareLimit==0)
		buffer.Format(_T(" (%s)"),GetResString(IDS_DISABLED));
	else
		buffer.Format(_T(" (%u)"),thePrefs.GetPowerShareLimit());
	m_PowerShareLimitMenu.ModifyMenu(MP_POWERSHARE_LIMIT, MF_STRING,MP_POWERSHARE_LIMIT, GetResString(IDS_DEFAULT) + buffer);
	if (iPowerShareLimit==-1)
		buffer = _T("Set");
	else if (iPowerShareLimit==0)
		buffer = GetResString(IDS_DISABLED);
	else
		buffer.Format(_T("%i"),iPowerShareLimit);
	m_PowerShareLimitMenu.ModifyMenu(MP_POWERSHARE_LIMIT_SET, MF_STRING,MP_POWERSHARE_LIMIT_SET, buffer);
	m_PowerShareLimitMenu.CheckMenuRadioItem(MP_POWERSHARE_LIMIT, MP_POWERSHARE_LIMIT_SET, uPowerShareLimitMenuItem, 0);
	// <--- Morph: PowerShare
//<<-- ADDED STORMIT - Morph: PowerShare //

	m_CollectionsMenu.EnableMenuItem(MP_MODIFYCOLLECTION, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL ) ? MF_ENABLED : MF_GRAYED);
	m_CollectionsMenu.EnableMenuItem(MP_VIEWCOLLECTION, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL ) ? MF_ENABLED : MF_GRAYED);
	m_CollectionsMenu.EnableMenuItem(MP_SEARCHAUTHOR, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL && !pSingleSelFile->m_pCollection->GetAuthorKeyHashString().IsEmpty()) ? MF_ENABLED : MF_GRAYED);
#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
	//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		if (iSelectedItems > 0 && theApp.IsConnected() && theApp.IsFirewalled() && theApp.clientlist->GetBuddy())
			m_SharedFilesMenu.EnableMenuItem(MP_GETKADSOURCELINK, MF_ENABLED);
		else
			m_SharedFilesMenu.EnableMenuItem(MP_GETKADSOURCELINK, MF_GRAYED);
	}
#endif
	if (thePrefs.IsExtControlsEnabled())
	//MORPH START - Added by IceCream, copy feedback feature
	m_SharedFilesMenu.EnableMenuItem(MP_COPYFEEDBACK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_COPYFEEDBACK_NOCODE, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	//MORPH END   - Added by IceCream, copy feedback feature
	CTitleMenu WebMenu;
	WebMenu.CreateMenu();
	WebMenu.AddMenuTitle(NULL, true);
	int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
	UINT flag2 = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_STRING;
	m_SharedFilesMenu.AppendMenu(flag2 | MF_POPUP, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));

	GetPopupMenuPos(*this, point);
	m_SharedFilesMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);

	m_SharedFilesMenu.RemoveMenu(m_SharedFilesMenu.GetMenuItemCount()-1,MF_BYPOSITION);
	VERIFY( WebMenu.DestroyMenu() );
	if (uInsertedMenuItem)
		VERIFY( m_SharedFilesMenu.RemoveMenu(uInsertedMenuItem, MF_BYCOMMAND) );
}

BOOL CSharedFilesCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	CTypedPtrList<CPtrList, CKnownFile*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL){
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
			selectedList.AddTail((CKnownFile*)GetItemData(index));
	}

	if (wParam==MP_CREATECOLLECTION || selectedList.GetCount() > 0)
	{
		CKnownFile* file = NULL;
		if (selectedList.GetCount() == 1)
			file = selectedList.GetHead();

		switch (wParam){
					case MP_GETED2KLINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += CreateED2kLink(file);
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
#if defined(_DEBUG)
			//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
			case MP_GETKADSOURCELINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += theApp.CreateKadSourceLink(file);
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
#endif

									 //MORPH START - Added by IceCream, copy feedback feature
			case MP_COPYFEEDBACK:
			case MP_COPYFEEDBACK_NOCODE:
				{
					CString feed = _T("");
						POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						feed.Append(file->GetFeedback(wParam==MP_COPYFEEDBACK));
						feed.Append(_T("\r\n"));
					}
					//Todo: copy all the comments too
					theApp.CopyTextToClipboard(feed);
					break;
				}
				//MORPH END - Added by IceCream, copy feedback feature 
			// file operations
			case MP_OPEN:
				if (file && !file->IsPartFile())
					OpenFile(file);
				break; 
			case MP_INSTALL_SKIN:
				if (file && !file->IsPartFile())
					InstallSkin(file->GetFilePath());
				break;
			case MP_OPENFOLDER:
				//Xman
				//Fix For Shared Files "open Folder" [Avi-3k]
				/* 
				if (file && !file->IsPartFile()){
					CString path = file->GetPath();
					int bspos = path.ReverseFind(_T('\\'));
					ShellExecute(NULL, _T("open"), path.Left(bspos), NULL, NULL, SW_SHOW);
				}
				*/
				if (file && !file->IsPartFile()){
					ShellExecute(NULL, _T("open"), file->GetPath(), NULL, NULL, SW_SHOW);
				}
				//Xman end
				break; 
			case MP_RENAME:
			case MPG_F2:
				if (file && !file->IsPartFile()){
					InputBox inputbox;
					CString title = GetResString(IDS_RENAME);
					title.Remove(_T('&'));
					inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), file->GetFileName());
					inputbox.SetEditFilenameMode();
					inputbox.DoModal();
					CString newname = inputbox.GetInput();
					if (!inputbox.WasCancelled() && newname.GetLength()>0)
					{
						// at least prevent users from specifying something like "..\dir\file"
						static const TCHAR _szInvFileNameChars[] = _T("\\/:*?\"<>|");
						if (newname.FindOneOf(_szInvFileNameChars) != -1){
							AfxMessageBox(GetErrorMessage(ERROR_BAD_PATHNAME));
							break;
						}

						CString newpath;
						PathCombine(newpath.GetBuffer(MAX_PATH), file->GetPath(), newname);
						newpath.ReleaseBuffer();
						if (_trename(file->GetFilePath(), newpath) != 0){
							CString strError;
							strError.Format(GetResString(IDS_ERR_RENAMESF), file->GetFilePath(), newpath, strerror(errno));
							AfxMessageBox(strError);
							break;
						}
						
						if (file->IsKindOf(RUNTIME_CLASS(CPartFile)))
							file->SetFileName(newname);
						else
						{
							theApp.sharedfiles->RemoveKeywords(file);
							file->SetFileName(newname);
							theApp.sharedfiles->AddKeywords(file);
						}
						file->SetFilePath(newpath);
						UpdateFile(file);
					}
				}
				else
					MessageBeep(MB_OK);
				break;
			case MP_REMOVE:
			case MPG_DELETE:{
				if (IDNO == AfxMessageBox(GetResString(IDS_CONFIRM_FILEDELETE),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
					return TRUE;

				SetRedraw(FALSE);
				bool bRemovedItems = false;
				while (!selectedList.IsEmpty())
				{
					CKnownFile* myfile = selectedList.RemoveHead();
					if (!myfile || myfile->IsPartFile())
						continue;
					
					BOOL delsucc = FALSE;
					if (!PathFileExists(myfile->GetFilePath()))
						delsucc = TRUE;
					else{
						// Delete
						if (!thePrefs.GetRemoveToBin()){
							delsucc = DeleteFile(myfile->GetFilePath());
						}
						else{
							// delete to recycle bin :(
							TCHAR todel[MAX_PATH+1];
							MEMSET(todel, 0, sizeof todel);
							_tcsncpy(todel, myfile->GetFilePath(), ARRSIZE(todel)-2);

							SHFILEOPSTRUCT fp = {0};
							fp.wFunc = FO_DELETE;
							fp.hwnd = theApp.emuledlg->m_hWnd;
							fp.pFrom = todel;
							fp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;// | FOF_NOERRORUI
							delsucc = (SHFileOperation(&fp) == 0);
						}
					}
					if (delsucc){
						theApp.sharedfiles->RemoveFile(myfile);
						bRemovedItems = true;
						if (myfile->IsKindOf(RUNTIME_CLASS(CPartFile)))
							theApp.emuledlg->transferwnd->downloadlistctrl.ClearCompleted(static_cast<CPartFile*>(myfile));
					}
					else{
						CString strError;
						strError.Format( GetResString(IDS_ERR_DELFILE) + _T("\r\n\r\n%s"), myfile->GetFilePath(), GetErrorMessage(GetLastError()));
						AfxMessageBox(strError);
					}
				}
				SetRedraw(TRUE);
				if (bRemovedItems)
					AutoSelectItem();
				break; 
			}
			case MP_CMT:
				ShowFileDialog(selectedList, IDD_COMMENT);
                break; 
			case MPG_ALTENTER:
			case MP_DETAIL:
				ShowFileDialog(selectedList);
				break;
			case MP_CREATECOLLECTION:
			{
				CCollection* pCollection = new CCollection();
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					pCollection->AddFileToCollection(selectedList.GetNext(pos),true);
				}
				CCollectionCreateDialog dialog;
				dialog.SetCollection(pCollection,true);
				dialog.DoModal();
				//We delete this collection object because when the newly created
				//collection file is added to the sharedfile list, it is read and verified
				//and which creates the colleciton object that is attached to that file..
				delete pCollection;
				break;
			}
			case MP_SEARCHAUTHOR:
			{
				if (selectedList.GetCount() == 1 && file->m_pCollection)
				{
					SSearchParams* pParams = new SSearchParams;
					pParams->strExpression = file->m_pCollection->GetCollectionAuthorKeyString();
					pParams->eType = SearchTypeKademlia;
					pParams->strFileType = ED2KFTSTR_EMULECOLLECTION;
					pParams->strSpecialTitle = file->m_pCollection->m_sCollectionAuthorName;
					if (pParams->strSpecialTitle.GetLength() > 50){
						pParams->strSpecialTitle = pParams->strSpecialTitle.Left(50) + _T("...");
					}
					theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pParams);
				}
				break;
			}
			case MP_VIEWCOLLECTION:
			{
				if (selectedList.GetCount() == 1 && file->m_pCollection)
				{
					CCollectionViewDialog dialog;
					dialog.SetCollection(file->m_pCollection);
					dialog.DoModal();
				}
				break;
			}
			case MP_MODIFYCOLLECTION:
			{
				if (selectedList.GetCount() == 1 && file->m_pCollection)
				{
					CCollectionCreateDialog dialog;
					CCollection* pCollection = new CCollection(file->m_pCollection);
					dialog.SetCollection(pCollection,false);
					dialog.DoModal();
					delete pCollection;				
				}
				break;
			}
			case MP_SHOWED2KLINK:
				ShowFileDialog(selectedList, IDD_ED2KLINK);
				break;
 			//FRTK(kts)+
			// xMule_MOD: showSharePermissions
			case MP_PERMNONE:
			case MP_PERMFRIENDS:
			case MP_PERMALL:
			{
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					CKnownFile* file = selectedList.GetNext(pos);
					switch (wParam)
					{
						case MP_PERMNONE:
							file->SetPermissions(PERM_NOONE);
							UpdateFile(file);
							break;
						case MP_PERMFRIENDS:
							file->SetPermissions(PERM_FRIENDS);
							UpdateFile(file);
							break;
						default : // case MP_PERMALL:
							file->SetPermissions(PERM_ALL);
							UpdateFile(file);
							break;
					}
				}
				break;
			}
			// xMule_MOD: showSharePermissions
			//FRTK(kts)-
			case MP_PRIOVERYLOW:
			case MP_PRIOLOW:
			case MP_PRIONORMAL:
			case MP_PRIOHIGH:
			case MP_PRIOVERYHIGH:
	//KTS+ webcache
			case MP_PRIOWCRELEASE: //JP webcache release
		
				//KTS- webcache
			
			case MP_PRIOAUTO:
				{
					//KTS+ webcache
					//jp webcache release START 
					// check if a click on MP_PRIOWCRELEASE should activate WC-release
					bool activateWCRelease = false;
					POSITION pos2 = selectedList.GetHeadPosition();
					CKnownFile* cur_file = NULL;
					while (pos2 != NULL)
					{
						cur_file = selectedList.GetNext(pos2);
						if (!cur_file->ReleaseViaWebCache)
							activateWCRelease = true;
					}
					//jp webcache release END
					//KTS- webcache

					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						switch (wParam) {
							case MP_PRIOVERYLOW:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_VERYLOW);
								UpdateFile(file);
								break;
							case MP_PRIOLOW:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_LOW);
								UpdateFile(file);
								break;
							case MP_PRIONORMAL:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_NORMAL);
								UpdateFile(file);
								break;
							case MP_PRIOHIGH:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_HIGH);
								UpdateFile(file);
								break;
							case MP_PRIOVERYHIGH:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_VERYHIGH);
								UpdateFile(file);
								break;	
							case MP_PRIOAUTO:
								file->SetAutoUpPriority(true);
								file->UpdateAutoUpPriority();
								UpdateFile(file); 
								break;
					//MORPH START - Added by SiRoB, WebCache 1.2f
							//jp webcache release start
							case MP_PRIOWCRELEASE:
								if (!file->IsPartFile())
									file->SetReleaseViaWebCache(activateWCRelease);
								else
									file->SetReleaseViaWebCache(false);
								break;
							//jp webcache release end
							//MORPH END   - Added by SiRoB, WebCache 1.2f		
				}
					}
					break;
				}
//Telp Super Release
			case MP_RELEASESET:
				{
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						file->SetReleaseFile(true);
						UpdateFile(file);
					}
					break;
				}
			case MP_RELEASEREMOVE:
				{
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						file->SetReleaseFile(false);
						UpdateFile(file);
					}
					break;
				}
			//Telp Super Release

//<<-- ADDED STORMIT - Morph: PowerShare //
case MP_POWERSHARE_ON:
case MP_POWERSHARE_OFF:
case MP_POWERSHARE_DEFAULT:
case MP_POWERSHARE_AUTO:
case MP_POWERSHARE_LIMITED: //<<-- ADDED STORMIT - POWERSHARE Limit //
	{
		POSITION pos = selectedList.GetHeadPosition();
		while (pos != NULL)
		{
			file = selectedList.GetNext(pos);
			switch (wParam) {
case MP_POWERSHARE_DEFAULT:
	file->SetPowerShared(-1);
	break;
case MP_POWERSHARE_ON:
	file->SetPowerShared(1);
	break;
case MP_POWERSHARE_OFF:
	file->SetPowerShared(0);
	break;
case MP_POWERSHARE_AUTO:
	file->SetPowerShared(2);
								break;
//<<-- ADDED STORMIT - POWERSHARE Limit //
case MP_POWERSHARE_LIMITED:
	file->SetPowerShared(3);
	break;
//<<-- ADDED STORMIT - POWERSHARE Limit //
						}
			UpdateFile(file);
		}
		break;
					}
case MP_POWERSHARE_LIMIT:
case MP_POWERSHARE_LIMIT_SET:
	{
		POSITION pos = selectedList.GetHeadPosition();
		int newPowerShareLimit = -1;
		if (wParam==MP_POWERSHARE_LIMIT_SET)
		{
			InputBox inputbox;
			CString title=GetResString(IDS_POWERSHARE);
			CString currPowerShareLimit;
			if (file)
				currPowerShareLimit.Format(_T("%i"), (file->GetPowerShareLimit()>=0)?file->GetPowerShareLimit():thePrefs.GetPowerShareLimit());
			else
				currPowerShareLimit = _T("0");
			inputbox.SetLabels(GetResString(IDS_POWERSHARE), GetResString(IDS_POWERSHARE_LIMIT), currPowerShareLimit);
			inputbox.SetNumber(true);
			int result = inputbox.DoModal();
			if (result == IDCANCEL || (newPowerShareLimit = inputbox.GetInputInt()) < 0)
					break;
				}
				
		while (pos != NULL)
		{
			file = selectedList.GetNext(pos);
			if  (newPowerShareLimit == file->GetPowerShareLimit()) break;
			file->SetPowerShareLimit(newPowerShareLimit);
			file->UpdatePartsInfo();
			UpdateFile(file);
		}
		break;
	}
			case MP_SPREADBAR_DEFAULT:
			case MP_SPREADBAR_OFF:
			case MP_SPREADBAR_ON:
			{
				SetRedraw(FALSE);
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					switch (wParam) {
						case MP_SPREADBAR_DEFAULT:
							file->SetSpreadbarSetStatus(-1);
							break;
						case MP_SPREADBAR_OFF:
							file->SetSpreadbarSetStatus(0);
							break;
						case MP_SPREADBAR_ON:
							file->SetSpreadbarSetStatus(1);
							break;
			default:
							file->SetSpreadbarSetStatus(-1);
							break;
					}
					UpdateFile(file);
				}
				SetRedraw(TRUE);
				break;
			}
			case MP_SPREADBAR_RESET:
			{
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					file->statistic.ResetSpreadBar();
				}
				SetRedraw(TRUE);
			}
			case MP_HIDEOS_DEFAULT:
			case MP_HIDEOS_SET:
			{
				POSITION pos = selectedList.GetHeadPosition();
				int newHideOS = -1;
				if (wParam==MP_HIDEOS_SET)
				{
					InputBox inputbox;
					CString title=GetResString(IDS_HIDEOS);
					CString currHideOS;
					if (file)
						currHideOS.Format(_T("%i"), (file->GetHideOS()>=0)?file->GetHideOS():thePrefs.GetHideOvershares());
					else
						currHideOS = _T("0");
					inputbox.SetLabels(GetResString(IDS_HIDEOS), GetResString(IDS_HIDEOVERSHARES), currHideOS);
					inputbox.SetNumber(true);
					int result = inputbox.DoModal();
					if (result == IDCANCEL || (newHideOS = inputbox.GetInputInt()) < 0)
						break;
				}
				SetRedraw(FALSE);
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if  (newHideOS == file->GetHideOS()) break;
					file->SetHideOS(newHideOS);
					UpdateFile(file);
					}
				SetRedraw(TRUE);
					break;
				}	
			default:
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
				if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+256){
					theWebServices.RunURL(file, wParam);
				}
					else if (wParam>=MP_SELECTIVE_CHUNK && wParam<=MP_SELECTIVE_CHUNK_1){
						file->SetSelectiveChunk(wParam==MP_SELECTIVE_CHUNK?-1:wParam-MP_SELECTIVE_CHUNK_0);
						UpdateFile(file);
					}else if (wParam>=MP_SHAREONLYTHENEED && wParam<=MP_SHAREONLYTHENEED_1){
						file->SetShareOnlyTheNeed(wParam==MP_SHAREONLYTHENEED?-1:wParam-MP_SHAREONLYTHENEED_0);
						UpdateFile(file);
					}
				}
				break;
	// <--- Morph: PowerShare
//<<-- ADDED STORMIT - Morph: PowerShare //

		}
	}
	return TRUE;
}

void CSharedFilesCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column

	bool sortAscending = (GetSortItem() != pNMListView->iSubItem) ? true : !GetSortAscending();

	// Ornis 4-way-sorting
	int adder=0;
	if (pNMListView->iSubItem>=5 && pNMListView->iSubItem<=7)
	{
		ASSERT( pNMListView->iSubItem - 5 < ARRSIZE(sortstat) );
		if (!sortAscending)
			sortstat[pNMListView->iSubItem - 5] = !sortstat[pNMListView->iSubItem - 5];
		adder = sortstat[pNMListView->iSubItem-5] ? 0 : 100;
	}
	else if (pNMListView->iSubItem==11)
	{
		ASSERT( 3 < ARRSIZE(sortstat) );
		if (!sortAscending)
			sortstat[3] = !sortstat[3];
		adder = sortstat[3] ? 0 : 100;
	}

	// Sort table
	if (adder==0)	
		SetSortArrow(pNMListView->iSubItem, sortAscending); 
	else
		SetSortArrow(pNMListView->iSubItem, sortAscending ? arrowDoubleUp : arrowDoubleDown);

	UpdateSortHistory(pNMListView->iSubItem + adder + (sortAscending ? 0:20),20);
	SortItems(SortProc, pNMListView->iSubItem + adder + (sortAscending ? 0:20));

	*pResult = 0;
}

int CSharedFilesCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CKnownFile* item1 = (CKnownFile*)lParam1;
	const CKnownFile* item2 = (CKnownFile*)lParam2;
	
	int iResult=0;
	switch(lParamSort){
		case 0: //filename asc
			iResult=CompareLocaleStringNoCase(item1->GetFileName(),item2->GetFileName());
			break;
		case 20: //filename desc
			iResult=CompareLocaleStringNoCase(item2->GetFileName(),item1->GetFileName());
			break;

		case 1: //filesize asc
			iResult=item1->GetFileSize()==item2->GetFileSize()?0:(item1->GetFileSize()>item2->GetFileSize()?1:-1);
			break;

		case 21: //filesize desc
			iResult=item1->GetFileSize()==item2->GetFileSize()?0:(item2->GetFileSize()>item1->GetFileSize()?1:-1);
			break;

		case 2: //filetype asc
			iResult=item1->GetFileTypeDisplayStr().Compare(item2->GetFileTypeDisplayStr());
			break;
		case 22: //filetype desc
			iResult=item2->GetFileTypeDisplayStr().Compare(item1->GetFileTypeDisplayStr());
			break;

		case 3: //prio asc
		{
			uint8 p1=item1->GetUpPriority() +1;
			if(p1==5)
				p1=0;
			uint8 p2=item2->GetUpPriority() +1;
			if(p2==5)
				p2=0;
			iResult=p1-p2;
			break;
		}

			// Morph: PowerShare
			if (item1->GetPowerShared() == false && item2->GetPowerShared() == true)
				return -1;
			else if (item1->GetPowerShared() == true && item2->GetPowerShared() == false)
				return 1;
			else
				if(item1->GetUpPriority() == PR_VERYLOW && item2->GetUpPriority() != PR_VERYLOW)
					return -1;
				else if (item1->GetUpPriority() != PR_VERYLOW && item2->GetUpPriority() == PR_VERYLOW)
					return 1;
				else
					return item1->GetUpPriority()-item2->GetUpPriority();
//Telp Super Release
		{
			uint8 p1=item1->GetUpPriority() +1;
			if(p1==5)
				p1=0;
			uint8 p2=item2->GetUpPriority() +1;
			if(p2==5)
				p2=0;
			if (item1->IsReleaseFile())
				p1+=10;
			if (item2->IsReleaseFile())
				p2+=10;
			return p1-p2;
		}
			//Telp Super Release	
		case 23: //prio desc
			if (item2->GetPowerShared() == false && item1->GetPowerShared() == true)
				return -1;
			else if (item2->GetPowerShared() == true && item1->GetPowerShared() == false)
				return 1;
			else
				if(item2->GetUpPriority() == PR_VERYLOW && item1->GetUpPriority() != PR_VERYLOW )
					return -1;
				else if (item2->GetUpPriority() != PR_VERYLOW && item1->GetUpPriority() == PR_VERYLOW)
					return 1;
				else
					return item2->GetUpPriority()-item1->GetUpPriority();
				// <--- Morph: PowerShare
//<<-- ADDED STORMIT - Morph: PowerShare //
//Telp Super Release
		{
			uint8 p1=item1->GetUpPriority() +1;
			if(p1==5)
				p1=0;
			uint8 p2=item2->GetUpPriority() +1;
			if(p2==5)
				p2=0;
			if (item1->IsReleaseFile())
				p1+=10;
			if (item2->IsReleaseFile())
				p2+=10;
			return p2-p1;
		}
			//Telp Super Release
		case 4: //fileID asc
			iResult=memcmp(item1->GetFileHash(), item2->GetFileHash(), 16);
			break;
		case 24: //fileID desc
			iResult=memcmp(item2->GetFileHash(), item1->GetFileHash(), 16);
			break;

		case 5: //requests asc
			iResult=item1->statistic.GetRequests() - item2->statistic.GetRequests();
			break;
		case 25: //requests desc
			iResult=item2->statistic.GetRequests() - item1->statistic.GetRequests();
			break;
		
		case 6: //acc requests asc
			iResult=item1->statistic.GetAccepts() - item2->statistic.GetAccepts();
			break;
		case 26: //acc requests desc
			iResult=item2->statistic.GetAccepts() - item1->statistic.GetAccepts();
			break;
		
		case 7: //all transferred asc
			iResult=item1->statistic.GetTransferred()==item2->statistic.GetTransferred()?0:(item1->statistic.GetTransferred()>item2->statistic.GetTransferred()?1:-1);
			break;
		case 27: //all transferred desc
			iResult=item1->statistic.GetTransferred()==item2->statistic.GetTransferred()?0:(item2->statistic.GetTransferred()>item1->statistic.GetTransferred()?1:-1);
			break;

		case 9: //folder asc
			iResult=CompareLocaleStringNoCase(item1->GetPath(),item2->GetPath());
			break;
		case 29: //folder desc
			iResult=CompareLocaleStringNoCase(item2->GetPath(),item1->GetPath());
			break;

		case 10: //complete sources asc
			iResult=CompareUnsigned(item1->m_nCompleteSourcesCount, item2->m_nCompleteSourcesCount);
			break;
		case 30: //complete sources desc
			iResult=CompareUnsigned(item2->m_nCompleteSourcesCount, item1->m_nCompleteSourcesCount);
			break;

		case 11: //ed2k shared asc
			iResult=item1->GetPublishedED2K() - item2->GetPublishedED2K();
			break;
		case 31: //ed2k shared desc
			iResult=item2->GetPublishedED2K() - item1->GetPublishedED2K();
			break;
//<<-- ADDED STORMIT - Morph: PowerShare //
	// <--- Morph: PowerShare
		case 12:
			if (item1->GetPowerShared() == false && item2->GetPowerShared() == true)
				return -1;
			else if (item1->GetPowerShared() == true && item2->GetPowerShared() == false)
				iResult= 1;
			else
				if (item1->GetPowerSharedMode() != item2->GetPowerSharedMode())
					iResult= item1->GetPowerSharedMode() - item2->GetPowerSharedMode();
				else
					if (item1->GetPowerShareAuthorized() == false && item2->GetPowerShareAuthorized() == true)
						iResult= -1;
					else if (item1->GetPowerShareAuthorized() == true && item2->GetPowerShareAuthorized() == false)
						iResult= 1;
					else
						if (item1->GetPowerShareAuto() == false && item2->GetPowerShareAuto() == true)
							iResult= -1;
						else if (item1->GetPowerShareAuto() == true && item2->GetPowerShareAuto() == false)
							iResult= 1;
						else
//<<-- ADDED STORMIT - POWERSHARE Limit //
							if (item1->GetPowerShareLimited() == false && item2->GetPowerShareLimited() == true)
								iResult= -1;
							else if (item1->GetPowerShareLimited() == true && item2->GetPowerShareLimited() == false)
								iResult=1;
							else
//<<-- ADDED STORMIT - POWERSHARE Limit //
								iResult= 0;
		case 32:
			if (item2->GetPowerShared() == false && item1->GetPowerShared() == true)
				iResult= -1;
			else if (item2->GetPowerShared() == true && item1->GetPowerShared() == false)
				iResult= 1;
			else
				if (item2->GetPowerSharedMode() == 0 && item1->GetPowerSharedMode() != 0)
					iResult=-1;
				else if (item2->GetPowerSharedMode() == 1 && item1->GetPowerSharedMode() != 1)
					iResult= 1;
				else if (item2->GetPowerSharedMode() == 2 && item1->GetPowerSharedMode() != 2)
					iResult= 1-item1->GetPowerSharedMode();
				else
					if (item2->GetPowerShareAuthorized() == false && item1->GetPowerShareAuthorized() == true)
						iResult= -1;
					else if (item2->GetPowerShareAuthorized() == true && item1->GetPowerShareAuthorized() == false)
						iResult= 1;
					else
						if (item2->GetPowerShareAuto() == false && item1->GetPowerShareAuto() == true)
							iResult= -1;
						else if (item2->GetPowerShareAuto() == true && item1->GetPowerShareAuto() == false)
							iResult=1;
						else
//<<-- ADDED STORMIT - POWERSHARE Limit //
							if (item2->GetPowerShareLimited() == false && item1->GetPowerShareLimited() == true)
								iResult= -1;
							else if (item2->GetPowerShareLimited() == true && item1->GetPowerShareLimited() == false)
								iResult= 1;
							else
//<<-- ADDED STORMIT - POWERSHARE Limit //
								iResult=
 0;
//<<-- ADDED STORMIT - Morph: PowerShare //
	// <--- Morph: PowerShare

		case 13: //spread asc
		case 14:
			iResult= 10000*(((CKnownFile*)lParam1)->statistic.GetSpreadSortValue()-((CKnownFile*)lParam2)->statistic.GetSpreadSortValue());
		case 33: //spread desc
		case 34:
			iResult= 10000*(((CKnownFile*)lParam2)->statistic.GetSpreadSortValue()-((CKnownFile*)lParam1)->statistic.GetSpreadSortValue());

		case 15: // VQB:  Simple UL asc
		case 35: //VQB:  Simple UL desc
			{
				float x1 = ((float)item1->statistic.GetAllTimeTransferred())/((float)item1->GetFileSize());
				float x2 = ((float)item2->statistic.GetAllTimeTransferred())/((float)item2->GetFileSize());
				if (lParamSort == 16) return 10000*(x1-x2); else return 10000*(x2-x1);
			}
		case 16: // SF:  Full Upload Count asc
			iResult= 10000*(((CKnownFile*)lParam1)->statistic.GetFullSpreadCount()-((CKnownFile*)lParam2)->statistic.GetFullSpreadCount());
		case 36: // SF:  Full Upload Count desc
			return 10000*(((CKnownFile*)lParam2)->statistic.GetFullSpreadCount()-((CKnownFile*)lParam1)->statistic.GetFullSpreadCount());

break;//<<-- ADDED STORMIT - SLUGFILLER: Spreadbars //

//<<-- ADDED STORMIT - SLUGFILLER HIDEOS //
		case 17:
			if (item1->GetHideOS() == item2->GetHideOS())
				iResult= item1->GetSelectiveChunk() - item2->GetSelectiveChunk();
			else
				iResult= item1->GetHideOS() - item2->GetHideOS();
		case 37:
			if (item2->GetHideOS() == item1->GetHideOS())
				iResult= item2->GetSelectiveChunk() - item1->GetSelectiveChunk();
			else
				iResult= item2->GetHideOS() - item1->GetHideOS();
//<<-- ADDED STORMIT - SLUGFILLER HIDEOS //

//<<-- ADDED STORMIT - SHARE_ONLY_THE_NEED //
		case 18:
			iResult= item1->GetShareOnlyTheNeed() - item2->GetShareOnlyTheNeed();
	
break;	case 38:
			iResult= item2->GetShareOnlyTheNeed() - item1->GetShareOnlyTheNeed();
break;
//<<-- ADDED STORMIT - SHARE_ONLY_THE_NEED //
//<<-- ADDED STORMIT - Morph: PowerShare //
			// xMule_MOD: showSharePermissions
		case 19: //permission asc
			return item2->GetPermissions()-item1->GetPermissions();
		case 39: //permission desc
			return item1->GetPermissions()-item2->GetPermissions();
		// xMule_MOD: showSharePermissions
		//FRTK(kts)-

		case 105: //all requests asc
			iResult=CompareUnsigned(item1->statistic.GetAllTimeRequests(), item2->statistic.GetAllTimeRequests());
			break;
		case 125: //all requests desc
			iResult=CompareUnsigned(item2->statistic.GetAllTimeRequests(), item1->statistic.GetAllTimeRequests());
			break;

		case 106: //all acc requests asc
			iResult=CompareUnsigned(item1->statistic.GetAllTimeAccepts(), item2->statistic.GetAllTimeAccepts());
			break;
		case 126: //all acc requests desc
			iResult=CompareUnsigned(item2->statistic.GetAllTimeAccepts(), item1->statistic.GetAllTimeAccepts());
			break;

		case 107: //all transferred asc
			iResult=item1->statistic.GetAllTimeTransferred()==item2->statistic.GetAllTimeTransferred()?0:(item1->statistic.GetAllTimeTransferred()>item2->statistic.GetAllTimeTransferred()?1:-1);
			break;
		case 127: //all transferred desc
			iResult=item1->statistic.GetAllTimeTransferred()==item2->statistic.GetAllTimeTransferred()?0:(item2->statistic.GetAllTimeTransferred()>item1->statistic.GetAllTimeTransferred()?1:-1);
			break;

		case 111:{ //kad shared asc
			uint32 tNow = time(NULL);
			int i1 = (tNow < item1->GetLastPublishTimeKadSrc()) ? 1 : 0;
			int i2 = (tNow < item2->GetLastPublishTimeKadSrc()) ? 1 : 0;
			iResult=i1 - i2;
			break;
		}
		case 131:{ //kad shared desc
			uint32 tNow = time(NULL);
			int i1 = (tNow < item1->GetLastPublishTimeKadSrc()) ? 1 : 0;
			int i2 = (tNow < item2->GetLastPublishTimeKadSrc()) ? 1 : 0;
			iResult=i2 - i1;
			break;
		}
		default: 
			iResult=0;
			break;
	}
	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->sharedfileswnd->sharedfilesctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}

	return iResult;

}

void CSharedFilesCtrl::OpenFile(const CKnownFile* file)
{
	if(file->m_pCollection)
	{
		CCollectionViewDialog dialog;
		dialog.SetCollection(file->m_pCollection);
		dialog.DoModal();
	}
	else
		ShellOpenFile(file->GetFilePath(), NULL);
}

void CSharedFilesCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		CKnownFile* file = (CKnownFile*)GetItemData(iSel);
		if (file)
		{
			if (GetKeyState(VK_MENU) & 0x8000)
			{
				CTypedPtrList<CPtrList, CKnownFile*> aFiles;
				aFiles.AddHead(file);
				ShowFileDialog(aFiles);
			}
			else if (!file->IsPartFile())
				OpenFile(file);
		}
	}
	*pResult = 0;
}

void CSharedFilesCtrl::CreateMenues()
{
	// Morph: PowerShare
    if (m_PowershareMenu) VERIFY( m_PowershareMenu.DestroyMenu() );
	if (m_PowerShareLimitMenu) VERIFY( m_PowerShareLimitMenu.DestroyMenu() );
	if (m_HideOSMenu) VERIFY( m_HideOSMenu.DestroyMenu() );
	if (m_SelectiveChunkMenu) VERIFY( m_SelectiveChunkMenu.DestroyMenu() );
	if (m_ShareOnlyTheNeedMenu) VERIFY( m_ShareOnlyTheNeedMenu.DestroyMenu() );
	// <--- Morph: PowerShare
	if(m_SpreadbarMenu)	VERIFY(m_SpreadbarMenu.DestroyMenu());

	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_CollectionsMenu) VERIFY( m_CollectionsMenu.DestroyMenu() );
	//FRTK(kts)+
	if (m_PermMenu) VERIFY( m_PermMenu.DestroyMenu() );	// xMule_MOD: showSharePermissions
	//FRTK(kts)-
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );

	m_PrioMenu.CreateMenu();
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYLOW,GetResString(IDS_PRIOVERYLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYHIGH, GetResString(IDS_PRIORELEASE));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));//UAP
	//KTS+ webcache
	m_PrioMenu.AppendMenu(MF_STRING|MF_SEPARATOR);//jp webcache release
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOWCRELEASE, _T("WC-Release"));//jp webcache release
	//KTS- webcache
//Telp Super Release
	m_PrioMenu.AppendMenu(MF_SEPARATOR);
	m_PrioMenu.AppendMenu(MF_STRING,MP_RELEASESET, GetResString(IDS_RELEASESET));
	m_PrioMenu.AppendMenu(MF_STRING,MP_RELEASEREMOVE, GetResString(IDS_RELEASEREMOVE));
	//Telp Super Release
//<<-- ADDED STORMIT - Morph: PowerShare //
// add powershare switcher
	m_PowershareMenu.CreateMenu();
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_DEFAULT,GetResString(IDS_DEFAULT));
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_OFF,GetResString(IDS_POWERSHARE_OFF_LABEL));
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_ON,GetResString(IDS_POWERSHARE_ON_LABEL));

//<<-- ADDED STORMIT - Avoid misusing of powersharing //
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_AUTO,GetResString(IDS_POWERSHARE_AUTO));
//<<-- ADDED STORMIT - Avoid misusing of powersharing //

//<<-- ADDED STORMIT - POWERSHARE Limit //
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_LIMITED,GetResString(IDS_POWERSHARE_LIMITED));
	m_PowerShareLimitMenu.CreateMenu();
	m_PowerShareLimitMenu.AppendMenu(MF_STRING,MP_POWERSHARE_LIMIT,	GetResString(IDS_DEFAULT));
	m_PowerShareLimitMenu.AppendMenu(MF_STRING,MP_POWERSHARE_LIMIT_SET,	GetResString(IDS_DISABLED));
//<<-- ADDED STORMIT - POWERSHARE Limit //
//<<-- ADDED STORMIT - ZZ Upload System //
	m_SpreadbarMenu.CreateMenu();
	m_SpreadbarMenu.AppendMenu(MF_STRING,MP_SPREADBAR_DEFAULT, GetResString(IDS_DEFAULT));
	m_SpreadbarMenu.AppendMenu(MF_STRING,MP_SPREADBAR_OFF, GetResString(IDS_DISABLED));
	m_SpreadbarMenu.AppendMenu(MF_STRING,MP_SPREADBAR_ON, GetResString(IDS_ENABLED));
	m_SpreadbarMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
	m_SpreadbarMenu.AppendMenu(MF_STRING,MP_SPREADBAR_RESET, GetResString(IDS_RESET));

//<<-- ADDED STORMIT - SLUGFILLER HIDEOS //
	m_HideOSMenu.CreateMenu();
	m_HideOSMenu.AppendMenu(MF_STRING,MP_HIDEOS_DEFAULT, GetResString(IDS_DEFAULT));
	m_HideOSMenu.AppendMenu(MF_STRING,MP_HIDEOS_SET, GetResString(IDS_DISABLED));
	m_SelectiveChunkMenu.CreateMenu();
	m_SelectiveChunkMenu.AppendMenu(MF_STRING,MP_SELECTIVE_CHUNK,	GetResString(IDS_DEFAULT));
	m_SelectiveChunkMenu.AppendMenu(MF_STRING,MP_SELECTIVE_CHUNK_0,	GetResString(IDS_DISABLED));
	m_SelectiveChunkMenu.AppendMenu(MF_STRING,MP_SELECTIVE_CHUNK_1,	GetResString(IDS_ENABLED));
//<<-- ADDED STORMIT - SLUGFILLER HIDEOS //

//<<-- ADDED STORMIT - SHARE_ONLY_THE_NEED //
	m_ShareOnlyTheNeedMenu.CreateMenu();
	m_ShareOnlyTheNeedMenu.AppendMenu(MF_STRING,MP_SHAREONLYTHENEED,	GetResString(IDS_DEFAULT));
	m_ShareOnlyTheNeedMenu.AppendMenu(MF_STRING,MP_SHAREONLYTHENEED_0,	GetResString(IDS_DISABLED));
	m_ShareOnlyTheNeedMenu.AppendMenu(MF_STRING,MP_SHAREONLYTHENEED_1,	GetResString(IDS_ENABLED));
//<<-- ADDED STORMIT - SHARE_ONLY_THE_NEED //

	m_CollectionsMenu.CreateMenu();
	m_CollectionsMenu.AddMenuTitle(NULL, true);
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_CREATECOLLECTION, GetResString(IDS_CREATECOLLECTION), _T("COLLECTION_ADD"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_MODIFYCOLLECTION, GetResString(IDS_MODIFYCOLLECTION), _T("COLLECTION_EDIT"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_VIEWCOLLECTION, GetResString(IDS_VIEWCOLLECTION), _T("COLLECTION_VIEW"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_SEARCHAUTHOR, GetResString(IDS_SEARCHAUTHORCOLLECTION), _T("COLLECTION_SEARCH"));
//FRTK(kts)+
	// xMule_MOD: showSharePermissions
	m_PermMenu.CreateMenu();
	m_PermMenu.AppendMenu(MF_STRING,MP_PERMNONE, GetResString(IDS_HIDDEN));
	m_PermMenu.AppendMenu(MF_STRING,MP_PERMFRIENDS, GetResString(IDS_FSTATUS_FRIENDSONLY));
	m_PermMenu.AppendMenu(MF_STRING,MP_PERMALL, GetResString(IDS_FSTATUS_PUBLIC));
	// xMule_MOD: showSharePermissions
	//FRTK(kts)-
	m_SharedFilesMenu.CreatePopupMenu();
	m_SharedFilesMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES), true);

	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_OPENFILE), _T("OPENFILE"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_RENAME, GetResString(IDS_RENAME) + _T("..."), _T("FILERENAME"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_REMOVE, GetResString(IDS_DELETE), _T("DELETE"));
	if (thePrefs.IsExtControlsEnabled())

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"), _T("FILEPRIORITY"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);


//<<-- ADDED STORMIT - Morph: PowerShare //
//<<-- ADDED STORMIT - ZZ Upload System //
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PowershareMenu.m_hMenu, GetResString(IDS_POWERSHARE), _T("POWERSHARED"));
//<<-- ADDED STORMIT -ZZ Upload System //

//<<-- ADDED STORMIT - POWERSHARE Limit //
	m_PowershareMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_PowershareMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PowerShareLimitMenu.m_hMenu, GetResString(IDS_POWERSHARE_LIMIT));
//<<-- ADDED STORMIT - END - POWERSHARE Limit //
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_SpreadbarMenu.m_hMenu, GetResString(IDS_SPREADBAR), _T("FILESPREADBAR"));

//<<-- ADDED STORMIT - START -  SLUGFILLER HIDEOS //
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_HideOSMenu.m_hMenu, GetResString(IDS_HIDEOS), _T("CONTACT4"));
	m_HideOSMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_HideOSMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_SelectiveChunkMenu.m_hMenu, GetResString(IDS_SELECTIVESHARE));
 //<<-- ADDED STORMIT - SLUGFILLER HIDEOS //

//<<-- ADDED STORMIT - START - SHARE_ONLY_THE_NEED //
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_ShareOnlyTheNeedMenu.m_hMenu, GetResString(IDS_SHAREONLYTHENEED), _T("SHAREDFILES"));
//<<-- ADDED STORMIT - END - SHARE_ONLY_THE_NEED //

//<<-- ADDED STORMIT - Morph: PowerShare //

	//FRTK(kts)+
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PermMenu.m_hMenu, GetResString(IDS_PERMISSION), _T("SECURITY"));	// xMule_MOD: showSharePermissions
	//FRTK(kts)-


	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_CollectionsMenu.m_hMenu, GetResString(IDS_META_COLLECTION), _T("COLLECTION"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 	

	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("FILEINFO"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_CMT, GetResString(IDS_CMT_ADD), _T("FILECOMMENTS")); 
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK") );
	else
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK") );
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 


#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
		//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETKADSOURCELINK, _T("Copy eD2K Links To Clipboard (Kad)"));
		m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	}
#endif
//MORPH START - Added by SiRoB, copy feedback feature
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_COPYFEEDBACK, GetResString(IDS_COPYFEEDBACK), _T("COPY"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_COPYFEEDBACK_NOCODE, GetResString(IDS_COPYFEEDBACK_NOCODE), _T("COPY"));
	m_SharedFilesMenu.AppendMenu(MF_SEPARATOR);
	//MORPH END   - Added by SiRoB, copy feedback feature
}

void CSharedFilesCtrl::ShowComments(CKnownFile* file)
{
	if (file)
	{
		CTypedPtrList<CPtrList, CKnownFile*> aFiles;
		aFiles.AddHead(file);
		ShowFileDialog(aFiles, IDD_COMMENT);
	}
}

void CSharedFilesCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
			const CKnownFile* pFile = reinterpret_cast<CKnownFile*>(pDispInfo->item.lParam);
			if (pFile != NULL){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, pFile->GetFileName(), pDispInfo->item.cchTextMax);
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

void CSharedFilesCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'C' && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		// Ctrl+C: Copy listview items to clipboard
		SendMessage(WM_COMMAND, MP_GETED2KLINK);
		return;
	}
	else if (nChar == VK_F5)
		ReloadFileList();

	CMuleListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSharedFilesCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage)
{
	if (aFiles.GetSize() > 0)
	{
		CFileDetailDialog dialog(aFiles, uPshInvokePage, this);
		dialog.DoModal();
	}
}

void CSharedFilesCtrl::SetDirectoryFilter(CDirectoryItem* pNewFilter, bool bRefresh){
	if (m_pDirectoryFilter == pNewFilter)
		return;
	m_pDirectoryFilter = pNewFilter;
	if (bRefresh)
		ReloadFileList();
}
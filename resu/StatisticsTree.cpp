/*	CStatisticsTree Class Implementation File by Khaos
	Copyright (C) 2003

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	This file is a part of the KX mod, and more
	specifically, it is a part of my statistics
	add-on.

	The purpose of deriving a custom class from CTreeCtrl
	was to provide another level of customization and control.
	This allows us to easily code complicated parsing features
	and a context menu.
*/
#include "stdafx.h"
#include "emule.h"
#include "StatisticsTree.h"
#include "StatisticsDlg.h"
#include "Statistics.h"
#include "emuledlg.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "Log.h"
#include "StringConversion.h"
#include "opcodes.h"
#include "clientlist.h"
#include "downloadqueue.h"
#include "sharedfilelist.h"
#include "uploadqueue.h"
#include "downloadlistctrl.h"
#include "./SysInfo/SystemInfo.h"
#include "transferwnd.h"
#include "./SysInfo/SysInfo.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CStatisticsTree, CTreeCtrl)

BEGIN_MESSAGE_MAP(CStatisticsTree, CTreeCtrl)
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemExpanded)
END_MESSAGE_MAP()

CStatisticsTree::CStatisticsTree()
{
}

CStatisticsTree::~CStatisticsTree()
{
	if (mnuHTML) VERIFY( mnuHTML.DestroyMenu() );
	if (mnuContext) VERIFY( mnuContext.DestroyMenu() );
}

// This function is called from CStatisticsDlg::OnInitDialog in StatisticsDlg.cpp
void CStatisticsTree::Init()
{
	m_bExpandingAll = false;
}

// It is necessary to disrupt whatever behavior was preventing
// us from getting OnContextMenu to work.  This seems to be the
// magic fix...
void CStatisticsTree::OnRButtonDown( UINT nFlags, CPoint point )
{
	UINT uHitFlags;
	HTREEITEM hItem = HitTest(point, &uHitFlags);
	if (hItem != NULL && (uHitFlags & TVHT_ONITEM))
	{
		Select(hItem, TVGN_CARET);
		SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
	}
	return;
}

void CStatisticsTree::OnContextMenu( CWnd* pWnd, CPoint point )
{
	DoMenu(point, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
}

void CStatisticsTree::OnLButtonUp( UINT nFlags, CPoint point )
{
	theApp.emuledlg->statisticswnd->ShowStatistics();
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

// This function saves the expanded tree items intelligently.  Instead
// of saving them every time we ShowStatistics, now they are only saved
// when a parent item is expanded or collapsed.
// m_bExpandingAll is TRUE when CollapseAll, ExpandAll or ApplyExpandedMask
// are executing.  This is to prevent us from saving the string a bajillion
// times whenever these functions are called.  CollapseAll and ExpandAll
// call GetExpandedMask() upon completion.
void CStatisticsTree::OnItemExpanded( NMHDR* pNMHDR, LRESULT* pResult )
{
	if (!m_bExpandingAll)
		thePrefs.SetExpandedTreeItems(GetExpandedMask());
}

// Displays the command menu.  This function is overloaded
// because it is used both to display the context menu and also
// the menu that drops down from the button.
void CStatisticsTree::DoMenu()
{
	CPoint cursorPos;
	GetCursorPos(&cursorPos);
	DoMenu(cursorPos);
}

void CStatisticsTree::DoMenu(CPoint doWhere)
{
	DoMenu( doWhere, TPM_RIGHTALIGN | TPM_RIGHTBUTTON );
}

void CStatisticsTree::DoMenu(CPoint doWhere, UINT nFlags)
{
	CFileFind	findBackUp;
	CString		myBuffer;
	int			myFlags;

	myBuffer.Format(_T("%sstatbkup.ini"),thePrefs.GetConfigDir());
	if (!findBackUp.FindFile(myBuffer)) myFlags = MF_GRAYED;
		else myFlags = MF_STRING;

	mnuContext.CreatePopupMenu();
	mnuContext.AddMenuTitle(GetResString(IDS_STATS_MNUTREETITLE), true);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_RESET, GetResString(IDS_STATS_BNRESET), _T("DELETE"));
	mnuContext.AppendMenu(myFlags, MP_STATTREE_RESTORE, GetResString(IDS_STATS_BNRESTORE), _T("RESTORE"));
	mnuContext.AppendMenu(MF_SEPARATOR);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_EXPANDMAIN, GetResString(IDS_STATS_MNUTREEEXPANDMAIN), _T("EXPANDMAIN"));
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_EXPANDALL, GetResString(IDS_STATS_MNUTREEEXPANDALL), _T("EXPANDALL"));
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COLLAPSEALL, GetResString(IDS_STATS_MNUTREECOLLAPSEALL), _T("COLLAPSE"));
	mnuContext.AppendMenu(MF_SEPARATOR);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COPYSEL, GetResString(IDS_STATS_MNUTREECPYSEL), _T("COPY"));
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COPYVIS, GetResString(IDS_STATS_MNUTREECPYVIS), _T("COPYVISIBLE"));
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COPYALL, GetResString(IDS_STATS_MNUTREECPYALL), _T("COPYSELECTED"));
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COPYLAMASTAT, GetResString(IDS_STATS_LAMASTAT), _T("COPYLAMASTAT"));
	mnuContext.AppendMenu(MF_SEPARATOR);

	mnuHTML.CreateMenu();
	mnuHTML.AddMenuTitle(NULL, true);
	mnuHTML.AppendMenu(MF_STRING, MP_STATTREE_HTMLCOPYSEL, GetResString(IDS_STATS_MNUTREECPYSEL), _T("COPY"));
	mnuHTML.AppendMenu(MF_STRING, MP_STATTREE_HTMLCOPYVIS, GetResString(IDS_STATS_MNUTREECPYVIS), _T("COPYVISIBLE"));
	mnuHTML.AppendMenu(MF_STRING, MP_STATTREE_HTMLCOPYALL, GetResString(IDS_STATS_MNUTREECPYALL), _T("COPYSELECTED"));
	mnuHTML.AppendMenu(MF_SEPARATOR);
	mnuHTML.AppendMenu(MF_STRING, MP_STATTREE_HTMLEXPORT, GetResString(IDS_STATS_EXPORT2HTML), _T("EXPORTALL"));
	mnuContext.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)mnuHTML.m_hMenu, GetResString(IDS_STATS_MNUTREEHTML), _T("WEB"));

	GetPopupMenuPos(*this, doWhere);
	mnuContext.TrackPopupMenu(nFlags, doWhere.x, doWhere.y, this);

	VERIFY( mnuHTML.DestroyMenu() );
	VERIFY( mnuContext.DestroyMenu() );
}

// Process context menu items...
BOOL CStatisticsTree::OnCommand( WPARAM wParam, LPARAM lParam )
{
	switch (wParam) {
		case MP_STATTREE_RESET:
			{
				if(AfxMessageBox(GetResString(IDS_STATS_MBRESET_TXT), MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
					break;

				thePrefs.ResetCumulativeStatistics();
				AddLogLine(false, GetResString(IDS_STATS_NFORESET));
				theApp.emuledlg->statisticswnd->ShowStatistics();

				CString myBuffer; 
				myBuffer.Format(GetResString(IDS_STATS_LASTRESETSTATIC), thePrefs.GetStatsLastResetStr(false));
				GetParent()->GetDlgItem(IDC_STATIC_LASTRESET)->SetWindowText(myBuffer);

				break;
			}
		case MP_STATTREE_RESTORE:
			{
				if (AfxMessageBox(GetResString(IDS_STATS_MBRESTORE_TXT), MB_YESNO | MB_ICONQUESTION) == IDNO)
					break;

				if(!thePrefs.LoadStats(1))
					LogError(LOG_STATUSBAR, GetResString(IDS_ERR_NOSTATBKUP));
				else {
					AddLogLine(false, GetResString(IDS_STATS_NFOLOADEDBKUP));
					CString myBuffer;
					myBuffer.Format(GetResString(IDS_STATS_LASTRESETSTATIC), thePrefs.GetStatsLastResetStr(false));
					GetParent()->GetDlgItem(IDC_STATIC_LASTRESET)->SetWindowText(myBuffer);
				}

				break;
			}
		case MP_STATTREE_EXPANDMAIN:
			{
				SetRedraw(false);
				ExpandAll(true);
				goto lblSaveExpanded;
			}
		case MP_STATTREE_EXPANDALL:
			{
				SetRedraw(false);
				ExpandAll();
				goto lblSaveExpanded;
			}
		case MP_STATTREE_COLLAPSEALL:
			{
				SetRedraw(false);
				CollapseAll();
lblSaveExpanded:
				thePrefs.SetExpandedTreeItems(GetExpandedMask());
				SetRedraw(true);
				break;
			}
		case MP_STATTREE_COPYSEL:
		case MP_STATTREE_COPYVIS:
		case MP_STATTREE_COPYALL:
		case MP_STATTREE_COPYLAMASTAT:
			{
				CopyText(wParam);
				break;
			}
		case MP_STATTREE_HTMLCOPYSEL:
		case MP_STATTREE_HTMLCOPYVIS:
		case MP_STATTREE_HTMLCOPYALL:
			{
				CopyHTML(wParam);
				break;
			}
		case MP_STATTREE_HTMLEXPORT:
			{
				ExportHTML();
				break;
			}
	}

	return true;
}

// If the item is bold it returns true, otherwise
// false.  Very straightforward.
// EX: if(IsBold(myTreeItem)) AfxMessageBox("It's bold.");
BOOL CStatisticsTree::IsBold(HTREEITEM theItem)
{
	UINT stateBold = GetItemState(theItem, TVIS_BOLD);
	return (stateBold & TVIS_BOLD);
}

// If the item is expanded it returns true, otherwise
// false.  Very straightforward.
// EX: if(IsExpanded(myTreeItem)) AfxMessageBox("It's expanded.");
BOOL CStatisticsTree::IsExpanded(HTREEITEM theItem)
{
	UINT stateExpanded = GetItemState(theItem, TVIS_EXPANDED);
	return (stateExpanded & TVIS_EXPANDED);
}

// This is a generic function to check if a state is valid or not.
// It accepts a tree item handle and a state/statemask/whatever.
// It then retrieves the state UINT value and does a bitand
// with the original input.  This should translate into a
// boolean result that tells us whether the checked state is
// true or not.  This is currently unused, but may come in handy
// for states other than bold and expanded.
// EX:  if(CheckState(myTreeItem, TVIS_BOLD)) AfxMessageBox("It's bold.");
BOOL CStatisticsTree::CheckState(HTREEITEM hItem, UINT state)
{
	UINT stateGeneric = GetItemState(hItem, state);
	return (stateGeneric & state);
}

// Returns the entire text label of an HTREEITEM.  This
// is an overloaded function.
// EX: CString itemText = GetItemText(myTreeItem);
CString CStatisticsTree::GetItemText(HTREEITEM theItem)
{
	if (theItem == NULL)
		return _T("");

	TVITEM item;
	TCHAR szText[1024]; 
	item.mask = TVIF_TEXT | TVIF_HANDLE;
	item.hItem = theItem;
	item.pszText = szText;
	item.cchTextMax = 1024;

	if (GetItem(&item))
		return CString(item.pszText);

	return _T("");
}

// This seperates the title from the value in a tree item that has
// a title to the left of a colon, and a value to the right, with
// a space seperating the value from the colon. ": "
// int getPart can be GET_TITLE (0) or GET_VALUE (1)
// EXAMPLE:
// HTREEITEM hMyItem = treeCtrl.InsertItem("Title: 5", hMyParent);
// CString strTitle = treeCtrl.GetItemText(hMyItem, GET_TITLE);
// CString strValue = treeCtrl.GetItemText(hMyItem, GET_VALUE);
// AfxMessageBox("The title is: " + strTitle + "\nThe value is: " + strValue);
CString CStatisticsTree::GetItemText(HTREEITEM theItem, int getPart)
{
	if (theItem == NULL)
		return _T("");

	CString fullText, returnText;
	fullText = GetItemText(theItem);

	if (fullText.IsEmpty())
		return _T("");

	int posSeparator = fullText.Find(_T(": "));
	
	if (posSeparator < 1) {
		returnText = getPart == GET_TITLE ? fullText : _T("");
		return returnText;
	}

	if (getPart == GET_TITLE)
		returnText = fullText.Left(posSeparator);
	else if (getPart == GET_VALUE)
		returnText = fullText.Mid(posSeparator + 2);
	else
		returnText = _T("");

	return returnText;
}

// This is the primary function for generating HTML output of the statistics tree.
// It is recursive.
CString CStatisticsTree::GetHTML(bool onlyVisible, HTREEITEM theItem, int theItemLevel, bool firstItem)
{
	HTREEITEM hCurrent;
	if (theItem == NULL)
	{
		if (!onlyVisible)
			theApp.emuledlg->statisticswnd->ShowStatistics(true);
		hCurrent = GetRootItem(); // Copy All Vis or Copy All
	}
	else
		hCurrent = theItem;

	CString	strBuffer;
	if (firstItem)
		strBuffer.Format(_T("<font face=\"Tahoma,Verdana,Courier New,Helvetica\" size=\"2\">\r\n<b>eMule v%s %s [%s]</b>\r\n<br><br>\r\n"), theApp.m_strCurVersionLong, GetResString(IDS_SF_STATISTICS), thePrefs.GetUserNick());

	while (hCurrent != NULL)
	{
		CString	strItem;
		if (IsBold(hCurrent))
			strItem = _T("<b>") + GetItemText(hCurrent) + _T("</b>");
		else
			strItem = GetItemText(hCurrent);
		for (int i = 0; i < theItemLevel; i++)
			strBuffer += _T("&nbsp;&nbsp;&nbsp;");
		if (theItemLevel == 0)
			strBuffer.Append(_T("\n"));
		strBuffer += strItem + _T("<br>");
		if (ItemHasChildren(hCurrent) && (!onlyVisible || IsExpanded(hCurrent)))
			strBuffer += GetHTML(onlyVisible, GetChildItem(hCurrent), theItemLevel+1, false);
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
		if (firstItem && theItem != NULL)
			break; // Copy Selected Branch was used, so we don't want to copy all branches at this level.  Only the one that was selected.
	}
	if (firstItem)
		strBuffer += _T("</font>");
	return strBuffer;
}

// Takes the HTML output generated by GetHTML
// and puts it on the clipboard.  Simplenuff.
bool CStatisticsTree::CopyHTML(int copyMode)
{
	switch (copyMode) {
		case MP_STATTREE_HTMLCOPYSEL:
			{
				HTREEITEM selectedItem = GetSelectedItem();
				if (selectedItem != NULL) {
					CString theHTML = GetHTML(true, selectedItem);
					if (theHTML.IsEmpty())
						return false;
					theApp.CopyTextToClipboard(theHTML);
					return true;
				}
				return false;
			}
		case MP_STATTREE_HTMLCOPYVIS:
			{
				CString theHTML = GetHTML();
				if (theHTML.IsEmpty())
					return false;
				theApp.CopyTextToClipboard(theHTML);
				return true;
			}
		case MP_STATTREE_HTMLCOPYALL:
			{
				CString theHTML = GetHTML(false);
				if (theHTML.IsEmpty())
					return false;
				theApp.CopyTextToClipboard(theHTML);
				return true;
			}
	}

	return false;
}

CString CStatisticsTree::GetText(bool onlyVisible, HTREEITEM theItem, int theItemLevel, bool firstItem)
{
	bool bPrintHeader = firstItem;
	HTREEITEM hCurrent;
	if (theItem == NULL)
	{
		hCurrent = GetRootItem(); // Copy All Vis or Copy All
	}
	else
	{
		if (bPrintHeader && (!ItemHasChildren(theItem) || !IsExpanded(theItem)))
			bPrintHeader = false;
		hCurrent = theItem;
	}

	CString	strBuffer;
	if (bPrintHeader)
		strBuffer.Format(_T("eMule v%s %s [%s]\r\n\r\n"), theApp.m_strCurVersionLong, GetResString(IDS_SF_STATISTICS) ,thePrefs.GetUserNick());

	while (hCurrent != NULL)
	{
		for (int i = 0; i < theItemLevel; i++)
			strBuffer += _T("   ");
		strBuffer += GetItemText(hCurrent);
		if (bPrintHeader || !firstItem)
			strBuffer += _T("\r\n");
		if (ItemHasChildren(hCurrent) && (!onlyVisible || IsExpanded(hCurrent)))
			strBuffer += GetText(onlyVisible, GetChildItem(hCurrent), theItemLevel+1, false);
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
		if (firstItem && theItem != NULL)
			break; // Copy Selected Branch was used, so we don't want to copy all branches at this level.  Only the one that was selected.
	}
	return strBuffer;
}

// Doh-nuts.
bool CStatisticsTree::CopyText(int copyMode)
{
	switch (copyMode) {
		case MP_STATTREE_COPYSEL:
			{
				HTREEITEM selectedItem = GetSelectedItem();
				if (selectedItem != NULL) {
					CString theText = GetText(true, selectedItem);
					if (theText.IsEmpty())
						return false;
					theApp.CopyTextToClipboard(theText);
					return true;
				}
				return false;
			}
		case MP_STATTREE_COPYVIS:
			{
				CString theText = GetText();
				if (theText.IsEmpty())
					return false;
				theApp.CopyTextToClipboard(theText);
				return true;
			}
		case MP_STATTREE_COPYALL:
			{
				CString theText = GetText(false);
				if (theText.IsEmpty())
					return false;
				theApp.CopyTextToClipboard(theText);
				return true;
			}
		case MP_STATTREE_COPYLAMASTAT:
					{
	CString fstats;
fstats.AppendFormat(_T("Basic System Infos:\r\n"));
fstats.AppendFormat(_T("-------------------\r\n"));
fstats.AppendFormat(_T("|--> Emule Nickname: %s \r\n"),thePrefs.GetUserNick());
fstats.AppendFormat(_T("|--> running Mod: %s \r\n"),MOD_VERSION);
fstats.AppendFormat(_T("|--> CPU Usage: %3d%% \r\n"), theApp.sysinfo->GetCpuUsage());
fstats.AppendFormat(_T("|--> Mem Usage: %.fMb \r\n"),(double)theApp.sysinfo->GetMemoryUsage()/1024);
fstats.AppendFormat(_T("|--> CPU Infos: %s %s %s @%3d%Mhz \r\n"),theApp.sysinfo->GetCPUNameString(),theApp.sysinfo->GetCPUVendorIdentifier(),theApp.sysinfo-> GetCPUIdentifier(),theApp.sysinfo->GetCPUSpeed());
fstats.AppendFormat(_T("|--> OS Info : %s Build:%u \r\n"), theApp.sysinfo->GetOSType(),theApp.sysinfo->GetBuildNumber());
fstats.AppendFormat(_T("\r\n"));

fstats.AppendFormat(_T("Emule Settings:\r\n"));
fstats.AppendFormat(_T("---------------\r\n"));
fstats.AppendFormat(_T("|--> Maxup: %u.00 (Limit: %u.00) \r\n"),thePrefs.GetMaxGraphUploadRate(true),thePrefs.GetMaxUpload());
fstats.AppendFormat(_T("|--> Maxdown: %u.00 (Limit: %u.00) \r\n"),thePrefs.GetMaxGraphDownloadRate(),thePrefs.maxdownload);
fstats.AppendFormat(_T("|--> Hardlimit: %u \r\n"),thePrefs.maxsourceperfile,theApp.emuledlg->transferwnd->downloadlistctrl);
fstats.AppendFormat(_T("|--> Max Connections: %u \r\n"),thePrefs.maxconnections);
fstats.AppendFormat(_T("|--> Max Cons/5: %u \r\n"),thePrefs.MaxConperFive);
fstats.AppendFormat(_T("|--> Max Half Connection: %u \r\n"),thePrefs.GetMaxHalfConnections());
fstats.AppendFormat(_T("|--> Ports: TCP: %u UDP: %u \r\n"),thePrefs.port,thePrefs.udpport);
fstats.AppendFormat(_T("\r\n"));
	//upload
fstats.AppendFormat(_T("Upload Stats:\r\n"));
 fstats.AppendFormat(_T("-------------\r\n"));
 __int64 sessionRunTime = (__int64)((GetTickCount()-theStats.starttime)/1000);
 fstats.AppendFormat(_T("|--> %s: %s (%1.1f%%)\r\n"), GetResString(IDS_STATS_UPTIME), CastSecondsToLngHM(theStats.GetUploadTime()), (double) (100 * theStats.GetUploadTime()) / sessionRunTime);
 fstats.AppendFormat(_T("|--> Upload-Speed: %.2f %s \r\n"),theStats.rateUp,GetResString(IDS_KBYTESEC));
 fstats.AppendFormat(_T("|--> Average Uploadrate: %.2f %s \r\n"),theStats.GetAvgUploadRate(AVG_SESSION),GetResString(IDS_KBYTESEC));
 fstats.AppendFormat(_T("|--> Max Upload Rate: %.2f %s \r\n"),theStats.maxUp,GetResString(IDS_KBYTESEC));
 fstats.AppendFormat(_T("|--> Max Average Upload Rate: %.2f %s \r\n"),theStats.maxUpavg,GetResString(IDS_KBYTESEC));
 fstats.AppendFormat(_T("|--> Uploaded Data: %s \r\n"),CastItoXBytes(theStats.sessionSentBytes));
 fstats.AppendFormat(_T("|--> %s %u \r\n"), GetResString(IDS_ACTIVE_UPLOADS),theApp.uploadqueue->GetActiveUploadsCount()); 
 fstats.AppendFormat(_T("|--> %s %u \r\n"), GetResString(IDS_UPLOADS_WAITING),theApp.uploadqueue->GetUploadQueueLength());
//fstats.AppendFormat(_T("|--> On Queue/Banned : %u (%u banned) \r\n"),CastItoXBytes(theStats.sessionSentBytes));
 fstats.AppendFormat(_T("\r\n"));
	//download
 fstats.AppendFormat(_T("Download Stats:\r\n"));
 fstats.AppendFormat(_T("---------------\r\n"));
 fstats.AppendFormat(_T("|--> %s: %s (%1.1f%%)\r\n"), GetResString(IDS_STATS_DOWNTIME), CastSecondsToLngHM(theStats.GetDownloadTime()), (double) (100 * theStats.GetDownloadTime()) / sessionRunTime);
 fstats.AppendFormat(_T("|--> Download-Speed: %.2f %s \r\n"),theStats.rateDown, GetResString(IDS_KBYTESEC));
 fstats.AppendFormat(_T("|--> Average Downloadrate: %.2f %s \r\n"),theStats.GetAvgDownloadRate(AVG_SESSION),GetResString(IDS_KBYTESEC));
 fstats.AppendFormat(_T("|--> Max Download Rate: %.2f %s \r\n"),theStats.maxDown,GetResString(IDS_KBYTESEC));
 fstats.AppendFormat(_T("|--> Max Average Download Rate: %.2f %s \r\n"),theStats.maxDownavg,GetResString(IDS_KBYTESEC));
 fstats.AppendFormat(_T("|--> Downloaded Data: %s \r\n"), CastItoXBytes( theStats.sessionReceivedBytes ));
 fstats.AppendFormat(_T("\r\n"));
	//mix
 fstats.AppendFormat(_T("Statistics:\r\n"));
 fstats.AppendFormat(_T("-----------\r\n"));
 if ( theStats.sessionReceivedBytes>0 && theStats.sessionSentBytes>0 ) {
		// Session
 if (theStats.sessionReceivedBytes<theStats.sessionSentBytes) {
 fstats.AppendFormat(_T("|--> %s %.2f : 1 \r\n"),GetResString(IDS_STATS_SRATIO),(float)theStats.sessionSentBytes/theStats.sessionReceivedBytes);
		} else {
 fstats.AppendFormat(_T("|--> %s 1 : %.2f \r\n"),GetResString(IDS_STATS_SRATIO),(float)theStats.sessionReceivedBytes/theStats.sessionSentBytes);
		}
	}
	else {
 fstats.AppendFormat(_T("|--> %s %s \r\n"), GetResString(IDS_STATS_SRATIO), GetResString(IDS_FSTAT_WAITING));
	}
	uint64 bytesLargestFile = 0;
	uint64 allsize=theApp.sharedfiles->GetDatasize(bytesLargestFile); // Func returns total share size and sets pointeredd uint64 to largest single filesize
  fstats.AppendFormat(_T("|--> Sharing: %s (%i Files)\r\n"),CastItoXBytes(allsize),theApp.sharedfiles->GetCount());
	int myRateStats[3];
	uint64 ui64TotFileSize=0;
	uint64 ui64TotBytesLeftToTransfer=0;
	uint64 ui64TotNeededSpace=0;
	uint64 t_FreeBytes=0;
	theApp.downloadqueue->GetDownloadStats(myRateStats,ui64TotFileSize,ui64TotBytesLeftToTransfer,ui64TotNeededSpace);
    fstats.AppendFormat(_T("|--> Number of Files to Download: %d \r\n"),myRateStats[2]);
	fstats.AppendFormat( _T("|--> %s: %u") , GetResString( IDS_STATS_COMPDL ) , thePrefs.GetDownSessionCompletedFiles() );
	fstats.AppendFormat(_T("\r\n"));
	CMap<uint32, uint32, uint32, uint32> clientVersionEDonkey;
	CMap<uint32, uint32, uint32, uint32> clientVersionEDonkeyHybrid;
	CMap<uint32, uint32, uint32, uint32> clientVersionEMule;
	CMap<uint32, uint32, uint32, uint32> clientVersionAMule;
	uint32 totalclient;
	int myStats[15];
	theApp.clientlist->GetStatistics(totalclient, myStats,
	clientVersionEDonkey,
	clientVersionEDonkeyHybrid,
	clientVersionEMule,
	clientVersionAMule);
	uint32 otherclient = totalclient-myStats[12]-myStats[13]; //this includes IS_IDNEEDED and clients without credits
    fstats.AppendFormat(_T("|--> %s: %u (%.1f%%) : %u (%.1f%%) : %u (%.1f%%)"), GetResString(IDS_STATS_SECUREIDENT), myStats[12] , (totalclient)?((double)100*myStats[12] / totalclient):0, myStats[13] , (totalclient)?((double)100*myStats[13] / totalclient ):0, otherclient, (totalclient)?((double)100*otherclient/totalclient):0); 
	fstats.AppendFormat(_T("\r\n"));
	double percentSessions = 0;
	uint32 statBadSessions = theApp.uploadqueue->GetFailedUpCount();
	uint32 statBadSessionsdown = thePrefs.GetDownS_FailedSessions();
	//fstats.AppendFormat(_T("|--> %s: %u \r\n"),(GetResString(IDS_STATS_FOUNDSRC), myStats[0]));
	//uint32 cnt;
	//double topper = 0.0;
	//topper = (double)cnt/myStats[2];
	//fstats.AppendFormat( _T("|--> %s: %u \r\n") , GetResString( IDS_ONQUEUE ) , myStats[2] );
	fstats.AppendFormat(_T("|--> Failed Up sessions: %u (%.2f%%) \r\n"),statBadSessions,percentSessions);
	fstats.AppendFormat(_T("|--> Failed Down sessions: %u (%.2f%%) \r\n"),statBadSessionsdown,percentSessions);
	fstats.AppendFormat (_T("|--> %s: %u"), GetResString(IDS_BANNED), theApp.clientlist->GetBannedCount());
    fstats.AppendFormat (_T("  |--> %s %u"), GetResString(IDS_FEED_FILTERED), theStats.filteredclients); 
	fstats.AppendFormat(_T("\r\n"));
	fstats.AppendFormat(_T("\r\n"));
	fstats.AppendFormat(_T("|--> *Modded by NoSFeLaMa* \r\n"));
	fstats.AppendFormat(_T("|--> SupportBoard: http://lamas_gruft.real-life-board.de/ \r\n"));

	theApp.CopyTextToClipboard(fstats); 
					break;
			}
	}

	return false;
}

// This function generates the HTML output for ExportHTML.  The reason this was made separate
// from GetHTML is because it uses style sheets.  This lets the user easily customize the look
// of the HTML file after it is saved, just by changing a value here and there.
// Styled ID Tags:	pghdr	= This is used for the header that gives the eMule build and date.
//					sec		= Sections, ie Transfer, Connection, Session, Cumulative
//					item	= Items, ie UL:DL Ratio, Peak Connections, Downloaded Data
//					bdy		= The BODY tag.  Used to control the background color.
CString CStatisticsTree::GetHTMLForExport(HTREEITEM theItem, int theItemLevel, bool firstItem)
{
	static int s_iHtmlId;
	if (theItem==NULL && theItemLevel==0 && firstItem)
		s_iHtmlId = 0;

	CString		strBuffer, strItem, strImage, strChild, strTab;
	int			nImage=0, nSelectedImage=0;
	HTREEITEM	hCurrent;

	CString strDivStart, strDiv, strDivA, strDivEnd, strJ, strName;

	strBuffer.Empty();

	if (firstItem) hCurrent = GetRootItem();
	else hCurrent = theItem;

	while (hCurrent != NULL)
	{
		strItem.Empty();
		if (ItemHasChildren(hCurrent))
		{
			s_iHtmlId++;
			strJ.Format(_T("%d"),s_iHtmlId);
			if (IsExpanded(hCurrent))
			{
				strChild = _T("visible");
				strDiv = _T("<div id=\"T") + strJ + _T("\" style=\"margin-left:18px\">");
			}
			else
			{
				strChild = _T("hidden");
				strDiv = _T("<div id=\"T") + strJ + _T("\" style=\"margin-left:18px; visibility:hidden; position:absolute\">");
			}
			strDivStart = _T("<a href=\"javascript:togglevisible('") + strJ + _T("')\">");
			strDivEnd = _T("</div>");
			strDivA = _T("</a>");
			strName = _T("name=\"I") + strJ + _T("\"");
		}
		else
		{
			strChild = _T("space");
			strDiv=_T("");
			strDivStart=_T("");
			strDivEnd=_T("");
			strName=_T("");
		}
		strBuffer += _T("\n");
		for (int i = 0; i < theItemLevel; i++)
			strBuffer += _T("\t");

		strItem += strDivStart;
		strItem += _T("<img ") + strName + _T("src=\"stats_") + strChild + _T(".gif\" align=\"middle\">&nbsp;");
		strItem += strDivA;

		if (GetItemImage(hCurrent, nImage, nSelectedImage))
			strImage.Format(_T("%u"),nImage);
		else
			strImage.Format(_T("%u"),0);

		strItem += _T("<img src=\"stats_") + strImage + _T(".gif\" align=\"middle\">&nbsp;");

		if (IsBold(hCurrent))
			strItem += _T("<b>") + GetItemText(hCurrent) + _T("</b>");
		else
			strItem += GetItemText(hCurrent);

		if (theItemLevel==0) strBuffer.Append(_T("\n"));
		strBuffer += strItem + _T("<br>");

		if (ItemHasChildren(hCurrent))
		{
			strTab = _T("\n");
			for (int i = 0; i < theItemLevel; i++)
				strTab += _T("\t");
			strBuffer += strTab + strDiv;
			strBuffer += strTab + _T("\t") + GetHTMLForExport(GetChildItem(hCurrent), theItemLevel+1, false);
			strBuffer += strTab + strDivEnd;
		}
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}
	return strBuffer;
}



// Get a file name from the user, obtain the generated HTML and then save it in that file.
void CStatisticsTree::ExportHTML()
{
	CFile htmlFile;

	TCHAR szDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szDir);

	CFileDialog saveAsDlg (false, _T("html"), _T("eMule Statistics.html"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER, _T("HTML Files (*.html)|*.html|All Files (*.*)|*.*||"), this, 0);
	if (saveAsDlg.DoModal() == IDOK)
	{
		CString		strHTML;

		strHTML.Format( CString("<html>\r\n<header>\r\n<title>eMule %s [%s]</title>\r\n"
			"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
			"<style type=\"text/css\">\r\n"
			"#pghdr { color: #000F80; font: bold 12pt/14pt Verdana, Courier New, Helvetica; }\r\n"
			"#pghdr2 { color: #000F80; font: bold 10pt/12pt Verdana, Courier New, Helvetica; }\r\n"
			"img { border: 0px; }\r\n"
			"a { text-decoration: none; }\r\n"
			"#sec { color: #000000; font: bold 9pt/11pt Verdana, Courier New, Helvetica; }\r\n"
			"#item { color: #000000; font: normal 8pt/10pt Verdana, Courier New, Helvetica; }\r\n"
			"#bdy { color: #000000; font: normal 8pt/10pt Verdana, Courier New, Helvetica; background-color: #FFFFFF; }\r\n</style>\r\n</header>\r\n"
			"<script language=\"JavaScript1.2\" type=\"text/javascript\">\r\n"
			"function obj(menu)\r\n"
			"{\r\n"
			"return (navigator.appName == \"Microsoft Internet Explorer\")?this[menu]:document.getElementById(menu);\r\n"
			"}\r\n"
			"function togglevisible(treepart)\r\n"
			"{\r\n"
			"if (this.obj(\"T\"+treepart).style.visibility == \"hidden\")\r\n"
			"{\r\n"
			"this.obj(\"T\"+treepart).style.position=\"\";\r\n"
			"this.obj(\"T\"+treepart).style.visibility=\"\";\r\n"
			"document[\"I\"+treepart].src=\"stats_visible.gif\";\r\n"
			"}\r\n"
			"else\r\n"
			"{\r\n"
			"this.obj(\"T\"+treepart).style.position=\"absolute\";\r\n"
			"this.obj(\"T\"+treepart).style.visibility=\"hidden\";\r\n"
			"document[\"I\"+treepart].src=\"stats_hidden.gif\";\r\n"
			"}\r\n"
			"}\r\n"
			"</script>\r\n"
			"<body id=\"bdy\">\r\n"

			"<span id=\"pghdr\"><b>eMule %s</b></span><br><span id=\"pghdr2\">%s %s</span>\r\n<br><br>\r\n"
			"%s</body></html>") ,
			GetResString(IDS_SF_STATISTICS), thePrefs.GetUserNick(),
			GetResString(IDS_SF_STATISTICS), GetResString(IDS_CD_UNAME), thePrefs.GetUserNick(),
			GetHTMLForExport() );

		htmlFile.Open(saveAsDlg.GetPathName(), CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);
		
		CStringA strHtmlA(wc2utf8(strHTML));
		htmlFile.Write(strHtmlA, strHtmlA.GetLength());		
		
		htmlFile.Close();

		static const TCHAR *const s_apcFileNames[] = {
			_T("stats_0.gif"), _T("stats_1.gif"), _T("stats_2.gif"), _T("stats_3.gif"), _T("stats_4.gif"),
			_T("stats_5.gif"), _T("stats_6.gif"), _T("stats_7.gif"), _T("stats_8.gif"), _T("stats_9.gif"),
			_T("stats_10.gif"), _T("stats_11.gif"), _T("stats_12.gif"), _T("stats_13.gif"),
			_T("stats_14.gif"), _T("stats_15.gif"), _T("stats_16.gif"), _T("stats_17.gif"),
			_T("stats_hidden.gif"), _T("stats_space.gif"), _T("stats_visible.gif")
		};
		CString		strDst = saveAsDlg.GetPathName().Left(saveAsDlg.GetPathName().GetLength() - saveAsDlg.GetFileName().GetLength());// EC - what if directory name == filename? this should fix this
		CString		strSrc = thePrefs.GetAppDir();

		strSrc += _T("\\WebServer\\");
		for (unsigned ui = 0; ui < ARRSIZE(s_apcFileNames); ui++)
			::CopyFile(strSrc + s_apcFileNames[ui], strDst + s_apcFileNames[ui], false);
	}

	SetCurrentDirectory(szDir);
}

// Expand all the tree sections.  Recursive.
// Can also expand only bold items (Main Sections)
void CStatisticsTree::ExpandAll(bool onlyBold, HTREEITEM theItem)
{
	HTREEITEM hCurrent;

	if (theItem == NULL) {
		if (onlyBold) CollapseAll();
		hCurrent = GetRootItem();
		m_bExpandingAll = true;
	}
	else
		hCurrent = theItem;

	while (hCurrent != NULL)
	{
		if (ItemHasChildren(hCurrent) && (!onlyBold || IsBold(hCurrent))) {
			Expand(hCurrent, TVE_EXPAND);
			ExpandAll(onlyBold, GetChildItem(hCurrent));
		}
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}

	if (theItem == NULL) m_bExpandingAll = false;
}

// Collapse all the tree sections.  This is recursive
// so that we can collapse submenus.  SetRedraw should
// be FALSE while this is executing.
void CStatisticsTree::CollapseAll(HTREEITEM theItem)
{
	HTREEITEM hCurrent;

	if (theItem == NULL) {
		hCurrent = GetRootItem();
		m_bExpandingAll = true;
	}
	else
		hCurrent = theItem;

	while (hCurrent != NULL)
	{
		if (ItemHasChildren(hCurrent))
			CollapseAll(GetChildItem(hCurrent));
		Expand(hCurrent, TVE_COLLAPSE);
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}

	if (theItem == NULL) m_bExpandingAll = false;
}

// This returns a string of 1's and 0's indicating
// which parent items are expanded.  Only saves the
// bold items.
CString CStatisticsTree::GetExpandedMask(HTREEITEM theItem)
{
	HTREEITEM	hCurrent;
	CString		tempMask;

	tempMask.Empty();

	if (theItem == NULL)
		hCurrent = GetRootItem();
	else
		hCurrent = theItem;

	while (hCurrent != NULL)
	{
		if (ItemHasChildren(hCurrent) && IsBold(hCurrent)) {
			if (IsExpanded(hCurrent))
				tempMask += "1";
			if (!IsExpanded(hCurrent))
				tempMask += "0";
			tempMask += GetExpandedMask(GetChildItem(hCurrent));
		}
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}
	return tempMask;
}

// This takes a string and uses it to set the expanded or
// collapsed state of the tree items.
int CStatisticsTree::ApplyExpandedMask(CString theMask, HTREEITEM theItem, int theStringIndex)
{
	HTREEITEM	hCurrent;

	if (theItem == NULL) {
		hCurrent = GetRootItem();
		SetRedraw(false);
		ExpandAll(true);
		m_bExpandingAll = true;
	}
	else
		hCurrent = theItem;

	while (hCurrent != NULL && theStringIndex < theMask.GetLength())
	{
		if (ItemHasChildren(hCurrent) && IsBold(hCurrent)) {
			if (theMask.GetAt(theStringIndex) == '0') Expand(hCurrent, TVE_COLLAPSE);
			theStringIndex++;
			theStringIndex = ApplyExpandedMask(theMask, GetChildItem(hCurrent), theStringIndex);			
		}
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}
	if (theItem == NULL) {
		SetRedraw(true);
		m_bExpandingAll = true;
	}
	return theStringIndex;
}
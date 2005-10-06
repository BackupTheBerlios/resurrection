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
#include "PreferencesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPreferencesDlg, CTreePropSheet)

BEGIN_MESSAGE_MAP(CPreferencesDlg, CTreePropSheet)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CPreferencesDlg::CPreferencesDlg()
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_wndGeneral.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDisplay.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndConnection.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDirectories.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndFiles.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndStats.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndWebServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndTweaks.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndSecurity.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndProxy.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndAckronicII.m_psp.dwFlags &= ~PSH_HASHELP;//Ackronic - Aggiunto da Aenarion[ITA] - 2° finestra Ack
 m_wndVipeR.m_psp.dwFlags &= ~PSH_HASHELP;  //Added by Spe64  Pref1
    m_wndNe.m_psp.dwFlags &= ~PSH_HASHELP;  //Added by Spe64  Pref2
m_wndSpe3.m_psp.dwFlags &= ~PSH_HASHELP;  //Spe64
  //KTS+ webcache
    m_wndWebcachesettings.m_psp.dwFlags &= ~PSH_HASHELP; 
    //KTS- webcache
	//m_wndAntiLeech.m_psp.dwFlags &= ~PSH_HASHELP; //>>> AntiLeech Class
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	m_wndDebug.m_psp.dwFlags &= ~PSH_HASHELP;
#endif

	CTreePropSheet::SetPageIcon(&m_wndGeneral, _T("Preferences"));
	CTreePropSheet::SetPageIcon(&m_wndDisplay, _T("DISPLAY"));
	CTreePropSheet::SetPageIcon(&m_wndConnection, _T("CONNECTION"));
	CTreePropSheet::SetPageIcon(&m_wndProxy, _T("PROXY"));
	CTreePropSheet::SetPageIcon(&m_wndServer, _T("SERVER"));
	CTreePropSheet::SetPageIcon(&m_wndDirectories, _T("FOLDERS"));
	CTreePropSheet::SetPageIcon(&m_wndFiles, _T("Transfer"));
	CTreePropSheet::SetPageIcon(&m_wndNotify, _T("NOTIFICATIONS"));
	CTreePropSheet::SetPageIcon(&m_wndStats, _T("STATISTICS"));
	CTreePropSheet::SetPageIcon(&m_wndSecurity, _T("SECURITY"));
	CTreePropSheet::SetPageIcon(&m_wndWebServer, _T("WEB"));
	CTreePropSheet::SetPageIcon(&m_wndTweaks, _T("TWEAK"));
	CTreePropSheet::SetPageIcon(&m_wndAckronicII, _T("ACKRONIC"));//Ackronic - Aggiunto da Aenarion[ITA] - 2° finestra Ack
 CTreePropSheet::SetPageIcon(&m_wndVipeR, _T("CONTACT4")); //Added by Spe64  Pref1
    CTreePropSheet::SetPageIcon(&m_wndNe, _T("CONTACT3")); //Added by Spe64  Pref2
CTreePropSheet::SetPageIcon(&m_wndSpe3, _T("DOWNLOADFILES"));  //Spe64
	CTreePropSheet::SetPageIcon(&m_wndWebcachesettings, _T("PREF_WEBCACHE"));
	//CTreePropSheet::SetPageIcon(&m_wndAntiLeech, _T("DELETE")); //>>> AntiLeech Class

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CTreePropSheet::SetPageIcon(&m_wndDebug, _T("Preferences"));
#endif

	AddPage(&m_wndGeneral);
	AddPage(&m_wndDisplay);
	AddPage(&m_wndConnection);
	AddPage(&m_wndProxy);
	AddPage(&m_wndServer);
	AddPage(&m_wndDirectories);
	AddPage(&m_wndFiles);
	AddPage(&m_wndNotify);
	AddPage(&m_wndStats);
	AddPage(&m_wndSecurity);
	AddPage(&m_wndWebServer);
	AddPage(&m_wndTweaks);
	AddPage(&m_wndAckronicII);//Ackronic - Aggiunto da Aenarion[ITA] - 2° finestra Ack
AddPage(&m_wndVipeR);  //Added by Spe64  Pref1
    AddPage(&m_wndNe);//Added by Spe64  Pref2
    AddPage(&m_wndSpe3); //Added by Spe34 Pref3
    AddPage(&m_wndWebcachesettings); //MORPH - Added by SiRoB, WebCache 1.2f
	//AddPage(&m_wndAntiLeech); //>>> AntiLeech Class

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	AddPage(&m_wndDebug);
#endif

	SetTreeViewMode(TRUE, TRUE, TRUE);
	SetTreeWidth(170);

	m_pPshStartPage = NULL;
	m_bSaveIniFile = false;
}

CPreferencesDlg::~CPreferencesDlg()
{
}

void CPreferencesDlg::OnDestroy()
{
	CTreePropSheet::OnDestroy();
	if (m_bSaveIniFile)
	{
		thePrefs.Save();
		m_bSaveIniFile = false;
	}
	m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
}

BOOL CPreferencesDlg::OnInitDialog()
{
	ASSERT( !m_bSaveIniFile );
	BOOL bResult = CTreePropSheet::OnInitDialog();
	InitWindowStyles(this);

	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		if (GetPage(i)->m_psp.pszTemplate == m_pPshStartPage)
		{
			SetActivePage(i);
			break;
		}
	}
	// [TPT] - New Preferences Banner		
	CBitmap bmp;
	bmp.LoadBitmap(IDB_IONIX);
	m_banner.SetTexture((HBITMAP)bmp.Detach());	
	m_banner.SetFillFlag(KCSB_FILL_TEXTURE);
	m_banner.SetSize(75);
	m_banner.SetTitle(_T(""));
	m_banner.SetCaption(_T(""));
	m_banner.Attach(this, KCSB_ATTACH_RIGHT);
	// [TPT] - New Preferences Banner end

	Localize();	
	return bResult;
}

void CPreferencesDlg::Localize()
{
	SetTitle(RemoveAmbersand(GetResString(IDS_EM_PREFS))); 

	m_wndGeneral.Localize();
	m_wndDisplay.Localize();
	m_wndConnection.Localize();
	m_wndServer.Localize();
	m_wndDirectories.Localize();
	m_wndFiles.Localize();
	m_wndStats.Localize();
	m_wndNotify.Localize();
	m_wndSecurity.Localize();
	m_wndTweaks.Localize();
	m_wndWebServer.Localize();
	m_wndProxy.Localize();
	m_wndAckronicII.Localize();//Ackronic - Aggiunto da Aenarion[ITA] - 2° finestra Ack
 m_wndVipeR.Localize(); //Added by Spe64  Pref1
    m_wndNe.Localize(); //Added by Spe64  Pref2
     m_wndSpe3.Localize(); //Added by Spe64 Pr
	//m_wndAntiLeech.Localize(); //>>> AntiLeech Class
	m_wndWebcachesettings.Localize();
	CTreeCtrl* pTree = GetPageTreeControl();
	if (pTree)
	{
		pTree->SetItemText(GetPageTreeItem(0), RemoveAmbersand(GetResString(IDS_PW_GENERAL)));
		pTree->SetItemText(GetPageTreeItem(1), RemoveAmbersand(GetResString(IDS_PW_DISPLAY))); 
		pTree->SetItemText(GetPageTreeItem(2), RemoveAmbersand(GetResString(IDS_PW_CONNECTION))); 
		pTree->SetItemText(GetPageTreeItem(3), RemoveAmbersand(GetResString(IDS_PW_PROXY))); 
		pTree->SetItemText(GetPageTreeItem(4), RemoveAmbersand(GetResString(IDS_PW_SERVER))); 
		pTree->SetItemText(GetPageTreeItem(5), RemoveAmbersand(GetResString(IDS_PW_DIR))); 
		pTree->SetItemText(GetPageTreeItem(6), RemoveAmbersand(GetResString(IDS_PW_FILES))); 
		pTree->SetItemText(GetPageTreeItem(7), RemoveAmbersand(GetResString(IDS_PW_EKDEV_OPTIONS))); 
		pTree->SetItemText(GetPageTreeItem(8), RemoveAmbersand(GetResString(IDS_STATSSETUPINFO))); 
		pTree->SetItemText(GetPageTreeItem(9), RemoveAmbersand(GetResString(IDS_SECURITY))); 
		pTree->SetItemText(GetPageTreeItem(10), RemoveAmbersand(GetResString(IDS_PW_WS)));
		pTree->SetItemText(GetPageTreeItem(11), RemoveAmbersand(GetResString(IDS_PW_TWEAK)));
		pTree->SetItemText(GetPageTreeItem(12), RemoveAmbersand(GetResString(IDS_ACKRONICII)));//Ackronic - Aggiunto da Aenarion[ITA] - 2° finestra Ack
        pTree->SetItemText(GetPageTreeItem(13), RemoveAmbersand(GetResString(IDS_PW_VipeR))); //Spe64 Pref1
		pTree->SetItemText(GetPageTreeItem(14), RemoveAmbersand(GetResString(IDS_PW_NE))); //Spe64 Pref2
	    pTree->SetItemText(GetPageTreeItem(15), RemoveAmbersand(GetResString(IDS_PW_Spe3))); //Spe64 Pref3
				//pTree->SetItemText(GetPageTreeItem(i++), RemoveAmbersand(_T("AntiLeech"))); //>>> AntiLeech Class
pTree->SetItemText(GetPageTreeItem(16), RemoveAmbersand(GetResString(IDS_PW_WEBCACHE)));//KTS

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
		pTree->SetItemText(GetPageTreeItem(20), _T("Debug"));
	#endif
	}
	m_banner.UpdateSize(); //[ionix] - Added: Preferences Banner [TPT]	
	UpdateCaption();
}

void CPreferencesDlg::SetStartPage(UINT uStartPageID)
{
	m_pPshStartPage = MAKEINTRESOURCE(uStartPageID);
}

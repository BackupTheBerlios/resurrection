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
#include "PPgSecurity.h"
#include "OtherFunctions.h"
#include "IPFilter.h"
#include "Preferences.h"
#include "CustomAutoComplete.h"
#include "HttpDownloadDlg.h"
#include "emuledlg.h"
#include "ZipFile.h"
#include "GZipFile.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	IPFILTERUPDATEURL_STRINGS_PROFILE	_T("AC_IPFilterUpdateURLs.dat")

IMPLEMENT_DYNAMIC(CPPgSecurity, CPropertyPage)
CPPgSecurity::CPPgSecurity()
	: CPropertyPage(CPPgSecurity::IDD)
{
	m_pacIPFilterURL = NULL;
}

CPPgSecurity::~CPPgSecurity()
{

}

void CPPgSecurity::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPPgSecurity, CPropertyPage)
	ON_BN_CLICKED(IDC_FILTERSERVERBYIPFILTER , OnSettingsChange)
	ON_BN_CLICKED(IDC_RELOADFILTER, OnReloadIPFilter)
	ON_BN_CLICKED(IDC_EDITFILTER, OnEditIPFilter)
	ON_EN_CHANGE(IDC_FILTERLEVEL, OnSettingsChange)
	ON_EN_CHANGE(IDC_FILTER, OnSettingsChange)
	ON_EN_CHANGE(IDC_COMMENTFILTER, OnSettingsChange)
	ON_BN_CLICKED(IDC_MSGONLYFRIENDS , OnSettingsChange) 
	ON_BN_CLICKED(IDC_MSGONLYSEC, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVSPAMFILTER , OnSettingsChange)
	ON_BN_CLICKED(IDC_USESECIDENT, OnSettingsChange)
	ON_BN_CLICKED(IDC_LOADURL, OnLoadIPFFromURL)
	ON_EN_CHANGE(IDC_UPDATEURL, OnEnChangeUpdateUrl)
	ON_BN_CLICKED(IDC_DD,OnDDClicked)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_RUNASUSER, OnBnClickedRunasuser)
END_MESSAGE_MAP()

void CPPgSecurity::LoadSettings(void)
{
	CString strBuffer;
	
	strBuffer.Format(_T("%i"),thePrefs.filterlevel);
	GetDlgItem(IDC_FILTERLEVEL)->SetWindowText(strBuffer);

	if(thePrefs.filterserverbyip)
		CheckDlgButton(IDC_FILTERSERVERBYIPFILTER,1);
	else
		CheckDlgButton(IDC_FILTERSERVERBYIPFILTER,0);

	if(thePrefs.msgonlyfriends)
		CheckDlgButton(IDC_MSGONLYFRIENDS,1);
	else
		CheckDlgButton(IDC_MSGONLYFRIENDS,0);

	if(thePrefs.msgsecure)
		CheckDlgButton(IDC_MSGONLYSEC,1);
	else
		CheckDlgButton(IDC_MSGONLYSEC,0);

	if(thePrefs.m_bAdvancedSpamfilter)
		CheckDlgButton(IDC_ADVSPAMFILTER,1);
	else
		CheckDlgButton(IDC_ADVSPAMFILTER,0);

	if(thePrefs.m_bUseSecureIdent)
		CheckDlgButton(IDC_USESECIDENT,1);
	else
		CheckDlgButton(IDC_USESECIDENT,0);

	if (thePrefs.GetWindowsVersion() == _WINVER_XP_ || thePrefs.GetWindowsVersion() == _WINVER_2K_)
		GetDlgItem(IDC_RUNASUSER)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_RUNASUSER)->EnableWindow(FALSE);

	if(thePrefs.IsRunAsUserEnabled())
		CheckDlgButton(IDC_RUNASUSER,1);
	else
		CheckDlgButton(IDC_RUNASUSER,0);

	GetDlgItem(IDC_FILTER)->SetWindowText(thePrefs.messageFilter);
	GetDlgItem(IDC_COMMENTFILTER)->SetWindowText(thePrefs.commentFilter);
}

BOOL CPPgSecurity::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	LoadSettings();
	Localize();

	if (thePrefs.GetUseAutocompletion()){
		if (!m_pacIPFilterURL) {
			m_pacIPFilterURL = new CCustomAutoComplete();
			m_pacIPFilterURL->AddRef();
			if (m_pacIPFilterURL->Bind(::GetDlgItem(m_hWnd, IDC_UPDATEURL), ACO_UPDOWNKEYDROPSLIST | ACO_AUTOSUGGEST | ACO_FILTERPREFIXES ))
				m_pacIPFilterURL->LoadList(thePrefs.GetConfigDir() + IPFILTERUPDATEURL_STRINGS_PROFILE);
		}
		SetDlgItemText(IDC_UPDATEURL,m_pacIPFilterURL->GetItem(0));
		if (theApp.m_fontSymbol.m_hObject){
			GetDlgItem(IDC_DD)->SetFont(&theApp.m_fontSymbol);
			GetDlgItem(IDC_DD)->SetWindowText(_T("6")); // show a down-arrow
		}
	}
	else
		GetDlgItem(IDC_DD)->ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgSecurity::OnApply()
{
	TCHAR buffer[510];
	if(GetDlgItem(IDC_FILTERLEVEL)->GetWindowTextLength())
	{
		GetDlgItem(IDC_FILTERLEVEL)->GetWindowText(buffer,4);
		thePrefs.filterlevel=_tstoi(buffer);
	}

	thePrefs.filterserverbyip = IsDlgButtonChecked(IDC_FILTERSERVERBYIPFILTER)!=0;
	thePrefs.msgonlyfriends = IsDlgButtonChecked(IDC_MSGONLYFRIENDS)!=0;
	thePrefs.msgsecure = IsDlgButtonChecked(IDC_MSGONLYSEC)!=0;
	thePrefs.m_bAdvancedSpamfilter = IsDlgButtonChecked(IDC_ADVSPAMFILTER)!=0;
	thePrefs.m_bUseSecureIdent = IsDlgButtonChecked(IDC_USESECIDENT)!=0;
	thePrefs.m_bRunAsUser = IsDlgButtonChecked(IDC_RUNASUSER)!=0;

	GetDlgItem(IDC_FILTER)->GetWindowText(thePrefs.messageFilter,ARRSIZE(thePrefs.messageFilter));

	CString strCommentFilters;
	GetDlgItem(IDC_COMMENTFILTER)->GetWindowText(strCommentFilters);
	strCommentFilters.MakeLower();
	CString strNewCommentFilters;
	int curPos = 0;
	CString strFilter(strCommentFilters.Tokenize(_T("|"), curPos));
	while (!strFilter.IsEmpty())
	{
		strFilter.Trim();
		if (!strNewCommentFilters.IsEmpty())
			strNewCommentFilters += _T('|');
		strNewCommentFilters += strFilter;
		strFilter = strCommentFilters.Tokenize(_T("|"), curPos);
	}
	thePrefs.commentFilter = strNewCommentFilters;
	if (thePrefs.commentFilter != strCommentFilters)
		SetDlgItemText(IDC_COMMENTFILTER, thePrefs.commentFilter);

	LoadSettings();
	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgSecurity::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_SECURITY));
		GetDlgItem(IDC_STATIC_IPFILTER)->SetWindowText(GetResString(IDS_IPFILTER));
		GetDlgItem(IDC_RELOADFILTER)->SetWindowText(GetResString(IDS_SF_RELOAD));
		GetDlgItem(IDC_EDITFILTER)->SetWindowText(GetResString(IDS_EDIT));
		GetDlgItem(IDC_STATIC_FILTERLEVEL)->SetWindowText(GetResString(IDS_FILTERLEVEL)+_T(":"));
		GetDlgItem(IDC_FILTERSERVERBYIPFILTER)->SetWindowText(GetResString(IDS_FILTERSERVERBYIPFILTER));

		GetDlgItem(IDC_FILTERCOMMENTSLABEL)->SetWindowText(GetResString(IDS_FILTERCOMMENTSLABEL));
		GetDlgItem(IDC_STATIC_COMMENTS)->SetWindowText(GetResString(IDS_COMMENT));

		GetDlgItem(IDC_FILTERLABEL)->SetWindowText(GetResString(IDS_FILTERLABEL));
		GetDlgItem(IDC_MSG)->SetWindowText(GetResString(IDS_CW_MESSAGES));

		GetDlgItem(IDC_MSGONLYFRIENDS)->SetWindowText(GetResString(IDS_MSGONLYFRIENDS));
		GetDlgItem(IDC_MSGONLYSEC)->SetWindowText(GetResString(IDS_MSGONLYSEC));

		GetDlgItem(IDC_ADVSPAMFILTER)->SetWindowText(GetResString(IDS_ADVSPAMFILTER));
		GetDlgItem(IDC_SEC_MISC)->SetWindowText(GetResString(IDS_PW_MISC));
		GetDlgItem(IDC_USESECIDENT)->SetWindowText(GetResString(IDS_USESECIDENT));

		SetDlgItemText(IDC_STATIC_UPDATEFROM,GetResString(IDS_UPDATEFROM));
		SetDlgItemText(IDC_LOADURL,GetResString(IDS_LOADURL));

		SetDlgItemText(IDC_RUNASUSER,GetResString(IDS_RUNASUSER));
	}
}

void CPPgSecurity::OnReloadIPFilter()
{
	CWaitCursor curHourglass;
	theApp.ipfilter->LoadFromDefaultFile();
}

void CPPgSecurity::OnEditIPFilter()
{
	ShellExecute(NULL, _T("open"), thePrefs.GetTxtEditor(),
		_T("\"") + thePrefs.GetConfigDir() + DFLT_IPFILTER_FILENAME _T("\""), NULL, SW_SHOW);
}

void CPPgSecurity::OnLoadIPFFromURL()
{
	CString url;
	GetDlgItemText(IDC_UPDATEURL,url);
	if (!url.IsEmpty())
	{
		// add entered URL to LRU list even if it's not yet known whether we can download from this URL (it's just more convenient this way)
		if (m_pacIPFilterURL && m_pacIPFilterURL->IsBound())
			m_pacIPFilterURL->AddItem(url, 0);

		CString strTempFilePath;
		_tmakepath(strTempFilePath.GetBuffer(MAX_PATH), NULL, thePrefs.GetConfigDir(), DFLT_IPFILTER_FILENAME, _T("tmp"));
		strTempFilePath.ReleaseBuffer();

		CHttpDownloadDlg dlgDownload;
		dlgDownload.m_strTitle = GetResString(IDS_DWL_IPFILTERFILE);
		dlgDownload.m_sURLToDownload = url;
		dlgDownload.m_sFileToDownloadInto = strTempFilePath;
		if (dlgDownload.DoModal() != IDOK)
		{
			_tremove(strTempFilePath);
			CString strError = GetResString(IDS_DWLIPFILTERFAILED);
			if (!dlgDownload.GetError().IsEmpty())
				strError += _T("\r\n\r\n") + dlgDownload.GetError();
			AfxMessageBox(strError);
			return;
		}

		bool bIsZipFile = false;
		bool bUnzipped = false;
		CZIPFile zip;
		if (zip.Open(strTempFilePath))
		{
			bIsZipFile = true;

			CZIPFile::File* zfile = zip.GetFile(_T("guarding.p2p"));
			if (zfile)
			{
				CString strTempUnzipFilePath;
				_tmakepath(strTempUnzipFilePath.GetBuffer(_MAX_PATH), NULL, thePrefs.GetConfigDir(), DFLT_IPFILTER_FILENAME, _T(".unzip.tmp"));
				strTempUnzipFilePath.ReleaseBuffer();

				if (zfile->Extract(strTempUnzipFilePath))
				{
					zip.Close();
					zfile = NULL;

					if (_tremove(theApp.ipfilter->GetDefaultFilePath()) != 0)
						TRACE(_T("*** Error: Failed to remove default IP filter file \"%s\" - %hs\n"), theApp.ipfilter->GetDefaultFilePath(), strerror(errno));
					if (_trename(strTempUnzipFilePath, theApp.ipfilter->GetDefaultFilePath()) != 0)
						TRACE(_T("*** Error: Failed to rename uncompressed IP filter file \"%s\" to default IP filter file \"%s\" - %hs\n"), strTempUnzipFilePath, theApp.ipfilter->GetDefaultFilePath(), strerror(errno));
					if (_tremove(strTempFilePath) != 0)
						TRACE(_T("*** Error: Failed to remove temporary IP filter file \"%s\" - %hs\n"), strTempFilePath, strerror(errno));
					bUnzipped = true;
				}
				else {
					CString strError;
					strError.Format(GetResString(IDS_ERR_IPFILTERZIPEXTR), strTempFilePath);
					AfxMessageBox(strError);
				}
			}
			else {
				CString strError;
				strError.Format(GetResString(IDS_ERR_IPFILTERCONTENTERR), strTempFilePath);
				AfxMessageBox(strError);
			}

			zip.Close();
		}
		else
		{
			CGZIPFile gz;
			if (gz.Open(strTempFilePath))
			{
				bIsZipFile = true;

				CString strTempUnzipFilePath;
				_tmakepath(strTempUnzipFilePath.GetBuffer(_MAX_PATH), NULL, thePrefs.GetConfigDir(), DFLT_IPFILTER_FILENAME, _T(".unzip.tmp"));
				strTempUnzipFilePath.ReleaseBuffer();

				// add filename and extension of uncompressed file to temporary file
				CString strUncompressedFileName = gz.GetUncompressedFileName();
				if (!strUncompressedFileName.IsEmpty())
				{
					strTempUnzipFilePath += _T('.');
					strTempUnzipFilePath += strUncompressedFileName;
				}

				if (gz.Extract(strTempUnzipFilePath))
				{
					gz.Close();

					if (_tremove(theApp.ipfilter->GetDefaultFilePath()) != 0)
						TRACE(_T("*** Error: Failed to remove default IP filter file \"%s\" - %hs\n"), theApp.ipfilter->GetDefaultFilePath(), strerror(errno));
					if (_trename(strTempUnzipFilePath, theApp.ipfilter->GetDefaultFilePath()) != 0)
						TRACE(_T("*** Error: Failed to rename uncompressed IP filter file \"%s\" to default IP filter file \"%s\" - %hs\n"), strTempUnzipFilePath, theApp.ipfilter->GetDefaultFilePath(), strerror(errno));
					if (_tremove(strTempFilePath) != 0)
						TRACE(_T("*** Error: Failed to remove temporary IP filter file \"%s\" - %hs\n"), strTempFilePath, strerror(errno));
					bUnzipped = true;
				}
				else {
					CString strError;
					strError.Format(GetResString(IDS_ERR_IPFILTERZIPEXTR), strTempFilePath);
					AfxMessageBox(strError);
				}
			}
			gz.Close();
		}

		if (!bIsZipFile && !bUnzipped)
		{
			_tremove(theApp.ipfilter->GetDefaultFilePath());
			_trename(strTempFilePath, theApp.ipfilter->GetDefaultFilePath());
		}
	}
	OnReloadIPFilter();
}

void CPPgSecurity::DeleteDDB()
{
	if (m_pacIPFilterURL)
	{
		m_pacIPFilterURL->SaveList(thePrefs.GetConfigDir() + IPFILTERUPDATEURL_STRINGS_PROFILE);
		m_pacIPFilterURL->Unbind();
		m_pacIPFilterURL->Release();
	}
}

BOOL CPPgSecurity::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN){

		if (pMsg->wParam == VK_ESCAPE)
			return FALSE;

		if( m_pacIPFilterURL && m_pacIPFilterURL->IsBound() && ((pMsg->wParam == VK_DELETE) && (pMsg->hwnd == GetDlgItem(IDC_UPDATEURL)->m_hWnd) && (GetAsyncKeyState(VK_MENU)<0 || GetAsyncKeyState(VK_CONTROL)<0)) )
			m_pacIPFilterURL->Clear();

		if (pMsg->wParam == VK_RETURN){
			if (pMsg->hwnd == GetDlgItem(IDC_UPDATEURL)->m_hWnd){
				if (m_pacIPFilterURL && m_pacIPFilterURL->IsBound() ){
					CString strText;
					GetDlgItem(IDC_UPDATEURL)->GetWindowText(strText);
					if (!strText.IsEmpty()){
						GetDlgItem(IDC_UPDATEURL)->SetWindowText(_T("")); // this seems to be the only chance to let the dropdown list to disapear
						GetDlgItem(IDC_UPDATEURL)->SetWindowText(strText);
						((CEdit*)GetDlgItem(IDC_UPDATEURL))->SetSel(strText.GetLength(), strText.GetLength());
					}
				}
				return TRUE;
			}
		}
	}
   
	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CPPgSecurity::OnEnChangeUpdateUrl()
{
	CString strUrl;
	GetDlgItemText(IDC_UPDATEURL, strUrl);
	GetDlgItem(IDC_LOADURL)->EnableWindow(!strUrl.IsEmpty());
}

void CPPgSecurity::OnDDClicked() {
	
	CWnd* box=GetDlgItem(IDC_UPDATEURL);
	box->SetFocus();
	box->SetWindowText(_T(""));
	box->SendMessage(WM_KEYDOWN,VK_DOWN,0x00510001);
}


void CPPgSecurity::OnBnClickedRunasuser()
{
	if ( ((CButton*)GetDlgItem(IDC_RUNASUSER))->GetCheck() == BST_CHECKED){
		if (AfxMessageBox(GetResString(IDS_RAU_WARNING),MB_OKCANCEL | MB_ICONINFORMATION,0) == IDCANCEL)
			((CButton*)GetDlgItem(IDC_RUNASUSER))->SetCheck(BST_UNCHECKED);
	}
	OnSettingsChange();
}

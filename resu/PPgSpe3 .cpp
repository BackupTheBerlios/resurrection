// PpgMorph2.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PPgSpe3.h"
#include "emuleDlg.h"
#include "OtherFunctions.h"
#include "serverWnd.h"
#include "Fakecheck.h"
#include "IPFilter.h"
#include ".\ppgspe3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#pragma warning(disable:4800)
// CPPgMorph dialog

IMPLEMENT_DYNAMIC(CPPgSpe3, CPropertyPage)
CPPgSpe3::CPPgSpe3()
: CPropertyPage(CPPgSpe3::IDD)
{
}

CPPgSpe3::~CPPgSpe3()
{
}

void CPPgSpe3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPPgSpe3, CPropertyPage)
	//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
	ON_BN_CLICKED(IDC_UPDATEFAKELISTSTART, OnSettingsChange)
	ON_BN_CLICKED(IDC_RESETFAKESURL, OnBnClickedResetfakes)
	ON_BN_CLICKED(IDC_UPDATEFAKES, OnBnClickedUpdatefakes)
	ON_EN_CHANGE(IDC_UPDATE_URL_FAKELIST, OnSettingsChange)
	//MORPH END - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
//MORPH START added by Yun.SF3: Ipfilter.dat update
	ON_EN_CHANGE(IDC_UPDATE_URL_IPFILTER, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPDATEIPFURL, OnBnClickedUpdateipfurl)
	ON_BN_CLICKED(IDC_RESETIPFURL, OnBnClickedResetipfurl)
	ON_BN_CLICKED(IDC_AUTOUPIPFILTER , OnSettingsChange)
	//MORPH END added by Yun.SF3: Ipfilter.dat update
END_MESSAGE_MAP()


// CPPgMorph message handlers

BOOL CPPgSpe3::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CPPgSpe3::LoadSettings(void)
{
CString strBuffer;	
//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
	GetDlgItem(IDC_UPDATE_URL_FAKELIST)->SetWindowText(thePrefs.UpdateURLFakeList);
	if(thePrefs.UpdateFakeStartup)
		CheckDlgButton(IDC_UPDATEFAKELISTSTART,1);
	else
		CheckDlgButton(IDC_UPDATEFAKELISTSTART,0);
	//MORPH END - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating

//MORPH START added by Yun.SF3: Ipfilter.dat update
	GetDlgItem(IDC_UPDATE_URL_IPFILTER)->SetWindowText(thePrefs.UpdateURLIPFilter);
	if(thePrefs.AutoUpdateIPFilter)
		CheckDlgButton(IDC_AUTOUPIPFILTER,1);
	else
		CheckDlgButton(IDC_AUTOUPIPFILTER,0);
	//MORPH END added by Yun.SF3: Ipfilter.dat update

}

BOOL CPPgSpe3::OnApply()
{

	CString buffer;
	
	//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
	GetDlgItem(IDC_UPDATE_URL_FAKELIST)->GetWindowText(buffer);
	_tcscpy(thePrefs.UpdateURLFakeList, buffer);
	thePrefs.UpdateFakeStartup = IsDlgButtonChecked(IDC_UPDATEFAKELISTSTART);
	//MORPH END   - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating


    //MORPH START - Added by Yun.SF3: Ipfilter.dat update
	GetDlgItem(IDC_UPDATE_URL_IPFILTER)->GetWindowText(buffer);
	_tcscpy(thePrefs.UpdateURLIPFilter, buffer);
	thePrefs.AutoUpdateIPFilter = IsDlgButtonChecked(IDC_AUTOUPIPFILTER);
	//MORPH END   - Added by Yun.SF3: Ipfilter.dat update

	LoadSettings();
	SetModified(FALSE);
	
	return CPropertyPage::OnApply();
}
void CPPgSpe3::Localize(void)
{
	if(m_hWnd)
	{
		//SetWindowText(GetResString(IDS_PW_NE2));
		
		//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
		GetDlgItem(IDC_MORPH2_FILE)->SetWindowText(GetResString(IDS_FILES));
		GetDlgItem(IDC_UPDATEFAKELISTSTART)->SetWindowText(GetResString(IDS_UPDATEFAKECHECKONSTART));
		GetDlgItem(IDC_UPDATEFAKES)->SetWindowText(GetResString(IDS_UPDATEFAKES));
		GetDlgItem(IDC_URL_FOR_UPDATING)->SetWindowText(GetResString(IDS_URL_FOR_UPDATING));
		GetDlgItem(IDC_RESETFAKESURL)->SetWindowText(GetResString(IDS_RESET));
		//MORPH END   - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
      
		//MORPH START - Added by Yun.SF3: Ipfilter.dat update
		GetDlgItem(IDC_MORPH2_SECURITY)->SetWindowText(GetResString(IDS_SECURITY));
		GetDlgItem(IDC_AUTOUPIPFILTER)->SetWindowText(GetResString(IDS_UPDATEIPFILTERONSTART));
		GetDlgItem(IDC_UPDATEIPFURL)->SetWindowText(GetResString(IDS_UPDATEIPCURL));
		GetDlgItem(IDC_URL_FOR_UPDATING2)->SetWindowText(GetResString(IDS_URL_FOR_UPDATING));
		GetDlgItem(IDC_RESETIPFURL)->SetWindowText(GetResString(IDS_RESET));
		//MORPH END   - Added by Yun.SF3: Ipfilter.dat update

		}
}

void CPPgSpe3::OnBnClickedUpdatefakes()
{
	OnApply();
	theApp.FakeCheck->DownloadFakeList();
	CString strBuffer;
	strBuffer.Format(_T("v.%u"), thePrefs.GetFakesDatVersion());
	GetDlgItem(IDC_FAKELIST_VERSION)->SetWindowText(strBuffer);
}

void CPPgSpe3::OnBnClickedResetfakes()
{
	CString strBuffer = _T("http://donkeyfakes.gambri.net/public/ed2kfakes.txt");
	GetDlgItem(IDC_UPDATE_URL_FAKELIST)->SetWindowText(strBuffer);
	thePrefs.m_FakesDatVersion = 0;
	strBuffer.Format(_T("v.%u"), thePrefs.GetFakesDatVersion());
	GetDlgItem(IDC_FAKELIST_VERSION)->SetWindowText(strBuffer);
}
//MORPH START added by Yun.SF3: Ipfilter.dat update
void CPPgSpe3::OnBnClickedUpdateipfurl()
{
	OnApply();
	theApp.ipfilter->UpdateIPFilterURL();
	CString strBuffer;
	strBuffer.Format(_T("v.%u"), thePrefs.GetIPfilterVersion());
	GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(strBuffer);
}
//MORPH END added by Yun.SF3: Ipfilter.dat update

void CPPgSpe3::OnBnClickedResetipfurl()
{
	CString strBuffer = _T("http://emulepawcio.sourceforge.net/ed2kipfilter.txt");
	GetDlgItem(IDC_UPDATE_URL_IPFILTER)->SetWindowText(strBuffer);
	thePrefs.m_IPfilterVersion = 0;
	strBuffer.Format(_T("v.%u"), thePrefs.GetIPfilterVersion());
	GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(strBuffer);

}


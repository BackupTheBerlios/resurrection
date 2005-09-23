#include "stdafx.h"
#include "emule.h"
#include "PPgNe.h"
#include "emuledlg.h"
#include "serverWnd.h" 
#include "OtherFunctions.h"
#include "UserMsgs.h"
#include "Preferences.h"
#include ".\ppgne.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	DFLT_MAXCONPERFIVE	45

#pragma warning(disable:4800)

IMPLEMENT_DYNAMIC(CPPgNe, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgNe, CPropertyPage)
	ON_WM_HSCROLL()	
	ON_STN_CLICKED(IDC_NICEHASH_RIGHT, OnStnClickedNicehashRight)
		//>>> WiZaRd - SP - AutoHL
	ON_EN_CHANGE(IDC_AutoHLUpdate, OnSettingsChange)
	ON_BN_CLICKED(IDC_AutoHL, OnBnClickedAutoHL)
	ON_EN_CHANGE(IDC_MinAutoHL, OnSettingsChange) 
	ON_EN_CHANGE(IDC_MaxAutoHL, OnSettingsChange)  
	ON_EN_CHANGE(IDC_MaxSourcesHL, OnSettingsChange) 
	//<<< WiZaRd - SP - AutoHL
	ON_BN_CLICKED(IDC_REASKSRCAFTERIPCHANGE, OnSettingsChange) // [Maella/sivka: -ReAsk SRCs after IP Change-]
	//  [TPT] - quick start
	ON_BN_CLICKED(IDC_PPG_PHOENIX_QUICKSTART, OnBnClickedQuickStart)	
	ON_EN_CHANGE(IDS_QUICK_START_MAX_CONN, OnSettingsChange)
	ON_EN_CHANGE(IDS_QUICK_START_MAX_CONN_PER_FIVE, OnSettingsChange)
	ON_EN_CHANGE(IDS_QUICK_START_MINUTES, OnSettingsChange)
	ON_BN_CLICKED(IDC_QUICKSTARTAFTERIPCHANGE, OnSettingsChange) // [ionix] quickstart after ip changeON_WM_DESTROY()
		ON_BN_CLICKED(IDC_AutoHL_Groupbox, OnBnClickedAutohlGroupbox)
	ON_BN_CLICKED(IDC_ANTINICKTHIEF, OnSettingsChange) 	// AntiNickThief 
	ON_EN_CHANGE(IDC_IsClientBanTime, OnSettingsChange) // AntiNickThief
	ON_BN_CLICKED(IDC_ANTIMODIDFAKER, OnSettingsChange) // Anti Mod Faker Version
	ON_BN_CLICKED(IDC_ANTILEECHERMOD, OnSettingsChange) // AntiLeecher
	ON_BN_CLICKED(IDC_LEECHERSECURELOG, OnSettingsChange) // LeecherSecureLog 
//Start download color
	ON_BN_CLICKED(IDC_EnableDownloadInColor, OnDLColorChange)
	ON_CBN_SELCHANGE(IDC_DownloadColor, OnSettingsChange)
ON_BN_CLICKED(IDC_EnableDownloadInBold, OnSettingsChange)
	ON_BN_CLICKED(IDC_UploadColor, OnSettingsChange)	
//End download color

ON_STN_CLICKED(IDC_NICEHASH_STATIC, OnStnClickedNicehashStatic)
	//sivka aggressive client handling
   ON_BN_CLICKED(IDC_CHECK_SIVKABAN, OnBnClickedSivkaBan)	// überprüfung und zuweisung einer aufgabe
   ON_BN_CLICKED(IDC_CHECK_SIVKA_BANLOG, OnSettingsChange)	
   ON_EN_CHANGE(IDC_EDIT_SIVKA_TIME, OnSettingsChange) 
   ON_EN_CHANGE(IDC_EDIT_SIVKA_ASK, OnSettingsChange) 
   //sivka aggressive client handling
END_MESSAGE_MAP()


CPPgNe::CPPgNe()
	: CPropertyPage(CPPgNe::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	m_bInitializedTreeOpts = false;
 
}

CPPgNe::~CPPgNe()
{
}

///------

BOOL CPPgNe::OnInitDialog()
{

CPropertyPage::OnInitDialog();

((CEdit*)GetDlgItem(IDS_QUICK_START_MAX_CONN))->SetLimitText(4); // [TPT] - quick start
	((CEdit*)GetDlgItem(IDS_QUICK_START_MAX_CONN_PER_FIVE))->SetLimitText(3); // [TPT] - quick start
	((CEdit*)GetDlgItem(IDS_QUICK_START_MINUTES))->SetLimitText(2); // [TPT] - quick start
	
	

//Start download color
((CComboBox*)GetDlgItem(IDC_DownloadColor))->InsertString(0,GetResString(IDS_DC_RED));
((CComboBox*)GetDlgItem(IDC_DownloadColor))->InsertString(1,GetResString(IDS_DC_BLUE));
((CComboBox*)GetDlgItem(IDC_DownloadColor))->InsertString(2,GetResString(IDS_DC_GREEN));
((CComboBox*)GetDlgItem(IDC_DownloadColor))->InsertString(3,GetResString(IDS_DC_YELLOW));
((CComboBox*)GetDlgItem(IDC_DownloadColor))->InsertString(4,GetResString(IDS_DC_GREY));
//Spe64 Add more color
((CComboBox*)GetDlgItem(IDC_DownloadColor))->InsertString(5,GetResString(IDS_DC_VIOLET));
((CComboBox*)GetDlgItem(IDC_DownloadColor))->InsertString(6,GetResString(IDS_DC_BROWN));
((CComboBox*)GetDlgItem(IDC_DownloadColor))->InsertString(7,GetResString(IDS_DC_BURLYWOOD));
((CComboBox*)GetDlgItem(IDC_DownloadColor))->InsertString(8,GetResString(IDS_DC_ORANGE));
//End download color
//Start download color

if(thePrefs.EnableDownloadInColor)
CheckDlgButton(IDC_EnableDownloadInColor,1);
else
CheckDlgButton(IDC_EnableDownloadInColor,0);
((CComboBox*)GetDlgItem(IDC_DownloadColor))->SetCurSel(thePrefs.DownloadColor);
CheckDlgButton(IDC_EnableDownloadInBold,(bool) thePrefs.m_bShowActiveDownloadsBold);
CheckDlgButton(IDC_UploadColor,(bool) thePrefs.UploadColor);	

//End download color
	// Nicehash from iONiX
	m_iNiceHashWeight = thePrefs.m_iNiceHashWeight;
	((CSliderCtrl*)GetDlgItem(IDC_NICEHASH_WEIGHT))->SetRange(0, 100, true); 
	((CSliderCtrl*)GetDlgItem(IDC_NICEHASH_WEIGHT))->SetPos(m_iNiceHashWeight); 
	((CSliderCtrl*)GetDlgItem(IDC_NICEHASH_WEIGHT))->SetTicFreq(10);                     
	// Nicehash 
	
	LoadSettings();
	InitWindowStyles(this);
	Localize();
	return TRUE;   // retourne TRUE  sauf si vous avez défini le focus sur un contrôle
}

void CPPgNe::Localize(void)
{
	CString temp;
	if (m_hWnd)
	{

	// Nicehash from iONiX

		temp.Format(GetResString(IDS_NICEHASH_STATIC)+_T(": %s %%"), GetFormatedUInt(m_iNiceHashWeight)); 
		GetDlgItem(IDC_NICEHASH_STATIC)->SetWindowText(temp);
		GetDlgItem(IDC_NICEHASH_LEFT)->SetWindowText(GetResString(IDS_NICEHASH_LEFT));
		GetDlgItem(IDC_NICEHASH_RIGHT)->SetWindowText(GetResString(IDS_NICEHASH_RIGHT)); 
		// Nicehash 
		GetDlgItem(IDC_REASKSRCAFTERIPCHANGE)->SetWindowText(GetResString(IDS_REASKSRCAFTERIPCHANGE));
		GetDlgItem(IDC_QUICKSTART_BOX)->SetWindowText(GetResString(IDS_QUICKSTART));
		GetDlgItem(IDC_PPG_PHOENIX_QUICKSTART)->SetWindowText(GetResString(IDS_PPG_PHOENIX_QUICKSTART));
		GetDlgItem(IDC_PPG_PHOENIX_QUICKSTART_CONN_STATIC)->SetWindowText(GetResString(IDS_PPG_PHOENIX_QUICKSTART_CONN_STATIC));
		GetDlgItem(IDC_PPG_PHOENIX_QUICKSTART_CONN_PER_FIVE_STATIC)->SetWindowText(GetResString(IDS_PPG_PHOENIX_QUICKSTART_CONN_PER_FIVE_STATIC));
		GetDlgItem(IDC_QUICKSTARTAFTERIPCHANGE)->SetWindowText(GetResString(IDS_QUICKSTARTAFTERIPCHANGE));
		GetDlgItem(IDC_QUICK_START_MINUTES_STATIC)->SetWindowText(GetResString(IDS_QUICK_START_MINUTES_STATIC));
        GetDlgItem(IDC_AutoHL_Groupbox)->SetWindowText(GetResString(IDS_AutoHL_Groupbox));
		GetDlgItem(IDC_AutoHL)->SetWindowText(GetResString(IDS_AutoHL));
		GetDlgItem(IDC_MinAutoHL_static)->SetWindowText(GetResString(IDS_MinAutoHL_static));
		GetDlgItem(IDC_MaxAutoHL_static)->SetWindowText(GetResString(IDS_MaxAutoHL_static));
		GetDlgItem(IDC_MaxSourcesHL_static)->SetWindowText(GetResString(IDS_MaxSourcesHL_static));
		GetDlgItem(IDC_AutoHLUpdate_static)->SetWindowText(GetResString(IDS_AutoHLUpdate_static));
		GetDlgItem(IDC_AutoHLUpdate_static2)->SetWindowText(GetResString(IDS_AutoHLUpdate_static2));
	    GetDlgItem(IDC_EnableDownloadInColor)->SetWindowText(GetResString(IDS_EnableDownloadInColor)); //Spe64
	    GetDlgItem(IDC_EnableDownloadInBold)->SetWindowText(GetResString(IDS_EnableDownloadInBold)); //Spe64
		GetDlgItem(IDC_UploadColor)->SetWindowText(GetResString(IDS_UploadColor));//lama

		GetDlgItem(IDC_CHECK_SIVKABAN)->SetWindowText(GetResString(IDS_CHECK_SIVKABAN));//lama
		GetDlgItem(IDC_CHECK_SIVKA_BANLOG)->SetWindowText(GetResString(IDS_CHECK_SIVKA_BANLOG));//lama
		GetDlgItem(IDC_IsClientBanTime_static)->SetWindowText(GetResString(IDS_ISCLIENTBANTIME_STATIC));//lama

	}
}

BOOL CPPgNe::OnApply()
{
	CString sBuffer;

//sivka aggressive client handling 
	thePrefs.m_bUseSivkaBan = IsDlgButtonChecked(IDC_CHECK_SIVKABAN);
	thePrefs.m_bLogSivkaBan = IsDlgButtonChecked(IDC_CHECK_SIVKA_BANLOG);
	
	GetDlgItem(IDC_EDIT_SIVKA_ASK)->GetWindowText(sBuffer);
	thePrefs.m_uiSivkaAskCount = _tstoi(sBuffer); 
	GetDlgItem(IDC_EDIT_SIVKA_TIME)->GetWindowText(sBuffer);
	thePrefs.m_uiSivkaTimeCount = _tstoi(sBuffer); 

	thePrefs.m_uiSivkaAskCount = max(4,  thePrefs.m_uiSivkaAskCount);
	thePrefs.m_uiSivkaAskCount = min(25,  thePrefs.m_uiSivkaAskCount);
	thePrefs.m_uiSivkaTimeCount = max(5,  thePrefs.m_uiSivkaTimeCount);
	thePrefs.m_uiSivkaTimeCount = min(12,  thePrefs.m_uiSivkaTimeCount);
	//sivka aggressive client handling 
	//>>> WiZaRd - SP - AutoHL
	if(GetDlgItem(IDC_AutoHLUpdate)->GetWindowTextLength())
	{
		GetDlgItem(IDC_AutoHLUpdate)->GetWindowText(sBuffer);
		thePrefs.m_uiAutoHLUpdateTimer = (_tstoi(sBuffer)) ? _tstoi(sBuffer) : 300;
	}

	thePrefs.m_bUseAutoHL = IsDlgButtonChecked(IDC_AutoHL)!=0;
		
	if(GetDlgItem(IDC_MinAutoHL)->GetWindowTextLength())
	{
		GetDlgItem(IDC_MinAutoHL)->GetWindowText(sBuffer);
		thePrefs.m_uiMinAutoHL = (_tstoi(sBuffer)) ? _tstoi(sBuffer) : 25;
	}

	if(GetDlgItem(IDC_MaxAutoHL)->GetWindowTextLength())
	{
		GetDlgItem(IDC_MaxAutoHL)->GetWindowText(sBuffer);
		thePrefs.m_uiMaxAutoHL = (_tstoi(sBuffer)) ? _tstoi(sBuffer) : 1500;
	}

	if(GetDlgItem(IDC_MaxSourcesHL)->GetWindowTextLength())
	{
		GetDlgItem(IDC_MaxSourcesHL)->GetWindowText(sBuffer);
		thePrefs.m_uiMaxSourcesHL = (_tstoi(sBuffer)) ? _tstoi(sBuffer) : 7500;
	}
	//<<< WiZaRd - SP - AutoHL

		// >>> [Maella/sivka: -ReAsk SRCs after IP Change-]
	thePrefs.bReAskSRCAfterIPChange = IsDlgButtonChecked(IDC_REASKSRCAFTERIPCHANGE)!=0;
	// >>> [Maella/sivka: -ReAsk SRCs after IP Change-]
	// [TPT] - quick start
	GetDlgItem(IDS_QUICK_START_MAX_CONN)->GetWindowText(sBuffer); 
	thePrefs.SetQuickStartMaxCon(_tstoi(sBuffer));
	if(thePrefs.GetQuickStartMaxCon() < 100) //kleiner 100 geht nicht 
	{ 
		thePrefs.SetQuickStartMaxCon(1201); // Startwert = 1201
		sBuffer.Format(_T("%d"),thePrefs.GetQuickStartMaxCon()); 
		GetDlgItem(IDS_QUICK_START_MAX_CONN)->SetWindowText(sBuffer); 
	} 
	else if(thePrefs.GetQuickStartMaxCon() >2000) //grösser 2000  wollen wir nicht.... 
	{ 
		thePrefs.SetQuickStartMaxCon(2000); 
		sBuffer.Format(_T("%d"),thePrefs.GetQuickStartMaxCon()); 
		GetDlgItem(IDS_QUICK_START_MAX_CONN)->SetWindowText(sBuffer); 
	} 
	GetDlgItem(IDS_QUICK_START_MAX_CONN_PER_FIVE)->GetWindowText(sBuffer); 
	thePrefs.SetQuickStartMaxConPerFive(_tstoi(sBuffer));
	if(thePrefs.GetQuickStartMaxConPerFive() < 50) //kleiner 50 geht nicht 
	{ 
		thePrefs.SetQuickStartMaxConPerFive(151); // Startwert = 151
		sBuffer.Format(_T("%d"),thePrefs.GetQuickStartMaxConPerFive()); 
		GetDlgItem(IDS_QUICK_START_MAX_CONN_PER_FIVE)->SetWindowText(sBuffer); 
	} 
	else if(thePrefs.GetQuickStartMaxConPerFive() >200) //grösser 200  wollen wir nicht.... 
	{ 
		thePrefs.SetQuickStartMaxConPerFive(200); 
		sBuffer.Format(_T("%d"),thePrefs.GetQuickStartMaxConPerFive()); 
		GetDlgItem(IDS_QUICK_START_MAX_CONN_PER_FIVE)->SetWindowText(sBuffer); 
	}
	GetDlgItem(IDS_QUICK_START_MINUTES)->GetWindowText(sBuffer); 
	thePrefs.SetQuickStartMinutes(_tstoi(sBuffer));
	if(thePrefs.GetQuickStartMinutes() < 5) //kleiner 5 geht nicht 
	{ 
		thePrefs.SetQuickStartMinutes(10); // Startwert = 151
		sBuffer.Format(_T("%d"),thePrefs.GetQuickStartMinutes()); 
		GetDlgItem(IDS_QUICK_START_MINUTES)->SetWindowText(sBuffer); 
}
	else if(thePrefs.GetQuickStartMinutes() >30) //grösser 30  wollen wir nicht.... 
{
		thePrefs.SetQuickStartMinutes(10); 
		sBuffer.Format(_T("%d"),thePrefs.GetQuickStartMinutes()); 
		GetDlgItem(IDS_QUICK_START_MINUTES)->SetWindowText(sBuffer); 
	}
	thePrefs.m_QuickStart = IsDlgButtonChecked(IDC_PPG_PHOENIX_QUICKSTART)!=0; 
	// [TPT] - quick start

	// [ionix] quickstart after ip change
	thePrefs.m_bQuickStartAfterIPChange = IsDlgButtonChecked(IDC_QUICKSTARTAFTERIPCHANGE)!=0; 
	// [ionix] quickstart after ip change

// AntiNickThief
	GetDlgItem(IDC_IsClientBanTime)->GetWindowText(sBuffer); 
	thePrefs.SetClientBanTime(_tstoi(sBuffer)?min(_tstoi(sBuffer), 25):4);
	thePrefs.m_bAntiNickThief = IsDlgButtonChecked(IDC_ANTINICKTHIEF);
	// AntiNickThief

	// Anti Mod Faker Version 
	thePrefs.m_bAntiModIdFaker = IsDlgButtonChecked(IDC_ANTIMODIDFAKER);
	// Anti Mod Faker Version

	// AntiLeecher
	thePrefs.m_bAntiLeecherMod = IsDlgButtonChecked(IDC_ANTILEECHERMOD);
	// AntiLeecher

	// LeecherSecureLog 
	thePrefs.m_bLeecherSecureLog = IsDlgButtonChecked(IDC_LEECHERSECURELOG);
	// LeecherSecureLog 
	thePrefs.m_iNiceHashWeight = m_iNiceHashWeight;	// Nicehash FROM iONiX
//Start Download color
    thePrefs.EnableDownloadInColor = IsDlgButtonChecked(IDC_EnableDownloadInColor)!=0;
	thePrefs.DownloadColor = ((CComboBox*)GetDlgItem(IDC_DownloadColor))->GetCurSel();
thePrefs.m_bShowActiveDownloadsBold = IsDlgButtonChecked(IDC_EnableDownloadInBold)!=0;
	thePrefs.UploadColor = IsDlgButtonChecked(IDC_UploadColor)!=0;	
//End Download color

	LoadSettings();
	return CPropertyPage::OnApply();
}



void CPPgNe::OnDestroy()
{	
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;
	


	CPropertyPage::OnDestroy();
}
void CPPgNe::LoadSettings(void){

if(m_hWnd)
{
        CString strBuffer;
	 
 //sivka aggressive client handling 
		CheckDlgButton(IDC_CHECK_SIVKABAN, thePrefs.IsSivkaBan());//wert auslesen 
        CheckDlgButton(IDC_CHECK_SIVKA_BANLOG, thePrefs.IsSivkaBanLog());

		strBuffer.Format(_T("%u"), thePrefs.GetSivkaTimeCount()); //wir erstellen uns einen string
		GetDlgItem(IDC_EDIT_SIVKA_TIME)->SetWindowText(strBuffer);// edit box setzen
		strBuffer.Format(_T("%u"), thePrefs.GetSivkaAskCount()); 
		GetDlgItem(IDC_EDIT_SIVKA_ASK)->SetWindowText(strBuffer);
		
		if(!thePrefs.IsSivkaBan())
		{
			GetDlgItem(IDC_EDIT_SIVKA_TIME)->EnableWindow(false);//ausgrauen der felder
			GetDlgItem(IDC_EDIT_SIVKA_ASK)->EnableWindow(false);
			GetDlgItem(IDC_CHECK_SIVKA_BANLOG)->EnableWindow(false); 
		}  
		    
		//sivka aggressive client handling 
	 
		//>>> WiZaRd - SP - AutoHL
		strBuffer.Format(_T("%d"), thePrefs.GetAutoHLUpdateTimer());
		GetDlgItem(IDC_AutoHLUpdate)->SetWindowText(strBuffer);

		CheckDlgButton(IDC_AutoHL, thePrefs.IsUseAutoHL());

		strBuffer.Format(_T("%d"), thePrefs.GetMinAutoHL());
		GetDlgItem(IDC_MinAutoHL)->SetWindowText(strBuffer);

		strBuffer.Format(_T("%d"), thePrefs.GetMaxAutoHL());
		GetDlgItem(IDC_MaxAutoHL)->SetWindowText(strBuffer);

		strBuffer.Format(_T("%d"), thePrefs.GetMaxSourcesHL());
		GetDlgItem(IDC_MaxSourcesHL)->SetWindowText(strBuffer);
		//<<< WiZaRd - SP - AutoHL

// >>> [Maella/sivka: -ReAsk SRCs after IP Change-]
		CheckDlgButton(IDC_REASKSRCAFTERIPCHANGE, thePrefs.bReAskSRCAfterIPChange);
		// <<< [Maella/sivka: -ReAsk SRCs after IP Change-]

	// [TPT] - quick start
		GetDlgItem(IDS_QUICK_START_MAX_CONN)->EnableWindow(thePrefs.GetQuickStart()); 
		GetDlgItem(IDS_QUICK_START_MAX_CONN_PER_FIVE)->EnableWindow(thePrefs.GetQuickStart()); 
		GetDlgItem(IDS_QUICK_START_MINUTES)->EnableWindow(thePrefs.GetQuickStart()); 

		strBuffer.Format(_T("%d"), thePrefs.GetQuickStartMaxCon()); 
		GetDlgItem(IDS_QUICK_START_MAX_CONN)->SetWindowText(strBuffer); 
		strBuffer.Format(_T("%d"), thePrefs.GetQuickStartMaxConPerFive());
		GetDlgItem(IDS_QUICK_START_MAX_CONN_PER_FIVE)->SetWindowText(strBuffer);
		strBuffer.Format(_T("%d"), thePrefs.GetQuickStartMinutes());
		GetDlgItem(IDS_QUICK_START_MINUTES)->SetWindowText(strBuffer);
		CheckDlgButton(IDC_PPG_PHOENIX_QUICKSTART, thePrefs.GetQuickStart()); 
		// [TPT] - quick start
		// [ionix] quickstart after ip change
		CheckDlgButton(IDC_QUICKSTARTAFTERIPCHANGE, thePrefs.GetQuickStartAfterIPChange()); 
		// [ionix] quickstart after ip change

// AntiNickThief
		GetDlgItem(IDC_IsClientBanTime)->EnableWindow(true); 
		strBuffer.Format(_T("%d"), thePrefs.GetClientBanTime()); 
		GetDlgItem(IDC_IsClientBanTime)->SetWindowText(strBuffer);  
		CheckDlgButton(IDC_ANTINICKTHIEF, thePrefs.GetAntiNickThief());
		// AntiNickThief 

		// Anti Mod Faker Version
		CheckDlgButton(IDC_ANTIMODIDFAKER, thePrefs.IsAntiModIdFaker());
		// Anti Mod Faker Version

		// AntiLeecher 
		CheckDlgButton(IDC_ANTILEECHERMOD, thePrefs.IsLeecherSecure());
		// AntiLeecher 

		// LeecherSecureLog 
		CheckDlgButton(IDC_LEECHERSECURELOG, thePrefs.IsLeecherSecureLog());
		// LeecherSecureLog 

}
}


BOOL CPPgNe::OnKillActive()
{
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}


BOOL CPPgNe::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

//>>> WiZaRd - SP - AutoHL
void CPPgNe::OnBnClickedAutoHL()
{
	if(IsDlgButtonChecked(IDC_AutoHL)!=0)
	{
		GetDlgItem(IDC_AutoHLUpdate)->EnableWindow(true);
		GetDlgItem(IDC_MinAutoHL)->EnableWindow(true);
		GetDlgItem(IDC_MaxAutoHL)->EnableWindow(true); 
		GetDlgItem(IDC_MaxSourcesHL)->EnableWindow(true); 
	}
	else
	{
		GetDlgItem(IDC_AutoHLUpdate)->EnableWindow(false);
		GetDlgItem(IDC_MinAutoHL)->EnableWindow(false);
		GetDlgItem(IDC_MaxAutoHL)->EnableWindow(false); 
		GetDlgItem(IDC_MaxSourcesHL)->EnableWindow(false);
	}
	SetModified();
}
//<<< WiZaRd - SP - AutoHL

void CPPgNe::OnBnClickedQuickStart()
{	
	if(IsDlgButtonChecked(IDC_PPG_PHOENIX_QUICKSTART)!=0)
	{
		GetDlgItem(IDS_QUICK_START_MAX_CONN)->EnableWindow(true);
		GetDlgItem(IDS_QUICK_START_MAX_CONN_PER_FIVE)->EnableWindow(true);
		GetDlgItem(IDS_QUICK_START_MINUTES)->EnableWindow(true);
		GetDlgItem(IDC_QUICKSTARTAFTERIPCHANGE)->EnableWindow(true);
	}
	else
	{
		GetDlgItem(IDS_QUICK_START_MAX_CONN)->EnableWindow(false);
		GetDlgItem(IDS_QUICK_START_MAX_CONN_PER_FIVE)->EnableWindow(false);
		GetDlgItem(IDS_QUICK_START_MINUTES)->EnableWindow(false);
		GetDlgItem(IDC_QUICKSTARTAFTERIPCHANGE)->EnableWindow(false);
	}
	SetModified();
}

void CPPgNe::OnBnClickedAutohlGroupbox()
{
}
void CPPgNe::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// Nicehash from iONiX

	if (pScrollBar == GetDlgItem(IDC_NICEHASH_WEIGHT))
	{
		m_iNiceHashWeight = ((CSliderCtrl*)pScrollBar)->GetPos() * 1;
		CString temp;
		temp.Format(GetResString(IDS_NICEHASH_STATIC)+_T(": %s %%"), GetFormatedUInt(m_iNiceHashWeight)); 
		GetDlgItem(IDC_NICEHASH_STATIC)->SetWindowText(temp);
		SetModified(TRUE);
	}
	UpdateData(false); 
	// Nicehash 
}
void CPPgNe::OnStnClickedNicehashRight()
{
	// TODO: Add your control notification handler code here
}

void CPPgNe::OnStnClickedNicehashStatic()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
}
void CPPgNe::OnDLColorChange()
{	
	GetDlgItem(IDC_DownloadColor)->EnableWindow(IsDlgButtonChecked(IDC_EnableDownloadInColor)!=0); 
	SetModified();
}//sivka aggressive client handling
void CPPgNe::OnBnClickedSivkaBan()
{
	if(IsDlgButtonChecked(IDC_CHECK_SIVKABAN))//wenn sivka ban AN ist 
	{
		GetDlgItem(IDC_EDIT_SIVKA_TIME)->EnableWindow(true);//aktiviere check und editboxen
        GetDlgItem(IDC_EDIT_SIVKA_ASK)->EnableWindow(true);
        GetDlgItem(IDC_CHECK_SIVKA_BANLOG)->EnableWindow(true);
	}
	else
	{
	   GetDlgItem(IDC_EDIT_SIVKA_TIME)->EnableWindow(false);//ansonsten lass sie grau 
       GetDlgItem(IDC_EDIT_SIVKA_ASK)->EnableWindow(false);
	   GetDlgItem(IDC_CHECK_SIVKA_BANLOG)->EnableWindow(false);
	}
    SetModified();
}
//sivka aggressive client handling

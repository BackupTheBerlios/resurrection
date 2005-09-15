// PPgVipeR.cpp : implementation file
#include "stdafx.h"
#include "emule.h"
#include "PPgVipeR.h"
#include "emuledlg.h"
#include "serverWnd.h" 
#include "OtherFunctions.h"
#include "UserMsgs.h"
#include "Preferences.h"
// IP-to-Country +
#include "IP2Country.h"
// IP-to-Country -
#include "ClientCredits.h" //Credit Syst Spe64


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable:4800)

#define	DFLT_MAXCONPERFIVE	45



IMPLEMENT_DYNAMIC(CPPgVipeR, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgVipeR, CPropertyPage)
	ON_WM_HSCROLL()
ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify) //spe64 fix apply button
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CPPgVipeR::CPPgVipeR()
	: CPropertyPage(CPPgVipeR::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
	, VP(0)
{
	m_bInitializedTreeOpts = false;
//==> Chunk Selection Patch by Xman [shadow2004]
	m_iEnableCSP = 0;
//<== Chunk Selection Patch by Xman [shadow2004]
//Telp + Menu VipeR
	m_AdditionalVipeR = NULL;
//Telp - Menu VipeR
	m_secu = NULL; 
m_htiClientPerc = NULL;
//Telp Start payback first
	m_htiPBF = NULL;
//Telp End payback first
 //Telp Start push small file
    m_htiEnablePushSmallFile = NULL; 
//Telp End push small file  
//Telp Start push rare file
    m_htiEnablePushRareFile = NULL; 
//Telp End push rare file

// CreditSystem
	m_htiCreditSystem = NULL;
	m_htiOfficialCredit = NULL;
	m_htiLovelaceCredit = NULL;
	m_htiSwatCredit = NULL;
	m_htiRatioCredit = NULL;
	m_htiPawcioCredit = NULL;
	m_htiESCredit = NULL; 
	m_htiFineCredit = NULL; //Spe64 add
	m_htiTK4Credit = NULL; //Spe64	added
        m_htiCreditsNone = NULL;
	
	// CreditSystem

//==> Chunk Selection Patch by Xman [lama]
	m_htiEnableCSP = NULL;
//<== Chunk Selection Patch by Xman [lama]

// IP-to-Country +
	m_htiIP2CountryName = NULL;
	m_htiIP2CountryName_DISABLE = NULL;
	m_htiIP2CountryName_SHORT = NULL;
	m_htiIP2CountryName_MID = NULL;
	m_htiIP2CountryName_LONG = NULL;
	m_htiIP2CountryShowFlag = NULL;
	// IP-to-Country -
// ==> Anti Uploader Ban - Stulle
	m_htiAntiUploaderBanLimit = NULL;
	m_htiAntiCase1 = NULL;
	m_htiAntiCase2 = NULL;
	m_htiAntiCase3 = NULL;
	// <== Anti Uploader Ban - Stulle
	//Ackronic START - Aggiunto da Aenarion[ITA] - Drop
	m_htiDropSources = NULL;
	m_htiDropSourcesTimerNNS = NULL;
	m_htiDropSourcesTimerFQ = NULL;
	m_htiDropSourcesTimerHQR = NULL;
	m_htiDropSourcesNNS = NULL;
	m_htiDropSourcesFQ = NULL;
	m_htiDropSourcesHQR = NULL;
    m_htiHqrBox = NULL;
	//Ackronic END - Aggiunto da Aenarion[ITA] - Drop
}

/////////////////////////////////////////////////////////////////////

CPPgVipeR::~CPPgVipeR()
{
}

void CPPgVipeR::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	
	DDX_Control(pDX, IDC_VP_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
//Telp+ Menu VipeR
		int iImgAddTweaks = 8;
//Telp- Menu VipeR
//==> Chunk Selection Patch by Xman [lama]
		int iImgCSP = 8;
//<== Chunk Selection Patch by Xman [lama]
int iImgSecu = 8; 
// IP-to-Country +
		int iImgIP2Country = 8;
		// IP-to-Country -
int iImgCS = 8; // Creditsystems
int iImgUM = 8; // default icon
		int iImgDrop = 8;//Ackronic - Aggiunto da Aenarion[ITA] - Drop
		int iImgPS = 8;
		int iImgSFM = 8;
CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
//Telp+ Menu VipeR
		iImgAddTweaks = piml->Add(CTempIconLoader(_T("TWEAK")));
//Telp- Menu VipeR
          iImgSecu = piml->Add(CTempIconLoader(_T("SECURITY")));
                   // IP-to-Country +
			iImgIP2Country = piml->Add(CTempIconLoader(_T("SEARCHMETHOD_GLOBAL"))); 
	         // IP-to-Country -	
iImgUM = piml->Add(CTempIconLoader(_T("UPLOAD")));
			iImgPS = piml->Add(CTempIconLoader(_T("Kadcontactlist")));
			iImgSFM = piml->Add(CTempIconLoader(_T("Sharedfiles")));
			// <--- Morph: PowerShare
 //<<-- ADDED STORMIT -  Morph: PowerShared //
					iImgDrop = piml->Add(CTempIconLoader(_T("DROP")));//Ackronic - Aggiunto da Aenarion[ITA] - Drop

//Telp+ Menu VipeR
		m_AdditionalVipeR = m_ctrlTreeOptions.InsertGroup(_T("Misc Functions "), iImgAddTweaks, TVI_ROOT);
//Telp- Menu VipeR 
		m_secu = m_ctrlTreeOptions.InsertGroup(_T("Security"), iImgSecu, m_AdditionalVipeR);
		m_ctrlTreeOptions.Expand(m_secu, TVE_EXPAND);
		m_ctrlTreeOptions.SetItemState(m_secu, TVIS_BOLD, TVIS_BOLD);

iImgCS = piml->Add(CTempIconLoader(_T("STATSCLIENTS"))); // Creditsystems
	//==> Chunk Selection Patch by Xman [lama]
			iImgCSP = piml->Add(CTempIconLoader(_T("CONVERT")));
//<== Chunk Selection Patch by Xman [lama]

// IP-to-Country +
		m_htiIP2CountryName = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_IP2COUNTRY), iImgIP2Country, TVI_ROOT);
		m_ctrlTreeOptions.SetItemState(m_htiIP2CountryName, TVIS_BOLD, TVIS_BOLD);
		m_htiIP2CountryName_DISABLE = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DISABLED), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_DISABLE);
		m_htiIP2CountryName_SHORT = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_SHORT), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_SHORT);
		m_htiIP2CountryName_MID = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_MID), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_MID);
		m_htiIP2CountryName_LONG = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_LONG), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_LONG);
		m_htiIP2CountryShowFlag = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_COUNTRYNAME_SHOWFLAG), m_htiIP2CountryName, m_bIP2CountryShowFlag);
		// IP-to-Country -

		//Telp Start payback first
		m_htiPBF = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PAYBACK_FIRST), m_AdditionalVipeR, m_bPBF);
//Telp End payback first
}
		//client percentage
		m_htiClientPerc = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CLIENT_PERC), m_AdditionalVipeR, m_bEnableClientPerc);
		m_ctrlTreeOptions.SetItemState(m_AdditionalVipeR, TVIS_BOLD, TVIS_BOLD);
		//client percentage
// ==> Anti Uploader Ban - Stulle
		m_htiAntiUploaderBanLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_UNBAN_UPLOADER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_secu);
		m_ctrlTreeOptions.AddEditBox(m_htiAntiUploaderBanLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAntiCase1 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_1), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 0);
		m_htiAntiCase2 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_2), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 1);
		m_htiAntiCase3 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_3), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 2);
		m_ctrlTreeOptions.Expand(m_htiAntiUploaderBanLimit, TVE_EXPAND);
		// <== Anti Uploader Ban - Stulle
	//Ackronic START - Aggiunto da Aenarion[ITA] - Drop
		m_htiDropSources = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DROPS), iImgDrop, TVI_ROOT);
		m_htiDropSourcesNNS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DROPS1), m_htiDropSources, m_iDropSources = 1);
		m_htiDropSourcesTimerNNS = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DROPS2), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDropSources);
		m_ctrlTreeOptions.AddEditBox(m_htiDropSourcesTimerNNS, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiDropSourcesFQ = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DROPS3), m_htiDropSources, m_iDropSources = 3);
		m_htiDropSourcesTimerFQ = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DROPS2), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDropSources);
		m_ctrlTreeOptions.AddEditBox(m_htiDropSourcesTimerFQ, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiDropSourcesHQR = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DROPS4), m_htiDropSources, m_iDropSources = 7);
		m_htiDropSourcesTimerHQR = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DROPS2), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDropSources);
		m_ctrlTreeOptions.AddEditBox(m_htiDropSourcesTimerHQR, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiHqrBox = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DROPHQSLIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDropSources);
		m_ctrlTreeOptions.AddEditBox(m_htiHqrBox , RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiDropSources, TVE_EXPAND);
		//Ackronic END - Aggiunto da Aenarion[ITA] - Drop
	//==> Chunk Selection Patch by Xman [lama]
		m_htiEnableCSP = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_CSP_ENABLE), iImgCSP, m_htiSFM);
		m_ctrlTreeOptions.SetItemState(m_htiEnableCSP, TVIS_BOLD, TVIS_BOLD);
		m_htiEnableCSPNormal = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CSP_NORMAL), m_htiEnableCSP, m_iEnableCSP == 0);
		m_htiEnableCSPXman = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CSP_XMAN), m_htiEnableCSP, m_iEnableCSP == 1);
		m_ctrlTreeOptions.Expand(m_htiEnableCSP, TVE_EXPAND);
//<== Chunk Selection Patch by Xman [lama]
		m_ctrlTreeOptions.Expand(m_htiHideOS, TVE_EXPAND);
		
		//Telp Start push small file
	        m_htiEnablePushSmallFile = m_ctrlTreeOptions.InsertCheckBox(_T("Push Small Files"),m_htiSFM, m_bEnablePushSmallFile); //Hawkstar, push small file
//Telp End push small file
//Telp Start push rare file
       		 m_htiEnablePushRareFile = m_ctrlTreeOptions.InsertCheckBox(_T("Push Rare Files"),m_htiSFM, m_bEnablePushRareFile); //Hawkstar, push rare file
//Telp End push rare file

		// Creditsystem  
		m_htiCreditSystem = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_CREDIT_SYSTEM), iImgCS, TVI_ROOT);
		m_ctrlTreeOptions.SetItemState(m_htiCreditSystem, TVIS_BOLD, TVIS_BOLD);
		m_htiOfficialCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_OFFICIAL_CREDIT), m_htiCreditSystem, m_iCreditSystem == 0);
		m_htiLovelaceCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_LOVELACE_CREDIT), m_htiCreditSystem, m_iCreditSystem == 1);
                m_htiPeaceCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PEACE_CREDIT), m_htiCreditSystem, m_iCreditSystem == 2);
                m_htiSivkaCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SIVKA_CREDIT), m_htiCreditSystem, m_iCreditSystem == 3);
                m_htiRtCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_RT_CREDIT), m_htiCreditSystem, m_iCreditSystem == 4);
		m_htiSwatCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SWAT_CREDIT), m_htiCreditSystem, m_iCreditSystem == 5);
		m_htiPawcioCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PAWCIO_CREDIT), m_htiCreditSystem, m_iCreditSystem == 6);
		m_htiESCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_EASTSHARE_CREDIT), m_htiCreditSystem, m_iCreditSystem == 7);
        m_htiFineCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_FINE_CREDIT), m_htiCreditSystem, m_iCreditSystem == 8); //Add by Spe64
         m_htiTK4Credit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_TK4_CREDIT), m_htiCreditSystem, m_iCreditSystem == 9); //Add by Spe64
		m_htiCreditsNone = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CREDITS_NONE), m_htiCreditSystem, m_iCreditSystem == 10); //Spe64 change 
	
		// CreditSystem
}
		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;

 ///////////////////////////////////////////////////////////////////////////////////
 // IP-to-Country +
	DDX_TreeRadio(pDX, IDC_VP_OPTS, m_htiIP2CountryName, (int &)m_iIP2CountryName);
	DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiIP2CountryShowFlag, m_bIP2CountryShowFlag);
	// IP-to-Country -
 		DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiClientPerc, m_bEnableClientPerc);
//Telp Start payback First
	DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiPBF, m_bPBF);
//Telp End payback First
	//Telp Start push small file
    	DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiEnablePushSmallFile, m_bEnablePushSmallFile); //Hawkstar, push small file
//Telp End push small file
//Telp Start push rare file
	DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiEnablePushRareFile, m_bEnablePushRareFile); //eMulefan83 push rare file
//Telp End push rare file

DDX_TreeRadio(pDX, IDC_VP_OPTS, m_htiCreditSystem, (int &)m_iCreditSystem); // CreditSystem
 // ==> Anti Uploader Ban - Stulle
	DDX_TreeEdit(pDX, IDC_VP_OPTS, m_htiAntiUploaderBanLimit, m_iAntiUploaderBanLimit);
	DDV_MinMaxInt(pDX, m_iAntiUploaderBanLimit, 0, 20);
	DDX_TreeRadio(pDX, IDC_VP_OPTS, m_htiAntiUploaderBanLimit, (int &)m_iAntiUploaderBanCase);
	// ==> Anti Uploader Ban - Stulle
	//Ackronic START - Aggiunto da Aenarion[ITA] - Drop
		DDX_TreeEdit(pDX, IDC_VP_OPTS, m_htiHqrBox, iMaxRemoveQRS);
		DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiDropSourcesNNS, m_iDropSourcesNNS);
		DDX_TreeEdit(pDX, IDC_VP_OPTS, m_htiDropSourcesTimerNNS, m_iDropSourcesTimerNNS);
		DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiDropSourcesFQ, m_iDropSourcesFQ);
		DDX_TreeEdit(pDX, IDC_VP_OPTS, m_htiDropSourcesTimerFQ, m_iDropSourcesTimerFQ);
		DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiDropSourcesHQR, m_iDropSourcesHQR);
		DDX_TreeEdit(pDX, IDC_VP_OPTS, m_htiDropSourcesTimerHQR, m_iDropSourcesTimerHQR);
		DDV_MinMaxInt(pDX, iMaxRemoveQRS, 2500, 100000);
		//Ackronic END - Aggiunto da Aenarion[ITA] - Drop
//==> Chunk Selection Patch by Xman [lama]
	DDX_TreeRadio(pDX, IDC_VP_OPTS, m_htiEnableCSP, m_iEnableCSP);
//<== Chunk Selection Patch by Xman [lama]
}

/////////////////////////////////////////  OnInitDialog  ///////////////////////////////////////////////////////------

BOOL CPPgVipeR::OnInitDialog()
{

// IP-to-Country +
	m_iIP2CountryName = thePrefs.GetIP2CountryNameMode(); 
	m_bIP2CountryShowFlag = thePrefs.IsIP2CountryShowFlag();
	// IP-to-Country -

m_iCreditSystem = thePrefs.GetCreditSystem(); // CreditSystem 

//client percentage
m_bEnableClientPerc = thePrefs.enableClientPerc;
//Telp Start payback first
	m_bPBF = thePrefs.m_bPBF;
//Telp end payback first
// ==> Anti Uploader Ban - Stulle
	m_iAntiUploaderBanLimit = thePrefs.m_iAntiUploaderBanLimit;
	m_iAntiUploaderBanCase = thePrefs.GetAntiUploaderBanCase();
	// <== Anti Uploader Ban - Stulle
	//Telp Start push small file
	m_bEnablePushSmallFile = thePrefs.enablePushSmallFile; //Hawkstar, push small file
//Telp End push small file
//Telp Start push rare file
    m_bEnablePushRareFile = thePrefs.enablePushRareFile; //Hawkstar, push rare file
//Telp End push rare file
//Ackronic START - Aggiunto da Aenarion[ITA] - Drop
	m_iDropSourcesNNS = thePrefs.m_bDropSourcesNNS;
	m_iDropSourcesTimerNNS = thePrefs.m_iDropSourcesTimerNNS;
	m_iDropSourcesFQ = thePrefs.m_bDropSourcesFQ;
	m_iDropSourcesTimerFQ = thePrefs.m_iDropSourcesTimerFQ;
	m_iDropSourcesHQR = thePrefs.m_bDropSourcesHQR;
	m_iDropSourcesTimerHQR = thePrefs.m_iDropSourcesTimerHQR;
    iMaxRemoveQRS = (int) thePrefs.GetMaxRemoveQRS();
	//Ackronic END - Aggiunto da Aenarion[ITA] - Drop
//==> Chunk Selection Patch by Xman [lama]
	m_iEnableCSP	= thePrefs.m_iEnableCSP;
//<== Chunk Selection Patch by Xman [lama]
	
	CPropertyPage::OnInitDialog();
	//LoadSettings();
	InitWindowStyles(this);
	Localize();
	return TRUE;   // retourne TRUE  sauf si vous avez défini le focus sur un contrôle
}

/////////////////////////////////////  Localize  //////////////////////////////////////////////

void CPPgVipeR::Localize(void)
{
	CString temp;
	if (m_hWnd)
	{
		//Telp Start payback first
		if(m_htiPBF) m_ctrlTreeOptions.SetItemText(m_htiPBF, _T("Payback First"));
//Telp end payback first
//Telp Start push small file
        if (m_htiEnablePushSmallFile) m_ctrlTreeOptions.SetItemText(m_htiEnablePushSmallFile, _T("Push Small Files")); //Hawkstar, push small file
//Telp End push small file
//Telp Start push rare file
        if (m_htiEnablePushRareFile) m_ctrlTreeOptions.SetItemText(m_htiEnablePushRareFile, _T("Push Rare Files")); //Hawkstar, push rare file
//Telp End push rare file

//CreditSystem+
	 if (m_htiCreditSystem)  
			m_ctrlTreeOptions.SetItemText(m_htiCreditSystem, GetResString(IDS_CREDIT_SYSTEM));
		if (m_htiOfficialCredit)
			m_ctrlTreeOptions.SetItemText(m_htiOfficialCredit, GetResString(IDS_OFFICIAL_CREDIT));
		if (m_htiLovelaceCredit)
			m_ctrlTreeOptions.SetItemText(m_htiLovelaceCredit, GetResString(IDS_LOVELACE_CREDIT));
		if (m_htiPawcioCredit)  
			m_ctrlTreeOptions.SetItemText(m_htiPawcioCredit, GetResString(IDS_PAWCIO_CREDIT));
	    if (m_htiPeaceCredit)  
			m_ctrlTreeOptions.SetItemText(m_htiPeaceCredit, GetResString(IDS_PEACE_CREDIT));
        if (m_htiSivkaCredit) 
			m_ctrlTreeOptions.SetItemText(m_htiSivkaCredit, GetResString(IDS_SIVKA_CREDIT));
        if (m_htiRtCredit)    
			m_ctrlTreeOptions.SetItemText(m_htiRtCredit, GetResString(IDS_RT_CREDIT));
		if (m_htiSwatCredit) 
			m_ctrlTreeOptions.SetItemText(m_htiSwatCredit, GetResString(IDS_SWAT_CREDIT));
		if (m_htiESCredit) 
			m_ctrlTreeOptions.SetItemText(m_htiESCredit, GetResString(IDS_EASTSHARE_CREDIT));
		if (m_htiFineCredit) 
			m_ctrlTreeOptions.SetItemText(m_htiFineCredit, GetResString(IDS_FINE_CREDIT)); //Add by Spe64
        if (m_htiTK4Credit) 
			m_ctrlTreeOptions.SetItemText(m_htiTK4Credit, GetResString(IDS_TK4_CREDIT)); //Add by Spe64
		if (m_htiCreditsNone)
			m_ctrlTreeOptions.SetItemText(m_htiCreditsNone, GetResString(IDS_CREDITS_NONE));
		//CreditSystem -
		// ==> Anti Uploader Ban - Stulle
	    if (m_htiAntiUploaderBanLimit) m_ctrlTreeOptions.SetEditLabel(m_htiAntiUploaderBanLimit, GetResString(IDS_UNBAN_UPLOADER));
// <== Anti Uploader Ban - Stulle
//==> Chunk Selection Patch by Xman [lama]
		if (m_htiEnableCSP) m_ctrlTreeOptions.SetItemText(m_htiEnableCSP, GetResString(IDS_CSP_ENABLE));
		if (m_htiEnableCSPNormal) m_ctrlTreeOptions.SetItemText(m_htiEnableCSPNormal, GetResString(IDS_CSP_NORMAL));		
		if (m_htiEnableCSPXman) m_ctrlTreeOptions.SetItemText(m_htiEnableCSPXman, GetResString(IDS_CSP_XMAN));		
//<== Chunk Selection Patch by Xman [lama]

}
}

////////////////////////////////////// OnApply //////////////////////////////////////

BOOL CPPgVipeR::OnApply()
{
	thePrefs.enableClientPerc = m_bEnableClientPerc;

thePrefs.SetCreditSystem(m_iCreditSystem); // CreditSystem

// IP-to-Country +
	if(	(thePrefs.m_iIP2CountryNameMode != IP2CountryName_DISABLE || thePrefs.m_bIP2CountryShowFlag) !=
		((IP2CountryNameSelection)m_iIP2CountryName != IP2CountryName_DISABLE || m_bIP2CountryShowFlag)	){
		//check if need to load or unload DLL and ip table
		if((IP2CountryNameSelection)m_iIP2CountryName != IP2CountryName_DISABLE || m_bIP2CountryShowFlag){
			theApp.ip2country->Load();
		}
		else{
			theApp.ip2country->Unload();
		}
	}
	thePrefs.m_iIP2CountryNameMode = m_iIP2CountryName;
	thePrefs.m_bIP2CountryShowFlag = m_bIP2CountryShowFlag;
	theApp.ip2country->Refresh();//refresh passive windows
	// IP-to-Country -
//==> Chunk Selection Patch by Xman [shadow2004]
	thePrefs.m_iEnableCSP	  = m_iEnableCSP;
//<== Chunk Selection Patch by Xman [shadow2004]

// ==> Anti Uploader Ban - Stulle
thePrefs.m_iAntiUploaderBanLimit = m_iAntiUploaderBanLimit;
	if(thePrefs.AntiUploaderBanCaseMode != m_iAntiUploaderBanCase){
		thePrefs.AntiUploaderBanCaseMode = m_iAntiUploaderBanCase;
	}
	// <== Anti Uploader Ban - Stulle
	//Ackronic START - Aggiunto da Aenarion[ITA] - Drop
	if(m_iDropSourcesTimerNNS < 2)
		thePrefs.m_iDropSourcesTimerNNS = 2;
	else thePrefs.m_iDropSourcesTimerNNS = m_iDropSourcesTimerNNS;

	if(m_iDropSourcesTimerFQ < 2)
		thePrefs.m_iDropSourcesTimerFQ = 2;
	else thePrefs.m_iDropSourcesTimerFQ = m_iDropSourcesTimerFQ;

	if(m_iDropSourcesTimerHQR < 2)
		thePrefs.m_iDropSourcesTimerHQR = 2;
	else thePrefs.m_iDropSourcesTimerHQR = m_iDropSourcesTimerHQR;
	
	thePrefs.SetDropSourcesNNS(m_iDropSourcesNNS);
	thePrefs.SetDropSourcesFQ(m_iDropSourcesFQ);
	thePrefs.SetDropSourcesHQR(m_iDropSourcesHQR);
	thePrefs.SetMaxRemoveQRS(iMaxRemoveQRS ? iMaxRemoveQRS : 5000);
	//Ackronic START - Aggiunto da Aenarion[ITA] - Drop
//Telp start payback first
	thePrefs.m_bPBF = m_bPBF;
//Telp end payback first
//Telp Start push small file
    thePrefs.enablePushSmallFile = m_bEnablePushSmallFile; 
//Telp End push small file
//Telp Start push rare file
    thePrefs.enablePushRareFile = m_bEnablePushRareFile; 
//Telp End push rare file
	
	return CPropertyPage::OnApply();
}

///////////////////////////////  OnDestroy  ////////////////////////////////////////

void CPPgVipeR::OnDestroy()
{	
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;

	//Telp Start payback first
	m_htiPBF = NULL;
//Telp end payback first
// ==> Anti Uploader Ban - Stulle
	m_htiAntiUploaderBanLimit = NULL;
	m_htiAntiCase1 = NULL;
	m_htiAntiCase2 = NULL;
	m_htiAntiCase3 = NULL;
	// <== Anti Uploader Ban - Stulle

//Telp Start push small file
    m_htiEnablePushSmallFile = NULL; //Hawkstar, push small file
//Telp End push small file
//Telp Start push rare file
    m_htiEnablePushRareFile = NULL; //Hawkstar, push rare file
//Telp End push rare file

	m_htiUM = NULL;
	m_htiSFM = NULL;
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	m_htiHideOS = NULL;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	m_htiSelectiveShare = NULL;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	m_htiShareOnlyTheNeed = NULL; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	//Ackronic START - Aggiunto da Aenarion[ITA] - Drop
	m_htiDropSources = NULL;
	m_htiDropSourcesTimerNNS = NULL;
	m_htiDropSourcesTimerFQ = NULL;
	m_htiDropSourcesTimerHQR = NULL;
	m_htiDropSourcesNNS = NULL;
	m_htiDropSourcesFQ = NULL;
	m_htiDropSourcesHQR = NULL;
    m_htiHqrBox = NULL;
	//Ackronic END - Aggiunto da Aenarion[ITA] - Drop
//==> Chunk Selection Patch by Xman [lama]
	m_htiEnableCSP			 = NULL;
//<== Chunk Selection Patch by Xman [lama]

// IP-to-Country +
	m_htiIP2CountryName = NULL;
	m_htiIP2CountryName_DISABLE = NULL;
	m_htiIP2CountryName_SHORT = NULL;
	m_htiIP2CountryName_MID = NULL;
	m_htiIP2CountryName_LONG = NULL;
	m_htiIP2CountryShowFlag = NULL;
	// IP-to-Country -

	CPropertyPage::OnDestroy();
}

/////////////////////////////////  OnKillActive  ////////////////////////////////////////

BOOL CPPgVipeR::OnKillActive()
{
	
	
	
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();

}




LRESULT CPPgVipeR::OnTreeOptsCtrlNotify(WPARAM wParam,LPARAM lParam)
{
	if (wParam == IDC_VP_OPTS)
	{
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		SetModified();
	}
	return 0;
}


void CPPgVipeR::LoadSettings(void){

	if(m_hWnd)
	{
		CString strBuffer;

	}
}




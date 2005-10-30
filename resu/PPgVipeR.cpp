// PPgVipeR.cpp : implementation file
#include "stdafx.h"
#include "emule.h"
#include "PPgVipeR.h"
#include "emuledlg.h"
#include "serverWnd.h" 
#include "OtherFunctions.h"
#include "UserMsgs.h"
#include "Preferences.h"
#include "SharedFileList.h" // Morph: PowerShare	
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
//Telp + Menu VipeR
	m_AdditionalVipeR = NULL;
//Telp - Menu VipeR
	m_secu = NULL; 
m_htiClientPerc = NULL;
//Telp Start payback first
	m_htiPBF = NULL;
//Telp End payback first
	//Telp + Start Slot Focus
	m_htiSlotFocus = NULL;
//Telp - End Slot Focus

 //Telp Start push small file
    m_htiEnablePushSmallFile = NULL; 
//Telp End push small file  
//Telp Start push rare file
    m_htiEnablePushRareFile = NULL; 
//Telp End push rare file
//<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	m_iHideOS = 0;
	m_iSelectiveShare = 0;
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	
 //<<-- ADDED STORMIT -  Morph: PowerShared //
	m_htiUM = NULL;
	m_htiSFM = NULL;
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	m_htiHideOS = NULL;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	m_htiSelectiveShare = NULL;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	m_htiShareOnlyTheNeed = NULL; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	m_htiPowerShareLimit = NULL; //MORPH - Added by SiRoB, POWERSHARE Limit
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
 //<<-- ADDED STORMIT - START - Added by SiRoB, Avoid misusing of powersharing	
	m_htiPowershareMode = NULL;
	m_htiPowershareDisabled = NULL;
	m_htiPowershareActivated = NULL;
	m_htiPowershareAuto = NULL;
	m_htiPowershareLimited = NULL;
//<<-- ADDED STORMIT - Morph: PowerShare //
 //<<-- ADDED STORMIT -  Morph: PowerShared //
	m_htiSpreadbar = NULL;


// CreditSystem
	m_htiCreditSystem = NULL;
	m_htiOfficialCredit = NULL;
	m_htiLovelaceCredit = NULL;
	m_htiSwatCredit = NULL;
	m_htiRatioCredit = NULL;
	m_htiPawcioCredit = NULL;
	m_htiESCredit = NULL; 
        m_htiCreditsNone = NULL;
// CreditSystem
	
	// ==> FunnyNick Tag - Stulle
	m_htiFnTag = NULL;
	m_htiNoTag = NULL;
	m_htiShortTag = NULL;
	m_htiFullTag = NULL;
	m_htiCustomTag = NULL;
	m_htiFnCustomTag = NULL;
	m_htiFnTagAtEnd = NULL;
	// <== FunnyNick Tag - Stulle

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
		int iImgOpt = 8;

//Telp+ Menu VipeR
		int iImgAddTweaks = 8;
//Telp- Menu VipeR
int iImgSecu = 8; 
int iImgCS = 8; // Creditsystems
 //<<-- ADDED STORMIT -  Morph: PowerShared //
 //<<-- ADDED STORMIT -  Morph: PowerShared //
int iImgUM = 8; // default icon
		int iImgDrop = 8;//Ackronic - Aggiunto da Aenarion[ITA] - Drop
		int iImgPS = 8;
		int iImgSFM = 8;
		int iImgFunnyNick = 8;

CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
//Telp+ Menu VipeR
		iImgAddTweaks = piml->Add(CTempIconLoader(_T("TWEAK")));
//Telp- Menu VipeR
          iImgSecu = piml->Add(CTempIconLoader(_T("SECURITY")));
iImgUM = piml->Add(CTempIconLoader(_T("UPLOAD")));
 //<<-- ADDED STORMIT -  Morph: PowerShared //
			// Morph: PowerShare
			iImgPS = piml->Add(CTempIconLoader(_T("Kadcontactlist")));
			iImgSFM = piml->Add(CTempIconLoader(_T("Sharedfiles")));
			// <--- Morph: PowerShare
 //<<-- ADDED STORMIT -  Morph: PowerShared //
					iImgDrop = piml->Add(CTempIconLoader(_T("DROP")));//Ackronic - Aggiunto da Aenarion[ITA] - Drop
 			iImgFunnyNick = piml->Add(CTempIconLoader(_T("FUNNYNICK")));


//Telp+ Menu VipeR
		m_AdditionalVipeR = m_ctrlTreeOptions.InsertGroup(_T("Misc Functions "), iImgAddTweaks, TVI_ROOT);
//Telp- Menu VipeR 

		m_secu = m_ctrlTreeOptions.InsertGroup(_T("Anti-Upload Ban"), iImgSecu, TVI_ROOT);
        m_ctrlTreeOptions.SetItemState(m_secu, TVIS_BOLD, TVIS_BOLD);

		m_ctrlTreeOptions.Expand(m_secu, TVE_EXPAND);
		m_ctrlTreeOptions.SetItemState(m_AdditionalVipeR, TVIS_BOLD, TVIS_BOLD);

iImgCS = piml->Add(CTempIconLoader(_T("STATSCLIENTS"))); // Creditsystems

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
	// ==> FunnyNick Tag - Stulle
		m_htiFnTag = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_FN_TAG), iImgFunnyNick, TVI_ROOT);
		m_htiNoTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_TAG), m_htiFnTag, m_iFnTag == 0);
		m_htiShortTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHORT_TAG), m_htiFnTag, m_iFnTag == 1);
		m_htiFullTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_FULL_TAG), m_htiFnTag, m_iFnTag == 2);
		m_htiCustomTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CUSTOM_TAG),m_htiFnTag,m_iFnTag == 3);
		m_htiFnCustomTag = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SET_CUSTOM_TAG), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiCustomTag);
		m_ctrlTreeOptions.AddEditBox(m_htiFnCustomTag, RUNTIME_CLASS(CTreeOptionsEdit));
		m_htiFnTagAtEnd = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FN_TAG_AT_END), m_htiFnTag, m_bFnTagAtEnd);
		m_ctrlTreeOptions.Expand(m_htiCustomTag, TVE_EXPAND);
		m_ctrlTreeOptions.SetItemState(m_htiFnTag, TVIS_BOLD, TVIS_BOLD);

		// <== FunnyNick Tag - Stulle
	//Ackronic START - Aggiunto da Aenarion[ITA] - Drop
		m_htiDropSources = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DROPS), iImgDrop, TVI_ROOT);
        m_ctrlTreeOptions.SetItemState(m_htiDropSources, TVIS_BOLD, TVIS_BOLD);
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
		//Ackronic END - Aggiunto da Aenarion[ITA] - Drop
//<<-- ADDED STORMIT -  Morph: PowerShared //
		m_htiUM = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_UM), iImgUM, TVI_ROOT);
m_ctrlTreeOptions.SetItemState(m_htiUM, TVIS_BOLD, TVIS_BOLD);
		//<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
		m_htiSpreadbar = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SPREADBAR_DEFAULT_CHECKBOX), m_htiUM, m_iSpreadbar);
		m_htiHideOS = m_ctrlTreeOptions.InsertItem(GetResString(IDS_HIDEOVERSHARES), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiUM);
		m_ctrlTreeOptions.AddEditBox(m_htiHideOS, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiSelectiveShare = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SELECTIVESHARE), m_htiHideOS, m_iSelectiveShare);
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
//Telp Start Slot Focus
		m_htiSlotFocus = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SLOT_FOCUS),m_htiUM, m_bSlotFocus);
//Telp End Slot Focus

		m_htiSFM = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SFM), iImgSFM, TVI_ROOT);
		m_ctrlTreeOptions.SetItemState(m_htiSFM, TVIS_BOLD, TVIS_BOLD);
		m_htiShareOnlyTheNeed = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHAREONLYTHENEED), m_htiSFM, m_iShareOnlyTheNeed);
		m_htiPowershareMode = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_POWERSHARE), iImgPS, m_htiSFM);
		m_htiPowershareDisabled = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_DISABLED), m_htiPowershareMode, m_iPowershareMode == 0);
		m_htiPowershareActivated =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_ACTIVATED), m_htiPowershareMode, m_iPowershareMode == 1);
		m_htiPowershareAuto =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_AUTO), m_htiPowershareMode, m_iPowershareMode == 2);
		m_htiPowershareLimited =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_LIMITED), m_htiPowershareMode, m_iPowershareMode == 3);
		m_htiPowerShareLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_POWERSHARE_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPowershareLimited );
// <--- Morph: PowerShare

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
		m_htiCreditsNone = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CREDITS_NONE), m_htiCreditSystem, m_iCreditSystem == 10); //Spe64 change 
	
		// CreditSystem
}
		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;

 ///////////////////////////////////////////////////////////////////////////////////
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
// ==> FunnyNick Tag - Stulle
	DDX_TreeRadio(pDX, IDC_VP_OPTS, m_htiFnTag, (int &)m_iFnTag);
	DDX_TreeEdit(pDX, IDC_VP_OPTS, m_htiFnCustomTag, m_sFnCustomTag);
	DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiFnTagAtEnd, m_bFnTagAtEnd);
	// <== FunnyNick Tag - Stulle
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
//<<-- ADDED STORMIT -  Morph: PowerShared //
	DDX_TreeEdit(pDX, IDC_VP_OPTS, m_htiHideOS, m_iHideOS);
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	DDV_MinMaxInt(pDX, m_iHideOS, 0, INT_MAX);
	DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiSelectiveShare, m_iSelectiveShare);
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiShareOnlyTheNeed, m_iShareOnlyTheNeed);
	DDX_TreeEdit(pDX, IDC_VP_OPTS, m_htiPowerShareLimit, m_iPowerShareLimit);
	DDV_MinMaxInt(pDX, m_iShareOnlyTheNeed, 0, INT_MAX);
	DDX_TreeRadio(pDX, IDC_VP_OPTS, m_htiPowershareMode, m_iPowershareMode);
        // <--- Morph: PowerShare
//Telp + Start Slot Focus
	DDX_TreeCheck(pDX, IDC_VP_OPTS, m_htiSlotFocus, m_bSlotFocus);
//Telp - End Slot Focus

}

/////////////////////////////////////////  OnInitDialog  ///////////////////////////////////////////////////////------

BOOL CPPgVipeR::OnInitDialog()
{

m_iCreditSystem = thePrefs.GetCreditSystem(); // CreditSystem 

m_iSpreadbar = thePrefs.GetSpreadbarSetStatus(); //SpreadBar
//client percentage
m_bEnableClientPerc = thePrefs.enableClientPerc;
//client percentage

//Telp Start payback first
	m_bPBF = thePrefs.m_bPBF;
//Telp end payback first
//Telp + Start Slot Focus
	m_bSlotFocus = thePrefs.SlotFocus;
//Telp - End Slot Focus

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
//<<-- ADDED STORMIT -  Morph: PowerShared //
	m_iPowershareMode = thePrefs.m_iPowershareMode;//MORPH - Added by SiRoB, Avoid misusing of powersharing
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	m_iHideOS = thePrefs.hideOS; //MORPH - Added by SiRoB, SLUGFILLER: hideOS
	m_iSelectiveShare = thePrefs.selectiveShare; //MORPH - Added by SiRoB, SLUGFILLER: hideOS
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	m_iShareOnlyTheNeed = thePrefs.ShareOnlyTheNeed; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	m_iPowerShareLimit = thePrefs.PowerShareLimit; //MORPH - Added by SiRoB, POWERSHARE Limit
        // <--- Morph: PowerShare
 //<<-- ADDED STORMIT -  Morph: PowerShared //
	// ==> FunnyNick Tag - Stulle
	m_iFnTag = thePrefs.GetFnTag();
	m_sFnCustomTag = thePrefs.m_sFnCustomTag;
	m_bFnTagAtEnd = thePrefs.GetFnTagAtEnd();
	// <== FunnyNick Tag - Stulle

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
//Telp Start Slot Focus
		if (m_htiSlotFocus) m_ctrlTreeOptions.SetItemText(m_htiSlotFocus, _T( "Disable Slot Focus"));
//Telp End Slot Focus

//Telp Start push small file
        if (m_htiEnablePushSmallFile) m_ctrlTreeOptions.SetItemText(m_htiEnablePushSmallFile, _T("Push Small Files")); //Hawkstar, push small file
//Telp End push small file
//Telp Start push rare file
        if (m_htiEnablePushRareFile) m_ctrlTreeOptions.SetItemText(m_htiEnablePushRareFile, _T("Push Rare Files")); //Hawkstar, push rare file
//Telp End push rare file

//<<-- ADDED STORMIT -  Morph: PowerShared //
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
		if (m_htiHideOS) m_ctrlTreeOptions.SetEditLabel(m_htiHideOS, GetResString(IDS_HIDEOVERSHARES));//MORPH - Added by SiRoB, SLUGFILLER: hideOS
		if (m_htiSelectiveShare) m_ctrlTreeOptions.SetItemText(m_htiSelectiveShare, GetResString(IDS_SELECTIVESHARE));//MORPH - Added by SiRoB, SLUGFILLER: hideOS
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
		if (m_htiShareOnlyTheNeed) m_ctrlTreeOptions.SetItemText(m_htiShareOnlyTheNeed, GetResString(IDS_SHAREONLYTHENEED));//MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
		if (m_htiPowershareMode) m_ctrlTreeOptions.SetItemText(m_htiPowershareMode, GetResString(IDS_POWERSHARE));
		if (m_htiPowershareDisabled) m_ctrlTreeOptions.SetItemText(m_htiPowershareDisabled, GetResString(IDS_POWERSHARE_DISABLED));
		if (m_htiPowershareActivated) m_ctrlTreeOptions.SetItemText(m_htiPowershareActivated, GetResString(IDS_POWERSHARE_ACTIVATED));
		if (m_htiPowershareAuto) m_ctrlTreeOptions.SetItemText(m_htiPowershareAuto, GetResString(IDS_POWERSHARE_AUTO));
		if (m_htiPowershareLimited) m_ctrlTreeOptions.SetItemText(m_htiPowershareLimited, GetResString(IDS_POWERSHARE_LIMITED));
		if (m_htiPowerShareLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPowerShareLimit, GetResString(IDS_POWERSHARE_LIMIT));
                // <--- Morph: PowerShare
 //<<-- ADDED STORMIT -  Morph: PowerShared //	
	
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
		if (m_htiCreditsNone)
			m_ctrlTreeOptions.SetItemText(m_htiCreditsNone, GetResString(IDS_CREDITS_NONE));
		//CreditSystem -
		// ==> Anti Uploader Ban - Stulle
	    if (m_htiAntiUploaderBanLimit) m_ctrlTreeOptions.SetEditLabel(m_htiAntiUploaderBanLimit, GetResString(IDS_UNBAN_UPLOADER));
// <== Anti Uploader Ban - Stulle
	// ==> FunnyNick Tag - Stulle
		if (m_htiFnCustomTag) m_ctrlTreeOptions.SetEditLabel(m_htiFnCustomTag, GetResString(IDS_SET_CUSTOM_TAG));
		if (m_htiFnTagAtEnd) m_ctrlTreeOptions.SetItemText(m_htiFnTagAtEnd, GetResString(IDS_FN_TAG_AT_END));
		// <== FunnyNick Tag - Stulle
}
}

////////////////////////////////////// OnApply //////////////////////////////////////

BOOL CPPgVipeR::OnApply()
{
		thePrefs.enableClientPerc = m_bEnableClientPerc;//Client Percentage

thePrefs.SetCreditSystem(m_iCreditSystem); // CreditSystem

// ==> Anti Uploader Ban - Stulle
thePrefs.m_iAntiUploaderBanLimit = m_iAntiUploaderBanLimit;
		if(thePrefs.AntiUploaderBanCaseMode != m_iAntiUploaderBanCase)
		{
		thePrefs.AntiUploaderBanCaseMode = m_iAntiUploaderBanCase;
	}
	// <== Anti Uploader Ban - Stulle
// ==> FunnyNick Tag - Stulle
	if(thePrefs.FnTagMode != m_iFnTag){
		thePrefs.FnTagMode = m_iFnTag;
	}
	_stprintf (thePrefs.m_sFnCustomTag,_T("%s"), m_sFnCustomTag);
	thePrefs.m_bFnTagAtEnd = m_bFnTagAtEnd;
	// <== FunnyNick Tag - Stulle

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
	//<<-- ADDED STORMIT -  Morph: PowerShared //
	thePrefs.m_iPowershareMode = m_iPowershareMode;//MORPH - Added by SiRoB, Avoid misusing of powersharing
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	thePrefs.hideOS = m_iHideOS;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	thePrefs.selectiveShare = m_iSelectiveShare; //MORPH - Added by SiRoB, SLUGFILLER: hideOS
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	thePrefs.ShareOnlyTheNeed = m_iShareOnlyTheNeed; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	thePrefs.PowerShareLimit = m_iPowerShareLimit;
	theApp.sharedfiles->UpdatePartsInfo();
        // <--- Morph: PowerShare
 //<<-- ADDED STORMIT -  Morph: PowerShared //
//Telp Start Slot Focus
	thePrefs.SlotFocus = m_bSlotFocus;
//Telp End Slot Focus

//Telp start payback first
	thePrefs.m_bPBF = m_bPBF;
//Telp end payback first
//Telp Start Slot Focus
	thePrefs.SlotFocus = m_bSlotFocus;
//Telp End Slot Focus

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
	// ==> FunnyNick Tag - Stulle
	m_htiFnTag = NULL;
	m_htiNoTag = NULL;
	m_htiShortTag = NULL;
	m_htiFullTag = NULL;
	m_htiCustomTag = NULL;
	m_htiFnCustomTag = NULL;
	m_htiFnTagAtEnd = NULL;
	// <== FunnyNick Tag - Stulle
//Telp + Slot focus
	m_htiSlotFocus = NULL;
//Telp - End Slot Focus

//Telp Start push small file
    m_htiEnablePushSmallFile = NULL; //Hawkstar, push small file
//Telp End push small file
//Telp Start push rare file
    m_htiEnablePushRareFile = NULL; //Hawkstar, push rare file
//Telp End push rare file

//<<-- ADDED STORMIT -  Morph: PowerShared //
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

//MORPH - Added by SiRoB, POWERSHARE Limit
		m_htiPowerShareLimit = NULL; 
	m_htiPowershareMode = NULL;
	m_htiPowershareDisabled = NULL;
	m_htiPowershareActivated = NULL;
	m_htiPowershareAuto = NULL;
	m_htiPowershareLimited = NULL;
 //<<-- ADDED STORMIT -  Morph: PowerShared //	


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




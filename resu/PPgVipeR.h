#pragma once

#include "TreeOptionsCtrlEx.h"
#include "Preferences.h"

class CPPgVipeR : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgVipeR)

public:
	CPPgVipeR();
	virtual ~CPPgVipeR();
	void Localize(void);
	void LoadSettings();
	enum { IDD = IDD_PPG_VipeR}; 

protected:
    CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;
//Telp+ Menu VipeR
	HTREEITEM m_AdditionalVipeR; 
	HTREEITEM m_secu;
	//client percentage
	HTREEITEM m_htiClientPerc;
	bool m_bEnableClientPerc;
	//client percentage
//Telp- Menu VipeR
	//Ackronic START - Aggiunto da Aenarion[ITA] - Drop
	int iMaxRemoveQRS;
	int m_iDropSources;
	bool m_iDropSourcesNNS;
	int m_iDropSourcesTimerNNS;
	bool m_iDropSourcesFQ;
	int m_iDropSourcesTimerFQ;
	bool m_iDropSourcesHQR;
	int m_iDropSourcesTimerHQR;
	//Ackronic END - Aggiunto da Aenarion[ITA] - Drop
//CreditSyst
int m_iCreditSystem;
	HTREEITEM m_htiCreditSystem;
	HTREEITEM m_htiOfficialCredit;
	HTREEITEM m_htiLovelaceCredit;
	HTREEITEM m_htiPeaceCredit;
	HTREEITEM m_htiSivkaCredit;
	HTREEITEM m_htiRtCredit;
	HTREEITEM m_htiSwatCredit;
	HTREEITEM m_htiRatioCredit;
	HTREEITEM m_htiPawcioCredit;
	HTREEITEM m_htiESCredit;
HTREEITEM m_htiFineCredit; //Add by Spe64
HTREEITEM m_htiTK4Credit; //Add by Spe64
        HTREEITEM m_htiCreditsNone;
		//CreditSyst
//Telp start payback first
	int m_bPBF;
	HTREEITEM m_htiPBF;
//Telp end Payback first
// ==> Anti Uploader Ban - Stulle
	int m_iAntiUploaderBanLimit;
	int m_iAntiUploaderBanCase;
	// <== Anti Uploader Ban - Stulle
// ==> Anti Uploader Ban - Stulle
	HTREEITEM m_htiAntiUploaderBanLimit;
	HTREEITEM m_htiAntiCase1;
	HTREEITEM m_htiAntiCase2;
	HTREEITEM m_htiAntiCase3;
	// <== Anti Uploader Ban - Stulle
//<<-- ADDED STORMIT - PowerShare - //
	int m_iPowershareMode; //MORPH - Added by SiRoB, Avoid misusing of powersharing
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	int m_iHideOS;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	int m_iSelectiveShare;  //MORPH - Added by SiRoB, SLUGFILLER: hideOS
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	int m_iShareOnlyTheNeed; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	int m_iPowerShareLimit; //MORPH - Added by SiRoB, POWERSHARE Limit
        // <--- Morph: PowerShare
 //<<-- ADDED STORMIT - PowerShare - //
//Telp Start push small file
    int m_bEnablePushSmallFile;
	HTREEITEM m_htiEnablePushSmallFile;
//Telp End push small file
//Telp Start push rare file
    int m_bEnablePushRareFile;
	HTREEITEM m_htiEnablePushRareFile; 
//Telp End push rare file
 //<<-- ADDED STORMIT - PowerShare - //
	HTREEITEM m_htiUM;
	HTREEITEM m_htiSFM;
	HTREEITEM m_htiPowershareMode;
	HTREEITEM m_htiPowershareDisabled;
	HTREEITEM m_htiPowershareActivated;
	HTREEITEM m_htiPowershareAuto;
	HTREEITEM m_htiPowershareLimited;
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	HTREEITEM m_htiHideOS;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	HTREEITEM m_htiSelectiveShare; //MORPH - Added by SiRoB, SLUGFILLER: hideOS
 //<<-- ADDED STORMIT - SLUGFILLER: hideOS - //
	HTREEITEM m_htiShareOnlyTheNeed; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	HTREEITEM m_htiPowerShareLimit; //MORPH - Added by SiRoB, POWERSHARE Limit
        // <--- Morph: PowerSHare
	int	m_iSpreadbar;
	HTREEITEM m_htiSpreadbar;
// Upload Tweaks
	HTREEITEM m_htiUploadTweaks;
	HTREEITEM m_htiUpSlotsMin;
	HTREEITEM m_htiUpSlotsMax;

// [ionix] - FunnyNick
	bool m_bFunnyNickEnabled;
	int m_iFunnyNickTag;
	CString m_strFunnyNickTagCustom;
	// [ionix] - FunnyNick
	//Ackronic START - Aggiunto da Aenarion[ITA] - Drop
	HTREEITEM m_htiDropSources;
	HTREEITEM m_htiDropSourcesNNS;
	HTREEITEM m_htiDropSourcesTimerNNS;
	HTREEITEM m_htiDropSourcesFQ;
	HTREEITEM m_htiDropSourcesTimerFQ;
	HTREEITEM m_htiDropSourcesHQR;
	HTREEITEM m_htiDropSourcesTimerHQR;
	HTREEITEM m_htiHqrBox;
	//Ackronic END - Aggiunto da Aenarion[ITA] - Drop
// IP-to-Country +
	IP2CountryNameSelection	m_iIP2CountryName;
	HTREEITEM m_htiIP2CountryName;
	HTREEITEM m_htiIP2CountryName_DISABLE;
	HTREEITEM m_htiIP2CountryName_SHORT;
	HTREEITEM m_htiIP2CountryName_MID;
	HTREEITEM m_htiIP2CountryName_LONG;
	int m_bIP2CountryShowFlag;
	HTREEITEM m_htiIP2CountryShowFlag;
	// IP-to-Country -
// [ionix] - FunnyNick
	HTREEITEM m_htiFunnyNick;
	HTREEITEM m_htiFunnyNickEnabled;
	HTREEITEM m_htiFunnyNickTag;
	HTREEITEM m_htiFunnyNickTag_0;
	HTREEITEM m_htiFunnyNickTag_1;
	HTREEITEM m_htiFunnyNickTag_2;
	HTREEITEM m_htiFunnyNickTag_3;
	HTREEITEM m_htiFunnyNickTagCustom;
	// [ionix] - FunnyNick
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnTvnSelchangedVPOpts(NMHDR *pNMHDR, LRESULT *pResult);
	int VP;
};

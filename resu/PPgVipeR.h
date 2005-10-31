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
	// ==> FunnyNick Tag - Stulle
	int m_iFnTag;
	CString   m_sFnCustomTag;
	bool m_bFnTagAtEnd;
	// <== FunnyNick Tag - Stulle
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
        HTREEITEM m_htiCreditsNone;
		//CreditSyst
//Telp start payback first
	int m_bPBF;
	HTREEITEM m_htiPBF;
//Telp end Payback first
	// ==> FunnyNick Tag - Stulle
	HTREEITEM m_htiFnTag;
	HTREEITEM m_htiNoTag;
	HTREEITEM m_htiShortTag;
	HTREEITEM m_htiFullTag;
	HTREEITEM m_htiCustomTag;
	HTREEITEM m_htiFnCustomTag;
	HTREEITEM m_htiFnTagAtEnd;
	// <== FunnyNick Tag - Stulle
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
//Telp Start Slot Focus
	int m_SlotFocus;
	int m_bSlotFocus;
	HTREEITEM m_htiSlotFocus;
	//Telp End Slot Focus

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

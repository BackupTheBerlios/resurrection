#pragma once

#include "TreeOptionsCtrlEx.h"
#include "Preferences.h"

class CPPgNe : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgNe)

public:
	CPPgNe();
	virtual ~CPPgNe();


	// Dialog Data
	enum { IDD = IDD_PPG_NE };

    void Localize(void);
	void LoadSettings();

	CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	afx_msg void OnBnClickedModified() { SetModified();}// [Maella/sivka: -ReAsk SRCs after IP Change-]
	afx_msg void OnBnClickedQuickStart();
	afx_msg void OnDLColorChange();
	// NiceHash from iONiX
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
afx_msg void OnSettingsChange() { SetModified();} 
	uint8 m_iNiceHashWeight;
	// NiceHash 
	afx_msg void OnBnClickedAutoHL(); //WiZaRd - AutoHL
	virtual BOOL OnKillActive();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
DECLARE_MESSAGE_MAP()
	//sivka aggressive client handling
	afx_msg void OnBnClickedSivkaBan(); 
    //sivka aggressive client handling
	void OnBnClickedLeecherBox(); // [ionix] - LeecherBox
	afx_msg void OnDestroy();


public:
	afx_msg void OnBnClickedAutohlGroupbox();
	afx_msg void OnStnClickedNicehashRight();
	afx_msg void OnStnClickedNicehashStatic();
};


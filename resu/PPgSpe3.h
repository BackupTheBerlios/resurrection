#pragma once

// CPPgMorph dialog

class CPPgSpe3 : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgSpe3)

public:
	CPPgSpe3();
	virtual ~CPPgSpe3();

// Dialog Data
	enum { IDD = IDD_PPG_Spe3 };
protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	void Localize(void);
	void LoadSettings(void);
	afx_msg void OnSettingsChange() {SetModified();}
	afx_msg void OnBnClickedUpdatefakes();//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
	afx_msg void OnBnClickedResetfakes();
	afx_msg void OnBnClickedResetipcurl();
	afx_msg void OnBnClickedUpdateipcurl();// Commander - Added: IP2Country auto-updating
	afx_msg void OnBnClickedResetipfurl();
	//KTS+
	afx_msg void OnBnClickedUpdateipfurl();
	//KTS-
};

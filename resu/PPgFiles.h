#pragma once

class CPPgFiles : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgFiles)

public:
	CPPgFiles();
	virtual ~CPPgFiles();

// Dialog Data
	enum { IDD = IDD_PPG_FILES };

	void Localize(void);

protected:
	CListBox m_uncfolders;

	void LoadSettings(void);
	void OnSettingsChangeCat(uint8 index);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	//virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam); removed help [lama]

	DECLARE_MESSAGE_MAP()

	afx_msg void OnSetCleanupFilter();
	afx_msg void BrowseVideoplayer();
	afx_msg void OnSettingsChange();
	afx_msg void OnSettingsChangeCat1() {OnSettingsChangeCat(1);}
	afx_msg void OnSettingsChangeCat2()	{OnSettingsChangeCat(2);}
	//afx_msg void OnHelp(); removed help [lama]
	//afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo); removed help [lama]
};

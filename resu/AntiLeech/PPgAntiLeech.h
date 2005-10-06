#pragma once
#include "ResizableLib/ResizableDialog.h"

class CPPgAntiLeech : public CResizableDialog
{
	DECLARE_DYNAMIC(CPPgAntiLeech)

public:
	CPPgAntiLeech();									// standard constructor
	virtual ~CPPgAntiLeech();

	// Dialog Data
	enum { IDD = IDD_PPG_ANTILEECH };

	void Localize(void);

protected:	
	CListCtrl m_lAntiLeech;

	void LoadSettings(void);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSettingsChange()					{ PreSetModified(); }
	afx_msg void OnApplyBtn()						{ OnApply(); }
	afx_msg void OnLvnItemchangedAntileechList(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	bool	m_bChanged;
	bool	IsModified() const							{return m_bChanged;}
	void	PreSetModified(const bool& bChanged = true)	{m_bChanged = bChanged; /*SetModified(bChanged)*/;}	
//>>> [ionix] - ionixguide
protected:
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
//<<< [ionix] - ionixguide
};
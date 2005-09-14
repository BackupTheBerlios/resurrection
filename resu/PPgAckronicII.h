#pragma once

class CPPgAckronicII : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgAckronicII)

public:
	CPPgAckronicII();
	virtual ~CPPgAckronicII();
	void Localize(void);
	// Dialog Data
	enum { IDD = IDD_PPG_ACKRONICII };

	

protected:
	bool	EnableCountryFlag;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnApply(void);
	void LoadSettings(void);
	BOOL OnInitDialog(void);

DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ SetModified(); }
	int m_iBufferTimeLimit;   //FrankyFive: Buffer Time Limit
	UINT m_iFileBufferSize;//BufferSize
	uint32 UpDataratePerClient;
	
	CSliderCtrl m_ctlBufferTimeLimit; //FrankyFive: Buffer Time Limit
	CSliderCtrl m_ctlFileBuffSize;//BufferSize
	CSliderCtrl m_ctlUpDataratePerClient;
	//DkD [sivka -Upload Slots and Upload Datarate Per Client-]
	//void ShowUpDRUpSlotsValues();
	//end Up Slots
	
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedChkBandierine();//Ackronic - Aggiunto da Aenarion[ITA] - IP to country
	
};

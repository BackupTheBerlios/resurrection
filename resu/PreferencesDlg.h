#pragma once
#include "PPgGeneral.h"
#include "PPgConnection.h"
#include "PPgServer.h"
#include "PPgDirectories.h"
#include "PPgFiles.h"
#include "PPgStats.h"
#include "PPgNotify.h"
#include "PPgTweaks.h"
#include "PPgDisplay.h"
#include "PPgSecurity.h"
#include "PPgWebServer.h"
#include "PPgProxy.h"
#include "PPgAckronicII.h"//Ackronic - Aggiunto da Aenarion[ITA] - 2° finestra Ack
#include "PPgVipeR.h" //Spe64 Pref 1
#include "PPGNe.h" //Spe64 Pref 2
#include "PPGSpe3.h"  //Spe64 Pref3
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
#include "PPgDebug.h"
#endif
#include "otherfunctions.h"
#include "TreePropSheet.h"
#include "KCSideBannerWnd.h" 	// [TPT] - New Preferences Banner	
//KTS+ webcache
#include "WebCache\PPgWebcachesettings.h" 
//KTS- webcache

class CPreferencesDlg : public CTreePropSheet
{
	DECLARE_DYNAMIC(CPreferencesDlg)

public:
	CPreferencesDlg();
	virtual ~CPreferencesDlg();
	
	CPPgGeneral		m_wndGeneral;
	CPPgConnection	m_wndConnection;
	CPPgServer		m_wndServer;
	CPPgDirectories	m_wndDirectories;
	CPPgFiles		m_wndFiles;
	CPPgStats		m_wndStats;
	CPPgNotify		m_wndNotify;
	CPPgTweaks		m_wndTweaks;
	CPPgDisplay		m_wndDisplay;
	CPPgSecurity	m_wndSecurity;
	CPPgWebServer	m_wndWebServer;
	CPPgProxy		m_wndProxy;
	CPPgAckronicII	m_wndAckronicII;//Ackronic - Aggiunto da Aenarion[ITA] - 2° finestra Ack
 CPPgVipeR	    m_wndVipeR;  //Spe64 Pref1
    CPPgNe			m_wndNe;  //Spe64 Pref2
CPPgSpe3	    m_wndSpe3; //Spe64 Pref 3
	//KTS+ webcache
	CPPgWebcachesettings	m_wndWebcachesettings; 
	//KTS- webcache
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CPPgDebug		m_wndDebug;
#endif

	void Localize();
	void SetStartPage(UINT uStartPageID);

protected:
	LPCTSTR m_pPshStartPage;
	bool m_bSaveIniFile;

	virtual BOOL OnInitDialog();
	//virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam); removed help [lama]

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	//afx_msg void OnHelp(); removed help [lama]
	//afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo); removed help [lama]
protected:
	CKCSideBannerWnd m_banner;	// [TPT] - New Preferences Banner
};

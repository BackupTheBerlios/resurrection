//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif
#include "resource.h"
//Start Optimizer Spe64
#include ".\Optimizer\cpu_info.h" 
//End Optimizer Spe64

#define	DEFAULT_NICK		thePrefs.GetHomepageBaseURL()
#define	DEFAULT_TCP_PORT	4662
#define	DEFAULT_UDP_PORT	(DEFAULT_TCP_PORT+10)

#define PORTTESTURL			_T("http://porttest.emule-project.net/connectiontest.php?tcpport=%i&udpport=%i&lang=%i")

class CSearchList;
class CUploadQueue;
class CListenSocket;
class CDownloadQueue;
class UploadBandwidthThrottler;
class LastCommonRouteFinder;
class CemuleDlg;
class CClientList;
class CKnownFileList;
class CServerConnect;
class CServerList;
class CSharedFileList;
class CClientCreditsList;
class CFriendList;
class CClientUDPSocket;
class CIPFilter;
class CWebServer;
class CMMServer;
class CAbstractFile;
class CUpDownClient;
class CPeerCacheFinder;
class CFirewallOpener;
class CFakecheck; //MORPH - Added by milobac, FakeCheck, FakeReport, Auto-updating
//KTS+// sicks ajout [lama]
class CSystemInfo;  
//KTS-// sicks ajout [lama]
// IP-to-Country +
class CIP2Country; 
// IP-to-Country -
struct SLogItem;

enum AppState{
	APP_STATE_RUNNING=0,
   	APP_STATE_SHUTINGDOWN,
	APP_STATE_DONE
};

class CemuleApp : public CWinApp
{
public:
	CemuleApp(LPCTSTR lpszAppName = NULL);

	// ZZ:UploadSpeedSense -->
    UploadBandwidthThrottler* uploadBandwidthThrottler;
    LastCommonRouteFinder* lastCommonRouteFinder;
	// ZZ:UploadSpeedSense <--

	//Start Optimizer Spe64
	CPUInfo 			cpu; 
	//End Optimizer Spe64

	// IP-to-Country +
	CIP2Country*		ip2country; 
	// IP-to-Country -	
	CemuleDlg*			emuledlg;
	CClientList*		clientlist;
	CKnownFileList*		knownfiles;
	CServerConnect*		serverconnect;
	CServerList*		serverlist;	
	CSharedFileList*	sharedfiles;
	CSearchList*		searchlist;
	CListenSocket*		listensocket;
	CUploadQueue*		uploadqueue;
	CDownloadQueue*		downloadqueue;
	CClientCreditsList*	clientcredits;
	CFriendList*		friendlist;
	CClientUDPSocket*	clientudp;
	CIPFilter*			ipfilter;
	CWebServer*			webserver;
	CMMServer*			mmserver;
	CPeerCacheFinder*	m_pPeerCache;
	CFirewallOpener*	m_pFirewallOpener;
CFakecheck*			FakeCheck; //MORPH - Added by milobac, FakeCheck, FakeReport, Auto-updating
//KTS+// sicks ajout [lama]
	CSystemInfo*		sysinfo; 
	//KTS-// sicks ajout [lama]
	HANDLE				m_hMutexOneInstance;
	int					m_iDfltImageListColorFlags;
	CFont				m_fontHyperText;
	CFont				m_fontDefaultBold;
	CFont				m_fontSymbol;
	CFont				m_fontLog;
	CBrush				m_brushBackwardDiagonal;
	static const UINT	m_nVersionMjr;
	static const UINT	m_nVersionMin;
	static const UINT	m_nVersionUpd;
	static const UINT	m_nVersionBld;
	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
	CString				m_strCurVersionLong;
	UINT				m_uCurVersionShort;
	UINT				m_uCurVersionCheck;
	ULONGLONG			m_ullComCtrlVer;
	AppState			m_app_state; // defines application state for shutdown 
    BOOL            m_reset_app; // [iONiX] - WiZaRd - Multiple Instances added by lama
	CMutex				hashing_mut;
	CString*			pstrPendingLink;
	COPYDATASTRUCT		sendstruct;

// Implementierung
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	// ed2k link functions
	void		AddEd2kLinksToDownload(CString strLinks, uint8 cat);
	void		SearchClipboard();
	void		IgnoreClipboardLinks(CString strLinks) {m_strLastClipboardContents = strLinks;}
	void		PasteClipboard(uint8 uCategory = 0);
	bool		IsEd2kFileLinkInClipboard();
	bool		IsEd2kServerLinkInClipboard();
	bool		IsEd2kLinkInClipboard(LPCSTR pszLinkType, int iLinkTypeLen);
	LPCTSTR		GetProfileFile()		{ return m_pszProfileName; }

	CString		CreateED2kSourceLink(const CAbstractFile* f);
//	CString		CreateED2kHostnameSourceLink(const CAbstractFile* f);
	CString		CreateKadSourceLink(const CAbstractFile* f);

	// clipboard (text)
	bool		CopyTextToClipboard(CString strText);
	CString		CopyTextFromClipboard();

	void		OnlineSig();
	void		UpdateReceivedBytes(uint32 bytesToAdd);
	void		UpdateSentBytes(uint32 bytesToAdd, bool sentToFriend = false);
	int			GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength = -1);
	HIMAGELIST	GetSystemImageList() { return m_hSystemImageList; }
	CSize		GetSmallSytemIconSize() { return m_sizSmallSystemIcon; }
	void		CreateBackwardDiagonalBrush();
	void		CreateAllFonts();
	bool		IsPortchangeAllowed();
	bool		IsConnected();
	bool		IsFirewalled();
	bool		DoCallback( CUpDownClient *client );
	uint32		GetID();
	uint32		GetPublicIP() const;	// return current (valid) public IP or 0 if unknown
	void		SetPublicIP(const uint32 dwIP);

	// because nearly all icons we are loading are 16x16, the default size is specified as 16 and not as 32 nor LR_DEFAULTSIZE
	HICON		LoadIcon(LPCTSTR lpszResourceName, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR) const;
	HICON		LoadIcon(UINT nIDResource) const;
	HBITMAP		LoadImage(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const;
	HBITMAP		LoadImage(UINT nIDResource, LPCTSTR pszResourceType) const;
	bool		LoadSkinColor(LPCTSTR pszKey, COLORREF& crColor) const;
	bool		LoadSkinColorAlt(LPCTSTR pszKey, LPCTSTR pszAlternateKey, COLORREF& crColor) const;
	CString		GetSkinFileItem(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const;
	void		ApplySkin(LPCTSTR pszSkinProfile);
	void		EnableRTLWindowsLayout();
	void		DisableRTLWindowsLayout();
	void		UpdateDesktopColorDepth();

	bool		GetLangHelpFilePath(CString& strResult);
	void		SetHelpFilePath(LPCTSTR pszHelpFilePath);
	void		ShowHelp(UINT uTopic, UINT uCmd = HELP_CONTEXT);
	bool		ShowWebHelp();

    // Elandal:ThreadSafeLogging -->
    // thread safe log calls
    void			QueueDebugLogLine(bool bAddToStatusBar, LPCTSTR line,...);
    void			QueueDebugLogLineEx(UINT uFlags, LPCTSTR line,...);
    void			HandleDebugLogQueue();
    void			ClearDebugLogQueue(bool bDebugPendingMsgs = false);

	void			QueueLogLine(bool bAddToStatusBar, LPCTSTR line,...);
    void			QueueLogLineEx(UINT uFlags, LPCTSTR line,...);
    void			HandleLogQueue();
    void			ClearLogQueue(bool bDebugPendingMsgs = false);
    // Elandal:ThreadSafeLogging <--

	bool			DidWeAutoStart() { return m_bAutoStart; }

protected:
	bool ProcessCommandline();
	void SetTimeOnTransfer();
    #define MAGIC      0xafecffbd // <<< [iONiX] - WiZaRd - Multiple Instances added by lama
	static BOOL CALLBACK SearchEmuleWindow(HWND hWnd, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHelp();

	HIMAGELIST m_hSystemImageList;
	CMapStringToPtr m_aExtToSysImgIdx;
	CSize m_sizSmallSystemIcon;

	bool		m_bGuardClipboardPrompt;
	CString		m_strLastClipboardContents;

    // Elandal:ThreadSafeLogging -->
    // thread safe log calls
    CCriticalSection m_queueLock;
    CTypedPtrList<CPtrList, SLogItem*> m_QueueDebugLog;
    CTypedPtrList<CPtrList, SLogItem*> m_QueueLog;
    // Elandal:ThreadSafeLogging <--

	uint32 m_dwPublicIP;
	bool m_bAutoStart;
	//Spe64 eWombat [WINSOCK2]
	public: 
	WSADATA				m_wsaData; 
	//Spe64 added eWombat [WINSOCK2]

	//Start Optimizer Spe64
	void OptimizerInfo(void); 
	//End Optimizer Spe64
};

extern CemuleApp theApp;


//////////////////////////////////////////////////////////////////////////////
// CTempIconLoader

class CTempIconLoader
{
public:
	// because nearly all icons we are loading are 16x16, the default size is specified as 16 and not as 32 nor LR_DEFAULTSIZE
	CTempIconLoader(LPCTSTR pszResourceID, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR);
	CTempIconLoader(UINT uResourceID, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR);
	~CTempIconLoader();

	operator HICON() const{
		return this == NULL ? NULL : m_hIcon;
	}

protected:
	HICON m_hIcon;
};

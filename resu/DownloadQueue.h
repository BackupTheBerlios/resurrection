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

class CSafeMemFile;
class CSearchFile;
class CUpDownClient;
class CServer;
class CPartFile;
class CSharedFileList;
class CKnownFile;
struct SUnresolvedHostname;

namespace Kademlia 
{
	class CUInt128;
};

class CSourceHostnameResolveWnd : public CWnd
{
// Construction
public:
	CSourceHostnameResolveWnd();
	virtual ~CSourceHostnameResolveWnd();

	void AddToResolve(const uchar* fileid, LPCSTR pszHostname, uint16 port, LPCTSTR pszURL = NULL);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnHostnameResolved(WPARAM wParam, LPARAM lParam);

private:
	struct Hostname_Entry {
		uchar fileid[16];
		CStringA strHostname;
		uint16 port;
		CString strURL;
	};
	CTypedPtrList<CPtrList, Hostname_Entry*> m_toresolve;
	char m_aucHostnameBuffer[MAXGETHOSTSTRUCT];
};


class CDownloadQueue
{
	friend class CAddFileThread;
	friend class CServerSocket;
    friend class CPartFile; // [TPT] - quick start
public:
	CDownloadQueue();
	~CDownloadQueue();

	void	Process();
	void	Init();
		//KTS+// sicks ajout [lama]
	double	cpuse;  
    double	memuse; 
	//KTS-// sicks ajout [lama]
	// add/remove entries
	void	AddPartFilesToShare();
	void	AddDownload(CPartFile* newfile, bool paused);
	void	AddSearchToDownload(CSearchFile* toadd,uint8 paused=2,uint8 cat=0);
	void	AddSearchToDownload(CString link,uint8 paused=2, uint8 cat=0);
	void	AddFileLinkToDownload(class CED2KFileLink* pLink,uint8 cat=0);
	void	RemoveFile(CPartFile* toremove);
	void	DeleteAll();

	int		GetFileCount() const { return filelist.GetCount(); }
	UINT	GetDownloadingFileCount() const;
	UINT	GetPausedFileCount() const;

	bool	IsFileExisting(const uchar* fileid, bool bLogWarnings = true) const;
	bool	IsPartFile(const CKnownFile* file) const;
	bool	IsTempFile(const CString& rstrDirectory, const CString& rstrName) const;	// SLUGFILLER: SafeHash

	CPartFile* GetFileByID(const uchar* filehash) const;
	CPartFile* GetFileByIndex(int index) const;
	CPartFile* GetFileByKadFileSearchID(uint32 ID) const;

    void    StartNextFileIfPrefs(int cat);
	void	StartNextFile(int cat=-1,bool force=false);

	// sources
	CUpDownClient* GetDownloadClientByIP(uint32 dwIP);
	CUpDownClient* GetDownloadClientByIP_UDP(uint32 dwIP, uint16 nUDPPort);
	bool	IsInList(const CUpDownClient* client) const;

	bool    CheckAndAddSource(CPartFile* sender,CUpDownClient* source);
	bool    CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source, bool bIgnoreGlobDeadList = false);
	bool	RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate = true);

	// statistics
	typedef struct{
		int	a[23];
	} SDownloadStats;
	void	GetDownloadStats(SDownloadStats& results);
	void	GetDownloadStats(int results[],uint64& pui64TotFileSize,uint64& pui64TotBytesLeftToTransfer,uint64& pui64TotNeededSpace);
	uint32	GetDatarate() {return datarate;}

	void	AddUDPFileReasks()								{m_nUDPFileReasks++;}
	uint32	GetUDPFileReasks() const						{return m_nUDPFileReasks;}
	void	AddFailedUDPFileReasks()						{m_nFailedUDPFileReasks++;}
	uint32	GetFailedUDPFileReasks() const					{return m_nFailedUDPFileReasks;}

	// categories
	void	ResetCatParts(int cat);
	void	SetCatPrio(int cat, uint8 newprio);
    void    RemoveAutoPrioInCat(int cat, uint8 newprio); // ZZ:DownloadManager
	void	SetCatStatus(int cat, int newstatus);
	void	MoveCat(uint8 from, uint8 to);
	void	SetAutoCat(CPartFile* newfile);

	// searching on local server
	void	SendLocalSrcRequest(CPartFile* sender);
	void	RemoveLocalServerRequest(CPartFile* pFile);
	void	ResetLocalServerRequests();

	// searching in Kad
	void	SetLastKademliaFileRequest()				{lastkademliafilerequest = ::GetTickCount();}
	bool	DoKademliaFileRequest();
	void	KademliaSearchFile(uint32 searchID, const Kademlia::CUInt128* pcontactID, const Kademlia::CUInt128* pkadID, uint8 type, uint32 ip, uint16 tcp, uint16 udp, uint32 serverip, uint16 serverport, uint32 clientid);

	// searching on global servers
	void	StopUDPRequests();

	// check diskspace
	void	SortByPriority();
	void	CheckDiskspace(bool bNotEnoughSpaceLeft = false);
	void	CheckDiskspaceTimed();

	void	ExportPartMetFilesOverview() const;
	void	OnConnectionState(bool bConnected);

	void	AddToResolved( CPartFile* pFile, SUnresolvedHostname* pUH );

	CString GetOptimalTempDir(uint8 nCat, uint32 nFileSize);

	CServer* cur_udpserver;
	//KTS+ webcache
	bool	ContainsUnstoppedFiles(); //jp webcache release
	//KTS- webcache

 //Sivka: AutoHL added by lama
	void InitTempVariables(CPartFile* file);
	void UpdateFileSettings(CPartFile* file);
        //Sivka: AutoHL added by lama
	void	DoQuickStart(); // [TPT] - changed by WiZaRd - QuickStart added by lama
protected:
	bool	SendNextUDPPacket();
	void	ProcessLocalRequests();
	int		GetMaxFilesPerUDPServerPacket() const;
	bool	SendGlobGetSourcesUDPPacket(CSafeMemFile* data, bool bExt2Packet);

private:
	bool	CompareParts(POSITION pos1, POSITION pos2);
	void	SwapParts(POSITION pos1, POSITION pos2);
	void	HeapSort(uint16 first, uint16 last);
	CTypedPtrList<CPtrList, CPartFile*> filelist;
	CTypedPtrList<CPtrList, CPartFile*> m_localServerReqQueue;
	uint16	filesrdy;
	uint32	datarate;
	
	CPartFile*	lastfile;
	uint32		lastcheckdiskspacetime;
	uint32		lastudpsearchtime;
	uint32		lastudpstattime;
	uint8		udcounter;
	uint8		m_cRequestsSentToServer;
	uint32		m_dwNextTCPSrcReq;
	int			m_iSearchedServers;
	uint32		lastkademliafilerequest;

	uint64		m_datarateMS;
	uint32		m_nUDPFileReasks;
	uint32		m_nFailedUDPFileReasks;

	// By BadWolf - Accurate Speed Measurement
	typedef struct TransferredData {
		uint32	datalen;
		DWORD	timestamp;
	};
	CList<TransferredData> avarage_dr_list;
	// END By BadWolf - Accurate Speed Measurement

	CSourceHostnameResolveWnd m_srcwnd;

    DWORD       m_dwLastA4AFtime; // ZZ:DownloadManager
//>>> WiZaRd - AutoHL added by lama
	private:
	uint32	m_uiHLCount;
	// AutoHL [Aireoreion]
	bool	m_bIsInit;
public:
	uint32  GetHLCount()	{return m_uiHLCount;}
	// AutoHL [Aireoreion]
	bool	IsInit()		{return m_bIsInit == true;}
//<<< WiZaRd - AutoHL added by lama
};
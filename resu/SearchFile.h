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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
#include "AbstractFile.h"

class CFileDataIO;
class CxImage;

class CSearchFile : public CAbstractFile
{
	DECLARE_DYNAMIC(CSearchFile)

	friend class CPartFile;
	friend class CSearchListCtrl;
public:
	CSearchFile(CFileDataIO* in_data, bool bOptUTF8, uint32 nSearchID,
				uint32 nServerIP=0, uint16 nServerPort=0,
				LPCTSTR pszDirectory = NULL, 
				bool nKademlia = false);
	CSearchFile(const CSearchFile* copyfrom);
	CSearchFile(uint32 nSearchID, const uchar* pucFileHash, uint32 uFileSize, LPCTSTR pszFileName, int iFileType, int iAvailability);
	virtual ~CSearchFile();

	bool	IsKademlia() const { return m_nKademlia; }
	uint32	AddSources(uint32 count);
	uint32	GetSourceCount() const;
	uint32	AddCompleteSources(uint32 count);
	uint32	GetCompleteSourceCount() const;
	int		IsComplete() const;
	int		IsComplete(UINT uSources, UINT uCompleteSources) const;
	time_t	GetLastSeenComplete() const;
    LPCTSTR	GetFakeComment() const { return m_pszIsFake; } //MORPH - Added by SiRoB, FakeCheck, FakeReport, Auto-updating
	uint32	GetSearchID() const { return m_nSearchID; }
	LPCTSTR GetDirectory() const { return m_pszDirectory; }

	uint32	GetClientID() const				{ return m_nClientID; }
	void	SetClientID(uint32 nClientID)	{ m_nClientID = nClientID; }
	uint16	GetClientPort() const			{ return m_nClientPort; }
	void	SetClientPort(uint16 nPort)		{ m_nClientPort = nPort; }
	uint32	GetClientServerIP() const		{ return m_nClientServerIP; }
	void	SetClientServerIP(uint32 uIP)   { m_nClientServerIP = uIP; }
	uint16	GetClientServerPort() const		{ return m_nClientServerPort; }
	void	SetClientServerPort(uint16 nPort) { m_nClientServerPort = nPort; }
	int		GetClientsCount() const			{ return ((GetClientID() && GetClientPort()) ? 1 : 0) + m_aClients.GetSize(); }

	virtual void	UpdateFileRatingCommentAvail();

	// GUI helpers
	CSearchFile* GetListParent() const		{ return m_list_parent; }
	void		 SetListParent(CSearchFile* parent) { m_list_parent = parent; }
	uint16		 GetListChildCount() const	{ return m_list_childcount; }
	void		 SetListChildCount(int cnt)	{ m_list_childcount = cnt; }
	void		 AddListChildCount(int cnt) { m_list_childcount += cnt; }
	bool		 IsListExpanded() const		{ return m_list_bExpanded; }
	void		 SetListExpanded(bool val)	{ m_list_bExpanded = val; }

	struct SClient {
		SClient() {
			m_nIP = m_nPort = m_nServerIP = m_nServerPort = 0;
		}
		SClient(uint32 nIP, UINT nPort, uint32 nServerIP, UINT nServerPort) {
			m_nIP = nIP;
			m_nPort = nPort;
			m_nServerIP = nServerIP;
			m_nServerPort = nServerPort;
		}
		uint32 m_nIP;
		uint32 m_nServerIP;
		uint16 m_nPort;
		uint16 m_nServerPort;
	};
	void AddClient(const SClient& client) { m_aClients.Add(client); }
	const CSimpleArray<SClient>& GetClients() const { return m_aClients; }

	struct SServer {
		SServer() {
			m_nIP = m_nPort = 0;
			m_uAvail = 0;
		}
		SServer(uint32 nIP, UINT nPort) {
			m_nIP = nIP;
			m_nPort = nPort;
			m_uAvail = 0;
		}
		uint32 m_nIP;
		uint16 m_nPort;
		UINT   m_uAvail;
	};
	void AddServer(const SServer& server) { m_aServers.Add(server); }
	const CSimpleArray<SServer>& GetServers() const { return m_aServers; }
	SServer& GetServerAt(int iServer) { return m_aServers[iServer]; }
	
	void	AddPreviewImg(CxImage* img)	{	m_listImages.Add(img); }
	const CSimpleArray<CxImage*>& GetPreviews() const { return m_listImages; }
	bool	IsPreviewPossible() const { return m_bPreviewPossible;}
	void	SetPreviewPossible(bool in)	{ m_bPreviewPossible = in; }

	enum EKnownType
	{
		NotDetermined,
		Shared,
		Downloading,
		Downloaded,
		Cancelled,
		Unknown
	};

	EKnownType GetKnownType() const { return m_eKnown; }
	void SetKnownType(EKnownType eType) { m_eKnown = eType; }

private:
	bool	m_nKademlia;
	uint32	m_nClientID;
	uint16	m_nClientPort;
	uint32	m_nSearchID;
	uint32	m_nClientServerIP;
	uint16	m_nClientServerPort;
	CSimpleArray<SClient> m_aClients;
	CSimpleArray<SServer> m_aServers;
	CSimpleArray<CxImage*> m_listImages;
	LPTSTR m_pszDirectory;
    LPTSTR m_pszIsFake; //MORPH - Added by SiRoB, FakeCheck, FakeReport, Auto-updating



	// GUI helpers
	bool		m_bPreviewPossible;
	bool		m_list_bExpanded;
	uint16		m_list_childcount;
	CSearchFile*m_list_parent;
	EKnownType	m_eKnown;
};

bool IsValidSearchResultClientIPPort(uint32 nIP, uint16 nPort);
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
#include "BarShader.h"
#include "StatisticFile.h"
#include "AbstractFile.h"
#include <list>

class CxImage;
class CUpDownClient;
class Packet;
class CFileDataIO;
class CAICHHashTree;
class CAICHHashSet;
class CCollection;
class CSafeMemFile;//Ackronic - Aggiunto da Aenarion[ITA] - PowerRelease

typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;
class CKnownFile : public CAbstractFile
{
	DECLARE_DYNAMIC(CKnownFile)

public:
	CKnownFile();
	virtual ~CKnownFile();

//Telp Start push rare file
	float	GetFileRatio(void) ;
//Telp End push rare file	
	//KTS+ webcache
	bool ReleaseViaWebCache; //JP webcache release
	uint32 GetNumberOfClientsRequestingThisFileUsingThisWebcache(CString webcachename, uint32 maxCount); //JP webcache release
	void SetReleaseViaWebCache(bool WCRelease) {ReleaseViaWebCache=WCRelease;} //JP webcache release
	//KTS- webcache
	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	const CString& GetPath() const { return m_strDirectory; }
	void SetPath(LPCTSTR path);

	const CString& GetFilePath() const { return m_strFilePath; }
	void SetFilePath(LPCTSTR pszFilePath);

	virtual bool CreateFromFile(LPCTSTR directory, LPCTSTR filename, LPVOID pvProgressParam); // create date, hashset and tags from a file
	virtual bool LoadFromFile(CFileDataIO* file);	//load date, hashset and tags from a .met file
	bool	WriteToFile(CFileDataIO* file);
	bool	CreateAICHHashSetOnly();

	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	CTime	GetUtcCFileDate() const { return CTime(m_tUtcLastModified); }
	uint32	GetUtcFileDate() const { return m_tUtcLastModified; }

	virtual void SetFileSize(uint32 nFileSize);

	// local available part hashs
	uint16	GetHashCount() const { return hashlist.GetCount(); }
	uchar*	GetPartHash(uint16 part) const;
	const CArray<uchar*, uchar*>& GetHashset() const { return hashlist; }
	bool	SetHashset(const CArray<uchar*, uchar*>& aHashset);

	// nr. of part hashs according the file size wrt ED2K protocol
	UINT	GetED2KPartHashCount() const { return m_iED2KPartHashCount; }

	// nr. of 9MB parts (file data)
	__inline uint16 GetPartCount() const { return m_iPartCount; }

	// nr. of 9MB parts according the file size wrt ED2K protocol (OP_FILESTATUS)
	__inline uint16 GetED2KPartCount() const { return m_iED2KPartCount; }

	// file upload priority
	uint8	GetUpPriority(void) const { return m_iUpPriority; }
	void	SetUpPriority(uint8 iNewUpPriority, bool bSave = true);
	bool	IsAutoUpPriority(void) const { return m_bAutoUpPriority; }
	void	SetAutoUpPriority(bool NewAutoUpPriority) { m_bAutoUpPriority = NewAutoUpPriority; }
	void	UpdateAutoUpPriority();
	// This has lost it's meaning here.. This is the total clients we know that want this file..
	// Right now this number is used for auto priorities..
	// This may be replaced with total complete source known in the network..
	uint32	GetQueuedCount() { return m_ClientUploadList.GetCount();}
	bool	LoadHashsetFromFile(CFileDataIO* file, bool checkhash);
	void	AddUploadingClient(CUpDownClient* client);
	void	RemoveUploadingClient(CUpDownClient* client);
	virtual void	UpdatePartsInfo();
	virtual	void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat) const;
	//Ackronic START- Aggiunto da Aenarion[ITA] - PowerRelease
	bool	HideOvershares(CSafeMemFile* file, CUpDownClient* client); //Xman PowerRelease
	uint16 GetOnUploadqueue() const		{return onuploadqueue;}
	//Ackronic END- Aggiunto da Aenarion[ITA] - PowerRelease
	// comment
	void	SetFileComment(LPCTSTR pszComment);

	void	SetFileRating(uint8 uRating);

	bool	GetPublishedED2K() const { return m_PublishedED2K; }
	void	SetPublishedED2K(bool val);

	uint32	GetKadFileSearchID() const { return kadFileSearchID; }
	void	SetKadFileSearchID(uint32 id) { kadFileSearchID = id; } //Don't use this unless you know what your are DOING!! (Hopefully I do.. :)

	const Kademlia::WordList& GetKadKeywords() const { return wordlist; }

	uint32	GetLastPublishTimeKadSrc() const { return m_lastPublishTimeKadSrc; }
	void	SetLastPublishTimeKadSrc(uint32 time, uint32 buddyip) { m_lastPublishTimeKadSrc = time; m_lastBuddyIP = buddyip;}
	uint32	GetLastPublishBuddy() const { return m_lastBuddyIP; }
	void	SetLastPublishTimeKadNotes(uint32 time) {m_lastPublishTimeKadNotes = time;}
	uint32	GetLastPublishTimeKadNotes() const { return m_lastPublishTimeKadNotes; }

	bool	PublishSrc();
	bool	PublishNotes();

	// file sharing
	virtual Packet* CreateSrcInfoPacket(const CUpDownClient* forClient) const;
	UINT	GetMetaDataVer() const { return m_uMetaDataVer; }
	void	UpdateMetaDataTags();
	void	RemoveMetaDataTags();

	// preview
	bool	IsMovie() const;
	virtual bool GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender);

	// aich
	CAICHHashSet*	GetAICHHashset() const							{return m_pAICHHashSet;}
	void			SetAICHHashset(CAICHHashSet* val)				{m_pAICHHashSet = val;}
	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	uint32	m_tUtcLastModified;

	CStatisticFile statistic;
	time_t m_nCompleteSourcesTime;
	uint16 m_nCompleteSourcesCount;
	uint16 m_nCompleteSourcesCountLo;
	uint16 m_nCompleteSourcesCountHi;
	CUpDownClientPtrList m_ClientUploadList;
	CArray<uint16, uint16> m_AvailPartFrequency;
	CCollection* m_pCollection;
	bool ShareOnlyTheNeed(CSafeMemFile* file, CUpDownClient* client);//wistily Share only the need

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//preview
	bool	GrabImage(CString strFileName, uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	bool	LoadTagsFromFile(CFileDataIO* file);
	bool	LoadDateFromFile(CFileDataIO* file);
	void	CreateHash(CFile* pFile, UINT uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	bool	CreateHash(FILE* fp, UINT uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	bool	CreateHash(const uchar* pucData, UINT uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	virtual void	UpdateFileRatingCommentAvail();
	uint32	*CalcPartSpread();	//Ackronic - Aggiunto da Aenarion[ITA] - PowerRelease


	CArray<uchar*, uchar*>	hashlist;
	CString					m_strDirectory;
	CString					m_strFilePath;
	CAICHHashSet*			m_pAICHHashSet;

private:
	uint16 onuploadqueue;//Ackronic - Aggiunto da Aenarion[ITA] - PowerRelease
	static CBarShader s_ShareStatusBar;
	uint16	m_iPartCount;
	uint16	m_iED2KPartCount;
	uint16	m_iED2KPartHashCount;
	uint8	m_iUpPriority;
	bool	m_bAutoUpPriority;
	bool	m_PublishedED2K;
	uint32	kadFileSearchID;
	uint32	m_lastPublishTimeKadSrc;
	uint32	m_lastPublishTimeKadNotes;
	uint32	m_lastBuddyIP;
	Kademlia::WordList wordlist;
	UINT	m_uMetaDataVer;
	uint16 hideos;			//Ackronic - Aggiunto da Aenarion[ITA] - PowerRelease && hideos

	//>>> WiZaRd::PowerShare
private:
	bool	m_bIsPowerShared;
public:
	bool	IsPowerShared() const;
	void	SetPowerShared(const bool& b);
	//<<< WiZaRd::PowerShare
};
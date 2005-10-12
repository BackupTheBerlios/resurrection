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
//KTS+ Hideos
#include "SafeFile.h"
//KTS- Hideos

class CxImage;
class CUpDownClient;
class Packet;
class CFileDataIO;
class CAICHHashTree;
class CAICHHashSet;
class CCollection;

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
	//FRTK(kts)+
	// xMule_MOD: showSharePermissions
	uint8	GetPermissions(void) const { return m_iPermissions; };
	void	SetPermissions(uint8 iNewPermissions) {m_iPermissions = iNewPermissions;};
	// xMule_MOD: showSharePermissions
	//FRTK(kts)-
	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	const CString& GetPath() const { return m_strDirectory; }
	void SetPath(LPCTSTR path);

	const CString& GetFilePath() const { return m_strFilePath; }
	void SetFilePath(LPCTSTR pszFilePath);

	virtual bool CreateFromFile(LPCTSTR directory, LPCTSTR filename, LPVOID pvProgressParam); // create date, hashset and tags from a file
	virtual bool LoadFromFile(CFileDataIO* file);	//load date, hashset and tags from a .met file
	bool	WriteToFile(CFileDataIO* file);
	//MORPH START - Added by [ionix], Import Parts [SR13]
	bool	SR13_ImportParts();
	//MORPH END   - Added by [ionix], Import Parts [SR13]
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
//Telp Super Release
	bool	IsReleaseFile(void) const { return m_bReleaseFile; }
	void	SetReleaseFile(bool bReleaseFile);
//Telp Super Release
	// This has lost it's meaning here.. This is the total clients we know that want this file..
	// Right now this number is used for auto priorities..
	// This may be replaced with total complete source known in the network..
	uint32	GetQueuedCount() { return m_ClientUploadList.GetCount();}
	bool	LoadHashsetFromFile(CFileDataIO* file, bool checkhash);
	bool	HideOvershares(CSafeMemFile* file, CUpDownClient* client); //<<-- ADDED STORMIT - SLUGFILLER: hideOS //
	void	AddUploadingClient(CUpDownClient* client);
	void	RemoveUploadingClient(CUpDownClient* client);
	virtual void	UpdatePartsInfo();
	virtual	void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat); /*const;*/
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
//<<-- ADDED STORMIT - Morph: PowerShare //
	uint16 m_nVirtualCompleteSourcesCount;
	bool ShareOnlyTheNeed(CSafeMemFile* file, CUpDownClient* client);//wistily Share only the need
//<<-- ADDED STORMIT - Morph: PowerShare //
	//KTS+ Hideos
	CArray<uint16> m_PartSentCount;
	void	SetHideOS(int newValue) {m_iHideOS = newValue;};
	int		GetHideOS() const {return m_iHideOS;}
	void	SetSelectiveChunk(int newValue) {m_iSelectiveChunk = newValue;};
	int		GetSelectiveChunk() const {return m_iSelectiveChunk;}
uint8	HideOSInWork() const;
	void	SetSpreadbarSetStatus(int newValue) {m_iSpreadbarSetStatus = newValue;}
	int		GetSpreadbarSetStatus() const {return m_iSpreadbarSetStatus;}

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
//MORPH START - Added by SiRoB, copy feedback feature
	CString GetFeedback(bool isUS = false);
	//MORPH END   - Added by SiRoB, copy feedback feature
//<<-- ADDED STORMIT - Morph: PowerShare //
	void    SetPowerShared(int newValue) {m_powershared = newValue;};
	bool    GetPowerShared() const;
	void	SetShareOnlyTheNeed(int newValue) {m_iShareOnlyTheNeed = newValue;};
	int		GetShareOnlyTheNeed() const {return m_iShareOnlyTheNeed;}
	int		GetPowerSharedMode() const {return m_powershared;}
	bool	GetPowerShareAuthorized() const {return m_bPowerShareAuthorized;}
	bool	GetPowerShareAuto() const {return m_bPowerShareAuto;}
	void	SetPowerShareLimit(int newValue) {m_iPowerShareLimit = newValue;};
	int		GetPowerShareLimit() const {return m_iPowerShareLimit;}
	bool	GetPowerShareLimited() const {return m_bPowerShareLimited;}
	void	UpdatePowerShareLimit(bool authorizepowershare,bool autopowershare, bool limitedpowershare) {m_bPowerShareAuthorized = authorizepowershare;m_bPowerShareAuto = autopowershare;m_bPowerShareLimited = limitedpowershare;}
//<<-- ADDED STORMIT - Morph: PowerShare //

protected:
	//FRTK(kts)+
	uint8	m_iPermissions;	// xMule_MOD: showSharePermissions
	//FRTK(kts)-
	//preview
	bool	GrabImage(CString strFileName, uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	bool	LoadTagsFromFile(CFileDataIO* file);
	bool	LoadDateFromFile(CFileDataIO* file);
public: // [ionix] for SR13
	void	CreateHash(CFile* pFile, UINT uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	bool	CreateHash(FILE* fp, UINT uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	bool	CreateHash(const uchar* pucData, UINT uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	virtual void	UpdateFileRatingCommentAvail();
//<<-- ADDED STORMIT - Morph: PowerShare //
	uint16	CalcPartSpread(CArray<uint32, uint32>& partspread, CUpDownClient* client); // Morph: PowerShare // SLUGFILLER: hideOS

	CArray<uchar*, uchar*>	hashlist;
	CString					m_strDirectory;
	CString					m_strFilePath;
	CAICHHashSet*			m_pAICHHashSet;
 //<<-- ADDED STORMIT - Morph: PowerShare //
 	//MORPH START - Added by SiRoB,  SharedStatusBar CPU Optimisation
	bool	InChangedSharedStatusBar;
	CDC 	m_dcSharedStatusBar;
	CBitmap m_bitmapSharedStatusBar;
	CBitmap *m_pbitmapOldSharedStatusBar;
	int	lastSize;
	bool	lastonlygreyrect;
	bool	lastbFlat;
	//MORPH END - Added by SiRoB,  SharedStatusBar CPU Optimisation
//<<-- ADDED STORMIT - Morph: PowerShare //

private:
	static CBarShader s_ShareStatusBar;
	uint16	m_iPartCount;
	uint16	m_iED2KPartCount;
	uint16	m_iED2KPartHashCount;
	uint8	m_iUpPriority;
	bool	m_bAutoUpPriority;
	bool	m_bReleaseFile;	//Telp Super Release
	bool	m_PublishedED2K;
	uint32	kadFileSearchID;
	uint32	m_lastPublishTimeKadSrc;
	uint32	m_lastPublishTimeKadNotes;
	uint32	m_lastBuddyIP;
	Kademlia::WordList wordlist;
	UINT	m_uMetaDataVer;

//<<-- ADDED STORMIT - Morph: PowerShare //
	int		m_iHideOS;
	int		m_iSelectiveChunk;
	int		m_iShareOnlyTheNeed;
	int		m_powershared;
	bool	m_bPowerShareAuthorized;
	bool	m_bPowerShareAuto;
	int		m_iPowerShareLimit;
	bool	m_bPowerShareLimited;
//<<-- ADDED STORMIT - Morph: PowerShare //
int		m_iSpreadbarSetStatus;
bool	m_bHideOSAuthorized;

};
//FRTK(kts)+
// xMule_MOD: showSharePermissions
// permission values for shared files
#define PERM_ALL		0
#define PERM_FRIENDS	1
#define PERM_NOONE		2
// xMule_MOD: showSharePermissions
//FRTK(kts)-

#include "types.h"

#pragma once

class CPartFile;
class CUpDownClient;

class CSourceSaver // WAS // class CSourceSaver //[ionix]- SLS - //
{
public:
	CSourceSaver(void);
	~CSourceSaver(void);

protected:
	class CSourceData
	{
	public:
		CSourceData(uint32 dwID, uint16 wPort, const TCHAR* exp, uint8 srcexver) {	sourceID = dwID; 
		sourcePort = wPort; 
		MEMCOPY(expiration, exp, 7 * sizeof(TCHAR));
		expiration[6] = 0;
		nSrcExchangeVer = srcexver;}

		CSourceData(CUpDownClient* client, const TCHAR* exp);
		CSourceData(CSourceData* pOld)
		{
		sourcePort = pOld->sourcePort; 
		MEMCOPY(expiration, pOld->expiration, 7); 
		partsavailable = pOld->partsavailable;
		expiration[6] = 0;
		nSrcExchangeVer = pOld->nSrcExchangeVer;}

		bool Compare(CSourceData* tocompare) {						return ((sourceID == tocompare->sourceID) 
			&& (sourcePort == tocompare->sourcePort)); }

		uint32	sourceID;
		uint16	sourcePort;
		uint32	partsavailable;
		TCHAR	expiration[7];
		uint8	nSrcExchangeVer;
	};
	typedef CTypedPtrList<CPtrList, CSourceData*> SourceList;

	void LoadSourcesFromFile(CPartFile* file, SourceList* sources, CString& slsfile);
	void AddSourcesToDownload(CPartFile* file, SourceList* sources);

	bool m_LoadSourcesOnStart;
	uint32	m_dwLastTimeLoaded;
	uint32  m_dwLastTimeSaved;
////	uint32	m_dwlastTimeActivated;

	CString CalcExpiration(int nDays);
	bool IsExpired(CString expirationdate);
};

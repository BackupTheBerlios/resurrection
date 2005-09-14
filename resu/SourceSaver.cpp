/////////////////////////////////////////////////////////////////////////////////////////////////
//<<-- STORMIT - Source Loader Saver [SLS] //

#include "StdAfx.h"
#include "SourceSaver.h"
#include "PartFile.h"
#include "emule.h"
#include "updownclient.h"
#include "Preferences.h" // for thePrefs
#include "emuleDlg.h"
#include "DownloadQueue.h" // for theApp.downloadqueue
#include "Clientlist.h" // for theApp.clientlist
#include "OtherFunctions.h" // for ipstr()
#include "log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define RELOADTIME	3600000 //60 minutes
#define RESAVETIME	 1600000 //30 minutes <<<!!!>>>


CSourceSaver::CSourceSaver(void)
{
	m_LoadSourcesOnStart = true;
	m_dwLastTimeLoaded = ::GetTickCount() - RELOADTIME;
	m_dwLastTimeSaved = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000 - RESAVETIME;
}

CSourceSaver::CSourceData::CSourceData(CUpDownClient* client, const TCHAR* exp)
{
	nSrcExchangeVer = client->GetSourceExchangeVersion();
	if(nSrcExchangeVer > 2)
		sourceID = client->GetUserIDHybrid();
	else
		sourceID = client->GetIP();
	sourcePort = client->GetUserPort();
	partsavailable = client->GetAvailablePartCount();
	MEMCOPY(expiration, exp, 7 * sizeof(TCHAR));
	expiration[6] = 0;
}

CSourceSaver::~CSourceSaver(void)
{
}

 bool CSourceSaver::Process(CPartFile* file) // , int maxSourcesToSave) // return false if sources not saved
 {
	if (thePrefs.UseSaveLoadSources()) 

	if ((int)(::GetTickCount() - m_dwLastTimeSaved) > RESAVETIME)
	{
		m_dwLastTimeSaved = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000;
		SourceList srcs;
		CString slsfilepath;
		slsfilepath.Format(_T("%s\\%s\\%s.ack"), thePrefs.GetTempDir(), _T("Saved Sources"), file->GetPartMetFileName()); // [ionix]
		LoadSourcesFromFile(file, &srcs, slsfilepath);
		SaveSources(file, &srcs, slsfilepath, thePrefs.GetSourcesToSaveSLS());
		
		if ( m_LoadSourcesOnStart ) 
		{
			m_LoadSourcesOnStart = false;

			if ((int)(::GetTickCount() - m_dwLastTimeLoaded) >= RELOADTIME)
			{	
				m_dwLastTimeLoaded = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000;
				AddSourcesToDownload(file, &srcs);
			}
		}

		while (!srcs.IsEmpty())
			delete srcs.RemoveHead();

		return true;
	}
	return false;
}

void CSourceSaver::DeleteFile(CPartFile* file, bool ErrIfFailed)
{
	CString slsfilepath;
	slsfilepath.Format(_T("%s\\%s\\%s.ack"), thePrefs.GetTempDir(), _T("Saved Sources"), file->GetPartMetFileName()); // [ionix]
	if(_tremove(slsfilepath) && errno != ENOENT && ErrIfFailed)
		AddLogLine(true, _T("Failed to delete 'temp\\Saved Sources\\%s.src', you will need to do this by hand."), file->GetPartMetFileName());
}

void CSourceSaver::LoadSourcesFromFile(CPartFile* file, SourceList* sources, CString& slsfile)
{
	CString strLine;
	CStdioFile f;

	if (!f.Open(slsfile, CFile::modeRead | CFile::typeText))
		return;

	while(f.ReadString(strLine)) {

		if (strLine.GetAt(0) == _T('#'))
			continue;
		int pos = strLine.Find(_T(':'));
		if (pos == -1)
			continue;
		CString strIP = strLine.Left(pos);
		uint32 dwID = inet_addr(CT2CA(strIP));
		if (dwID == INADDR_NONE)
			continue;
		strLine = strLine.Mid(pos+1);
		pos = strLine.Find(_T(","));
		if (pos == -1)
			continue;
		CString strPort = strLine.Left(pos);
		uint16 wPort = _tstoi(strPort);
		if (!wPort)
			continue;
		strLine = strLine.Mid(pos+1);		
		pos = strLine.Find(_T(","));
		if (pos == -1)
			continue;
		CString strExpiration = strLine.Left(pos);
 // sivka [-bugfix-] //
		for(int i = strExpiration.GetLength(); i < 10; )
			 i = strExpiration.Insert(i, _T("0"));
 // sivka [-bugfix-] //
		if (IsExpired(strExpiration))
			continue;
		strLine = strLine.Mid(pos+1);
		pos = strLine.Find(_T(";"));

		if (pos == -1 /*|| strLine.GetLength() < 2*/)
			continue;
		uint8 nSrcExchangeVer = _tstoi(strLine.Left(pos));

		CSourceData* newsource = new CSourceData(dwID, wPort, strExpiration, nSrcExchangeVer);
		sources->AddTail(newsource);
	}
	f.Close();
}

void CSourceSaver::AddSourcesToDownload(CPartFile* file, SourceList* sources)
{

	for (POSITION pos = sources->GetHeadPosition(); pos;)
	{
		if (file->GetMaxSources() <= file->GetSourceCount())
			return;

		CSourceData* cur_src = sources->GetNext(pos);
		CUpDownClient* newclient;
		if( cur_src->nSrcExchangeVer == 3 )
			newclient = new CUpDownClient(file, cur_src->sourcePort, cur_src->sourceID, 0, 0, false);
		else
			newclient = new CUpDownClient(file, cur_src->sourcePort, cur_src->sourceID, 0, 0, true);
		if (theApp.downloadqueue->CheckAndAddSource(file, newclient)) 
    //[ionix]- Source Loader Saver [SLS] //
			newclient->SetSourceFrom(SF_SLS);  //[ionix]- Source Loader Saver [SLS] //
  //[ionix]- Source Loader Saver [SLS] //
 
	}
	AddDebugLogLine(/*TBN_NONOTIFY, */false, _T("SLS: Loaded %i sources for file %s"), sources->GetCount(), file->GetFileName());
}

////#define SOURCESTOSAVE	25
#define EXPIREIN	1 //days

void CSourceSaver::SaveSources(CPartFile* file, SourceList* prevsources, CString& slsfile, int maxSourcesToSave)
{
	SourceList srcstosave;
	CSourceData* sourcedata;

	ASSERT(srcstosave.IsEmpty());
	POSITION pos, pos2;

	// Choose best sources for the file
		for (pos = file->srclist.GetHeadPosition(); pos != NULL;)
		{
			CUpDownClient*cur_src = file->srclist.GetNext(pos);
			if (cur_src->HasLowID())
			continue;
		if (srcstosave.IsEmpty()) {
			sourcedata = new CSourceData(cur_src, CalcExpiration(EXPIREIN));
			srcstosave.AddHead(sourcedata);
			continue;
		}
			if (srcstosave.GetCount() < maxSourcesToSave || cur_src->GetAvailablePartCount() > srcstosave.GetTail()->partsavailable || cur_src->GetSourceExchangeVersion() > srcstosave.GetTail()->nSrcExchangeVer)
		{
				if (srcstosave.GetCount() == maxSourcesToSave)
				delete srcstosave.RemoveTail();
				ASSERT(srcstosave.GetCount() < maxSourcesToSave);
			bool bInserted = false;
			for (pos2 = srcstosave.GetTailPosition(); pos2 != NULL; srcstosave.GetPrev(pos2))
			{
					CSourceData* cur_srctosave = srcstosave.GetAt(pos2);
				if (file->GetValidSourcesCount() > (maxSourcesToSave*2) 
					&& cur_srctosave->nSrcExchangeVer > cur_src->GetSourceExchangeVersion())
					{
						bInserted = true;
					}
				else if (file->GetValidSourcesCount() > (maxSourcesToSave*2) 
					&& cur_srctosave->nSrcExchangeVer == cur_src->GetSourceExchangeVersion() 
					&& cur_srctosave->partsavailable > cur_src->GetAvailablePartCount())
					{
						bInserted = true;
					}
				else if (file->GetValidSourcesCount() <= (maxSourcesToSave*2) 
					&& cur_srctosave->partsavailable > cur_src->GetAvailablePartCount())
					{
						bInserted = true;
					}
					if (bInserted)
					{
						sourcedata = new CSourceData(cur_src, CalcExpiration(EXPIREIN));
						srcstosave.InsertAfter(pos2, sourcedata);
						break;
					}
				}
				if (!bInserted) 
				{
					sourcedata = new CSourceData(cur_src, CalcExpiration(EXPIREIN));
					srcstosave.AddHead(sourcedata);
				}
			}
	}

	// Add previously saved sources if found sources does not reach the limit
	for (pos = prevsources->GetHeadPosition(); pos; prevsources->GetNext(pos)) {
		CSourceData* cur_sourcedata = prevsources->GetAt(pos);
		if (srcstosave.GetCount() == maxSourcesToSave)
			break;
		ASSERT(srcstosave.GetCount() <= maxSourcesToSave);

		bool bFound = false;
		for (pos2 = srcstosave.GetHeadPosition(); pos2; srcstosave.GetNext(pos2)) {
			if (srcstosave.GetAt(pos2)->Compare(cur_sourcedata)) {
				bFound = true;
				break;
			}
		}
		if (!bFound) {
			srcstosave.AddTail(new CSourceData(cur_sourcedata));
		}

	}

	//DEBUG_ONLY(AddLogLine(/*TBN_NONOTIFY, */false, _T("Saving %i sources for file %s"), srcstosave.GetCount(), file->GetFileName()));

	CString strLine;
	CStdioFile f;
	if (!f.Open(slsfile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return;
	f.WriteString(_T("#format: a.b.c.d:port,expirationdate(yymmdd);\r\n"));
	f.WriteString(_T("#") + CreateED2kLink(file) + _T("\r\n")); // ADDED STORMIT - SAVE FILE INFORMATION //
	while (!srcstosave.IsEmpty()) {
		CSourceData* cur_src = srcstosave.RemoveHead();
		uint32 dwID = cur_src->sourceID;
		strLine.Format(_T("%u.%u.%u.%u:%u,%s,%u;\r\n"), (uint8)dwID,(uint8)(dwID>>8),(uint8)(dwID>>16),(uint8)(dwID>>24), cur_src->sourcePort, cur_src->expiration, cur_src->nSrcExchangeVer);
		delete cur_src;
		f.WriteString(strLine);
	}
	f.Close();
}

CString CSourceSaver::CalcExpiration(int nDays)
{
	CTime expiration = CTime::GetCurrentTime();
	CTimeSpan timediff(nDays, 0, 0, 0);
	expiration += timediff;

	CString strExpiration;
	strExpiration.Format(_T("%02i%02i%02i"), (expiration.GetYear() % 100), expiration.GetMonth(), expiration.GetDay());

	return strExpiration;
}

bool CSourceSaver::IsExpired(CString expirationdate)
{
	int year = _tstoi(expirationdate.Mid(0, 2)) + 2000;
	int month = _tstoi(expirationdate.Mid(2, 2));
	int day = _tstoi(expirationdate.Mid(4, 2));

	CTime expiration(year, month, day, 0, 0, 0);
	return (expiration < CTime::GetCurrentTime());
}

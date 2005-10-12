//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

#include "StdAfx.h"
#include <share.h>
#include "fakecheck.h"
#include "emule.h"
#include "otherfunctions.h"
#include "HttpDownloadDlg.h"
#include "emuleDlg.h"
#include "Preferences.h"
#include "ZipFile.h"
#include "log.h"
//>>> WiZaRd::eD2K Updates
#include "extractfile.h" 
#include "ED2KLink.h" 
#include "DownloadQueue.h" 
#include "PartFile.h"
//<<< WiZaRd::eD2K Updates

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

CFakecheck::CFakecheck(){
	m_pLastHit = NULL;
	LoadFromFile();
}

CFakecheck::~CFakecheck(){
	RemoveAllFakes();
}

void CFakecheck::AddFake(uchar* Hash,uint32& Length,CString& Realtitle){
	Fakes_Struct* newFilter=new Fakes_Struct;
	md4cpy(newFilter->Hash, Hash);
	newFilter->Length=Length;
	newFilter->RealTitle=Realtitle;
	m_fakelist.Add(newFilter);
}

static int __cdecl CmpFakeByHash_Length(const void* p1, const void* p2)
{
	const Fakes_Struct* pFake1 = *(Fakes_Struct**)p1;
	const Fakes_Struct* pFake2 = *(Fakes_Struct**)p2;
	int diff = memcmp(pFake1->Hash, pFake2->Hash, 16);
	if (diff)
		return diff;
	return pFake1->Length-pFake2->Length;
}

//>>> WiZaRd::ed2K Updates
/*int CFakecheck::LoadFromFile(){
	FILE* readFile = _tfsopen(thePrefs.GetConfigDir()+DFLT_FAKECHECK_FILENAME, _T("r"), _SH_DENYWR);
	if (readFile!=NULL) {
		CString sbuffer, sbuffer2;
		int pos;
		uint32 Lenght;
		CString Title;
		char buffer[1024];
		int fakecounter = 0;
		int iDuplicate = 0;
		int iMerged = 0;
		RemoveAllFakes();
		while (fgets(buffer, ARRSIZE(buffer), readFile) != NULL)
		{
			
			sbuffer=buffer;
			if (sbuffer.GetAt(0) == _T('#') || sbuffer.GetAt(0) == _T('/') || sbuffer.GetLength() < 5)
				continue;
			pos=sbuffer.Find(_T(','));
			if (pos==-1) continue;
			sbuffer2=sbuffer.Left(pos).Trim();
			uchar Hash[16];
			DecodeBase16(sbuffer2.GetBuffer(),sbuffer2.GetLength(),Hash,ARRSIZE(Hash));
			int pos2=sbuffer.Find(_T(","),pos+1);
			if (pos2==-1) continue;
			Lenght=_tstoi(sbuffer.Mid(pos+1,pos2-pos-1).Trim());
			Title=sbuffer.Mid(pos2+1,sbuffer.GetLength()-pos2-2);
			AddFake(&Hash[0],Lenght,Title);
			++fakecounter;
		}
		fclose(readFile);
		// sort the FakeCheck entry by Hash 
		qsort(m_fakelist.GetData(), m_fakelist.GetCount(), sizeof(m_fakelist[0]), CmpFakeByHash_Lenght);

		// merge overlapping and adjacent filter ranges
		if (m_fakelist.GetCount() >= 2)
		{
			Fakes_Struct* pPrv = m_fakelist[0];
			int i = 1;
			while (i < m_fakelist.GetCount())
			{
				Fakes_Struct* pCur = m_fakelist[i];
				if ( pCur->Hash == pPrv->Hash && pCur->Lenght == pPrv->Lenght)
				{
					if (pCur->RealTitle != pPrv->RealTitle)
					{
						//pPrv->RealTitle += _T("; ") + pCur->RealTitle;
						iMerged++;
					}
					else
					{
						iDuplicate++;
					}
					delete pCur;
					m_fakelist.RemoveAt(i);
					continue;
				}
				pPrv = pCur;
				++i;
			}
		}
*/
bool CFakecheck::LoadFromFile(const CString& path)
{
	CString strfakecheckfile = path;
	if(strfakecheckfile.IsEmpty())
		strfakecheckfile = GetDefaultFilePath();

	FILE* readFile = _tfsopen(strfakecheckfile, _T("r"), _SH_DENYWR);
	if (readFile == NULL) 
	{
		thePrefs.SetFakesDatVersion(NULL);
		return false;
	}

	CString sbuffer, sbuffer2;
	int pos;
	uint32 Length;
	CString Title;
	char buffer[512];
	int fakecounter = 0;
	int iDuplicate = 0;
	int iMerged = 0;
	RemoveAllFakes();
	while (fgets(buffer, ARRSIZE(buffer), readFile) != NULL)
	{			
		sbuffer=buffer;
		if (sbuffer.GetAt(0) == _T('#') || sbuffer.GetAt(0) == _T('/') || sbuffer.GetLength() < 5)
			continue;
		pos=sbuffer.Find(_T(','));
		if (pos==-1) continue;
		sbuffer2=sbuffer.Left(pos).Trim();
		uchar Hash[16];
		DecodeBase16(sbuffer2.GetBuffer(),sbuffer2.GetLength(),Hash,ARRSIZE(Hash));
		int pos2=sbuffer.Find(_T(","),pos+1);
		if (pos2==-1) continue;
		Length=_tstoi(sbuffer.Mid(pos+1,pos2-pos-1).Trim());
		Title=sbuffer.Mid(pos2+1,sbuffer.GetLength()-pos2-2);
		AddFake(&Hash[0],Length,Title);
		++fakecounter;
	}
	fclose(readFile);
	// sort the FakeCheck entry by Hash 
	qsort(m_fakelist.GetData(), m_fakelist.GetCount(), sizeof(m_fakelist[0]), CmpFakeByHash_Length);

	// merge overlapping and adjacent filter ranges
	if (m_fakelist.GetCount() >= 2)
	{
		Fakes_Struct* pPrv = m_fakelist[0];
		int i = 1;
		while (i < m_fakelist.GetCount())
		{
			Fakes_Struct* pCur = m_fakelist[i];
			if ( pCur->Hash == pPrv->Hash && pCur->Length == pPrv->Length)
			{
				if (pCur->RealTitle != pPrv->RealTitle)
				{
					//pPrv->RealTitle += _T("; ") + pCur->RealTitle;
					iMerged++;
				}
				else
				{
					iDuplicate++;
				}
				delete pCur;
				m_fakelist.RemoveAt(i);
				continue;
			}
			pPrv = pCur;
			++i;
		}
	}
//<<< WiZaRd::eD2K Updates
		AddLogLine(m_fakelist.GetCount() == 0 ? LOG_WARNING : LOG_SUCCESS, _T("%i Fake Check reference loaded"), m_fakelist.GetCount());
		if (thePrefs.GetVerbose())
		{
			AddDebugLogLine(false, _T("Found Fake Reference:%u  Duplicate:%u  Merged:%u"), fakecounter, iDuplicate, iMerged);
		}
	//return m_fakelist.GetCount();
	return true;
	}

//>>> WiZaRd::eD2K Updates
bool CFakecheck::SaveToFile(const CString& path)
{
	CString strfakecheckfile = path;
	if(strfakecheckfile.IsEmpty())
		strfakecheckfile = GetDefaultFilePath();

	CStdioFile f;
	if (!f.Open(strfakecheckfile, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive | CFile::typeText))
		return false;

	const DWORD startMesure = GetTickCount();	

	for(int i = 0; i < m_fakelist.GetCount(); ++i) 
	{
		Fakes_Struct* s = m_fakelist.GetAt(i);
		CString towrite;
		towrite.Format(_T("%s,%u,%s\n"), md4str(s->Hash), s->Length, s->RealTitle);
		f.WriteString(towrite); //write our string
}

	AddLogLine(false, _T("FakeCheck data for %u file(s) saved in %s"), m_fakelist.GetCount(), CastSecondsToHM((::GetTickCount()-startMesure)/1000));
	f.Close(); //exit	
	return true;
}
//<<< WiZaRd::eD2K Updates

bool CFakecheck::IsFake(uchar* Hash2test, uint32 lenght){
	if (m_fakelist.GetCount() == 0)
		return false;
	Fakes_Struct** ppFound = (Fakes_Struct**)bsearch(&Hash2test, m_fakelist.GetData(), m_fakelist.GetCount(), sizeof(m_fakelist[0]), CmpFakeByHash_Length);
	if (ppFound)
	{
		m_pLastHit = *ppFound;
		return true;
	}

	return false;
}
CString CFakecheck::GetLastHit() const
{
	return m_pLastHit ? m_pLastHit->RealTitle : _T("No Fake");
}

void CFakecheck::RemoveAllFakes()
{
	for (int i = 0; i < m_fakelist.GetCount(); i++)
		delete m_fakelist[i];
	m_fakelist.RemoveAll();
	m_pLastHit = NULL;
}

//>>> WiZaRd::eD2K Updates
void CFakecheck::DownloadFakeList(const CString& m_sURL)
//void CFakecheck::DownloadFakeList()
//<<< WiZaRd::eD2K Updates
{
//>>> WiZaRd::eD2K Updates
	CString strURL;
	if(m_sURL.IsEmpty())
		strURL = thePrefs.GetUpdateURLFakeList();
	else
		strURL = m_sURL;
	//CString strURL = thePrefs.GetUpdateURLFakeList();
//<<< WiZaRd::eD2K Updates
	strURL.TrimRight(_T(".txt"));
	strURL.TrimRight(_T(".dat"));
	strURL.TrimRight(_T(".zip"));
	strURL.Append(_T(".txt"));

	TCHAR szTempFilePath[_MAX_PATH];
	_tmakepath(szTempFilePath, NULL, thePrefs.GetConfigDir(), DFLT_FAKECHECK_FILENAME, _T("tmp"));
//>>> WiZaRd::eD2K Updates
	//FILE* readFile= _tfsopen(szTempFilePath, _T("r"), _SH_DENYWR); //HANDLE LEAK - is never closed/used!
//<<< WiZaRd::eD2K Updates

	CHttpDownloadDlg dlgDownload;
	dlgDownload.m_strTitle = _T("Downloading Fake Check version file");
	dlgDownload.m_sURLToDownload = strURL;
	dlgDownload.m_sFileToDownloadInto = szTempFilePath;

	if (dlgDownload.DoModal() != IDOK)
	{
		_tremove(szTempFilePath);
		AddLogLine(true, _T("Error downloading %s"), strURL);
		return;
	}
//>>> WiZaRd::eD2K Updates
	/*
	FILE* readFile = _tfsopen(szTempFilePath, _T("r"), _SH_DENYWR);

	char buffer[9];
	int lenBuf = 9;
	fgets(buffer,lenBuf,readFile);
	CString sbuffer = buffer;
	sbuffer = sbuffer.Trim();
	fclose(readFile);
	_tremove(szTempFilePath);
	*/
	CStdioFile FakesDotTxtFile;
	CString strVersion = _T("");
	CString strED2KLink = _T("");
	if(FakesDotTxtFile.Open(szTempFilePath, CFile::modeRead | CFile::shareDenyWrite))
	{
		FakesDotTxtFile.ReadString(strVersion);
		FakesDotTxtFile.ReadString(strED2KLink);
		FakesDotTxtFile.Close();		
		strVersion.Trim();
		strED2KLink.Trim();
	}
	_tremove(szTempFilePath);
//<<< WiZaRd::eD2K Updates

	if ((thePrefs.GetFakesDatVersion() < (uint32) _tstoi(strVersion)) || !PathFileExists(GetDefaultFilePath()) || FileSize(GetDefaultFilePath()) < 10240)
	{
//>>> WiZaRd::eD2K Updates
		if(strED2KLink.IsEmpty())
		{
			CString FakeCheckURL;
			if(m_sURL.IsEmpty())
				FakeCheckURL = thePrefs.GetUpdateURLFakeList();
			else
				FakeCheckURL = m_sURL;
			//CString FakeCheckURL = thePrefs.GetUpdateURLFakeList();
//<<< WiZaRd::eD2K Updates

		_tmakepath(szTempFilePath, NULL, thePrefs.GetConfigDir(), DFLT_FAKECHECK_FILENAME, _T("tmp"));
	
		CHttpDownloadDlg dlgDownload;
			dlgDownload.m_strTitle = _T("Downloading Fake Check file");
		dlgDownload.m_sURLToDownload = FakeCheckURL;
		dlgDownload.m_sFileToDownloadInto = szTempFilePath;
		if (dlgDownload.DoModal() != IDOK || FileSize(szTempFilePath) < 10240)
		{
			_tremove(szTempFilePath);
			//AddLogLine(true, RGB_LOG_ERROR +  GetResString(IDS_FAKECHECKUPERROR));
			AddLogLine(true, GetResString(IDS_FAKECHECKUPERROR));
		return;
	}

		bool bIsZipFile = false;
		bool bUnzipped = false;
//>>> WiZaRd::eD2K Updates
			Unzip(szTempFilePath, thePrefs.GetConfigDir(), DFLT_FAKECHECK_FILENAME, bIsZipFile, bUnzipped);
/*
		CZIPFile zip;
		if (zip.Open(szTempFilePath))
		{
			bIsZipFile = true;

			CZIPFile::File* zfile = zip.GetFile(DFLT_FAKECHECK_FILENAME);
			if (zfile)
			{
				TCHAR szTempUnzipFilePath[MAX_PATH];
				_tmakepath(szTempUnzipFilePath, NULL, thePrefs.GetConfigDir(), DFLT_FAKECHECK_FILENAME, _T(".unzip.tmp"));
				if (zfile->Extract(szTempUnzipFilePath))
				{
					zip.Close();
					zfile = NULL;

					if (_tremove(GetDefaultFilePath()) != 0)
						TRACE("*** Error: Failed to remove default fake check file \"%s\" - %s\n", GetDefaultFilePath(), _tcserror(errno));
					if (_trename(szTempUnzipFilePath, GetDefaultFilePath()) != 0)
						TRACE("*** Error: Failed to rename uncompressed fake check file \"%s\" to default fake check file \"%s\" - %s\n", szTempUnzipFilePath, GetDefaultFilePath(), _tcserror(errno));
					if (_tremove(szTempFilePath) != 0)
						TRACE("*** Error: Failed to remove temporary fake check file \"%s\" - %s\n", szTempFilePath, _tcserror(errno));
					bUnzipped = true;
				}
				else
						AddLogLine(true, _T("Failed to extract fake check file from downloaded fake check ZIP file \"%s\"."), szTempFilePath);
			}
			else
					AddLogLine(true, _T("Downloaded fake check file \"%s\" is a ZIP file with unexpected content."), szTempFilePath);

			zip.Close();
		}
*/
//<<< WiZaRd::eD2K Updates
		if (!bIsZipFile && !bUnzipped)
		{
			_tremove(GetDefaultFilePath());
			_trename(szTempFilePath, GetDefaultFilePath());
		}

			if(bIsZipFile && !bUnzipped)
			return;
		

		LoadFromFile();

			thePrefs.SetFakesDatVersion(_tstoi(strVersion));
	thePrefs.Save();
		}
//>>> WiZaRd::eD2K Updates
		else if(uint32(_tstoi(strVersion)) > thePrefs.GetDLingFakeListVersion())
		{		
			if (!thePrefs.GetDLingFakeListLink().IsEmpty())
			{
				CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(thePrefs.GetDLingFakeListLink());
				if (pLink != NULL)
				{
					CED2KFileLink* pFileLink = pLink->GetFileLink();
					if (pFileLink != NULL)
					{
						CPartFile* file = theApp.downloadqueue->GetFileByID(pFileLink->GetHashKey());
						if(file)
						{
							//TODO - delete old file!
						}
					}
				}
				delete pLink;
			}

			CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(strED2KLink);
			if (pLink != NULL)
			{
				CED2KFileLink* pFileLink = pLink->GetFileLink();
				if (pFileLink != NULL)
				{
					if (!theApp.downloadqueue->IsFileExisting(pFileLink->GetHashKey()))
					{
						theApp.downloadqueue->AddFileLinkToDownload(pFileLink);
						CPartFile* file = theApp.downloadqueue->GetFileByID(pFileLink->GetHashKey());
						if (file != NULL)
						{
							thePrefs.SetDLingFakeListVersion(_tstoi(strVersion));
							thePrefs.SetDLingFakeListLink(strED2KLink);
							file->SetSpecialFile(TYPE_FAKE);
		thePrefs.Save();
	}
						else
							AddLogLine(false, _T("Error updating fake list"));
					}
				}
			}
			delete pLink;
		}
//<<< WiZaRd::eD2K Updates
	}
}
CString CFakecheck::GetDefaultFilePath() const
{
	return thePrefs.GetConfigDir() + DFLT_FAKECHECK_FILENAME;
}//>>> WiZaRd::eD2K Updates
void CFakecheck::AddFakeFile(const CString& path, const bool& removeold)
{
	if(!PathFileExists(path)) //just to be sure...
	{
		AddDebugLogLine(false, _T("Failed to update \"%s\" with \"%s\" - file does not exist!"), GetDefaultFilePath(), path);
			return;
		}

	if(removeold)
		RemoveAllFakes();

	//Backup current file
	const CString strBackupFilePath = GetDefaultFilePath() + _T(".bak");
	::MoveFile(GetDefaultFilePath(), strBackupFilePath);

	bool bLoaded = false; //success indicator

	bool bIsZipFile = false;
	bool bUnzipped = false;
	if(IsRar(path) && (UnRAR(path, thePrefs.GetConfigDir())) )
		bLoaded = LoadFromFile(GetDefaultFilePath())!=0;
	else // Plaintext will be unzipped too ;)
		Unzip(path, thePrefs.GetConfigDir(), DFLT_FAKECHECK_FILENAME, bIsZipFile, bUnzipped);
	//an error occured here...

	if(bUnzipped)
		bLoaded = LoadFromFile(GetDefaultFilePath())!=0; 

	//success
	if(bLoaded)
	{
		SaveToFile(); //save the current file...
		::DeleteFile(strBackupFilePath); //kick the old file...		
		AddDebugLogLine(false, _T("Successfully updated \"%s\" with \"%s\""), GetDefaultFilePath(), path);
}
	else //failed 
{
		::MoveFile(strBackupFilePath, GetDefaultFilePath()); //restore backup
		AddDebugLogLine(false, _T("Failed to update \"%s\" with \"%s\""), GetDefaultFilePath(), path);
	}
}//<<< WiZaRd::eD2K Updates

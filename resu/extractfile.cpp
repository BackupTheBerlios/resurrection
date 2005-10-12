#include "stdafx.h"
#include "extractfile.h"
#include "Log.h"
#include "ZipFile.h"
#include "GZipFile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define __SFUNCTION__ CString(__FUNCTION__)
HANDLE	(WINAPI *pfnRAROpenArchiveEx)(RAROpenArchiveDataEx *pArchiveData);
int		(WINAPI *pfnRARCloseArchive)(HANDLE hArcData);
int		(WINAPI *pfnRARReadHeader)(HANDLE hArcData, RARHeaderData *pHeaderData);
int		(WINAPI *pfnRARProcessFileW)(HANDLE hArcData,int Operation,wchar_t *DestPath,wchar_t *DestName);

void OutOpenArchiveError(int iError, const CString& strArchivePath)
{
	CString pcError = _T("");

	switch (iError)
	{
		case ERAR_NO_MEMORY:
			pcError = _T("%s: Not enough memory to extract %s");
			break;

		case ERAR_EOPEN:
			pcError = _T("%s: Cannot open %s");
			break;

		case ERAR_BAD_ARCHIVE:
			pcError = _T("%s: %s is not RAR archive");
			break;

		case ERAR_BAD_DATA:
			pcError = _T("%s: %s: archive header broken");
			break;

		case ERAR_UNKNOWN:
			pcError = _T("%s: Unknown error when extracting %s");
			break;
	}

	if(!pcError.IsEmpty())
		AddDebugLogLine(false, pcError, __SFUNCTION__, strArchivePath);
}

void OutProcessFileError(int iError, const CString &strArchivePath)
{
	CString pcError = _T("");

	switch (iError)
	{
		case ERAR_UNKNOWN_FORMAT:
			pcError = _T("%s: %s: Unknown archive format");
			break;

		case ERAR_BAD_ARCHIVE:
			pcError = _T("%s: %s: Bad volume");
			break;

		case ERAR_ECREATE:
			pcError = _T("%s: %s: File create error");
			break;

		case ERAR_EOPEN:
			pcError = _T("%s: %s: Volume open error");
			break;

		case ERAR_ECLOSE:
			pcError = _T("%s: %s: File close error");
			break;

		case ERAR_EREAD:
			pcError = _T("%s: %s: Read error");
			break;

		case ERAR_EWRITE:
			pcError = _T("%s: %s: Write error");
			break;

		case ERAR_BAD_DATA:
			pcError = _T("%s: %s: CRC error");
			break;

		case ERAR_UNKNOWN:
			pcError = _T("%s: %s: Unknown error");
			break;
	}

	if (!pcError.IsEmpty())
		AddDebugLogLine(false, pcError, __SFUNCTION__, strArchivePath); // Is this right? - compiler gives a warning
}

bool UnRAR(const CString &strArchivePath, CString strDestFolder)
{
	HINSTANCE hUnRARDLL = LoadLibrary(_T("unrar.dll"));
	if (hUnRARDLL == NULL)
	{
		AddDebugLogLine(false, _T("%s: Can't find or load Unrar.dll"), __SFUNCTION__);
		return false;
	}
	AddDebugLogLine(false, _T("%s: Unrar.dll loaded"), __SFUNCTION__);

	(FARPROC&)pfnRAROpenArchiveEx = GetProcAddress(hUnRARDLL, "RAROpenArchiveEx");
	(FARPROC&)pfnRARCloseArchive = GetProcAddress(hUnRARDLL, "RARCloseArchive");
	(FARPROC&)pfnRARReadHeader = GetProcAddress(hUnRARDLL, "RARReadHeader");
	(FARPROC&)pfnRARProcessFileW = GetProcAddress(hUnRARDLL, "RARProcessFileW");

	if ( pfnRAROpenArchiveEx == NULL 
		|| pfnRARCloseArchive == NULL 
		|| pfnRARReadHeader == NULL
		|| pfnRARProcessFileW  == NULL )
	{
		FreeLibrary(hUnRARDLL);
		AddDebugLogLine(false, _T("%s: Unrar.dll unloaded"), __SFUNCTION__);
		return false;
	}

	RAROpenArchiveDataEx OpenArchiveData;

	ZeroMemory(&OpenArchiveData, sizeof(OpenArchiveData));
	OpenArchiveData.ArcNameW = (wchar_t*)strArchivePath.GetString(); //UNICODE - use WCHAR
	OpenArchiveData.CmtBuf = NULL;
	OpenArchiveData.OpenMode = RAR_OM_EXTRACT;

	HANDLE hArcData = pfnRAROpenArchiveEx(&OpenArchiveData);
	if (OpenArchiveData.OpenResult != 0)
	{
		OutOpenArchiveError(OpenArchiveData.OpenResult, strArchivePath);
		FreeLibrary(hUnRARDLL);
		AddDebugLogLine(false, _T("%s: Unrar.dll unloaded"), __SFUNCTION__);
		return false;
	}

	RARHeaderData HeaderData;

	HeaderData.CmtBuf = NULL;
	int iReadHeaderCode;
	while ((iReadHeaderCode = pfnRARReadHeader(hArcData, &HeaderData)) == 0)
	{
		int iProcessFileCode = pfnRARProcessFileW(hArcData, (int)RAR_EXTRACT, strDestFolder.GetBuffer(), NULL);
		if (iProcessFileCode != 0)
		{
			OutProcessFileError(iProcessFileCode, strArchivePath);
			break;
		}
	}

	pfnRARCloseArchive(hArcData);

	FreeLibrary(hUnRARDLL);
	AddDebugLogLine(false, _T("Unrar.dll unloaded"));

	if (iReadHeaderCode == ERAR_BAD_DATA)
		return false;
	return true;
}

void Unzip(const CString& strArchivePath, const CString& destfolder, const CString& filetoget, bool& bIsZipFile, bool& bUnzipped, const CString& filetoget2)
{
	CZIPFile zip;
	if(zip.Open(strArchivePath))
	{
		bIsZipFile = true;

		CZIPFile::File* zfile = zip.GetFile(filetoget);	
		if (!zfile && !filetoget2.IsEmpty())
			zfile = zip.GetFile(filetoget2);
		if (zfile)
		{
			TCHAR szTempUnzipFilePath[MAX_PATH];
			_tmakepath(szTempUnzipFilePath, NULL, destfolder, filetoget, _T(".unzip.tmp"));
			if (zfile->Extract(szTempUnzipFilePath))
			{
				const CString strDefaultFilePath = destfolder+filetoget;
				if (_tremove(strDefaultFilePath) != 0)
					TRACE("*** Error: Failed to remove default file \"%s\" - %s\n", strDefaultFilePath, _tcserror(errno));
				if (_trename(szTempUnzipFilePath, strDefaultFilePath) != 0)
					TRACE("*** Error: Failed to rename uncompressed file \"%s\" to default file \"%s\" - %s\n", szTempUnzipFilePath, strDefaultFilePath, _tcserror(errno));
				if (_tremove(strArchivePath) != 0)
					TRACE("*** Error: Failed to remove temporary file \"%s\" - %s\n", strArchivePath, _tcserror(errno));
				bUnzipped = true;
			}
			else
				LogError(LOG_STATUSBAR, _T("Failed to extract \"%s\" from downloaded ZIP file \"%s\"."), filetoget, strArchivePath);

			zip.Close();
		}
		else
			LogError(LOG_STATUSBAR, _T("Downloaded file \"%s\" is a ZIP file with unexpected content."), strArchivePath);		
	}
	else
	{
		CGZIPFile gz;
		if (gz.Open(strArchivePath))
		{
			bIsZipFile = true;

			CString szTempUnzipFilePath;
			_tmakepath(szTempUnzipFilePath.GetBuffer(_MAX_PATH), NULL, destfolder, filetoget, _T(".unzip.tmp"));
			szTempUnzipFilePath.ReleaseBuffer();

			// add filename and extension of uncompressed file to temporary file
			CString strUncompressedFileName = gz.GetUncompressedFileName();
			if (!strUncompressedFileName.IsEmpty())
			{
				szTempUnzipFilePath += _T('.');
				szTempUnzipFilePath += strUncompressedFileName;
			}

			if (gz.Extract(szTempUnzipFilePath))
			{
				const CString strDefaultFilePath = destfolder+filetoget;
				if (_tremove(strDefaultFilePath) != 0)
					TRACE("*** Error: Failed to remove default file \"%s\" - %s\n", strDefaultFilePath, _tcserror(errno));
				if (_trename(szTempUnzipFilePath, strDefaultFilePath) != 0)
					TRACE("*** Error: Failed to rename uncompressed file \"%s\" to default file \"%s\" - %s\n", szTempUnzipFilePath, strDefaultFilePath, _tcserror(errno));
				if (_tremove(strArchivePath) != 0)
					TRACE("*** Error: Failed to remove temporary file \"%s\" - %s\n", strArchivePath, _tcserror(errno));
				bUnzipped = true;
			}
			else
				LogError(LOG_STATUSBAR, _T("Downloaded file \"%s\" is a ZIP file with unexpected content."), strArchivePath);		

			gz.Close();
		}		
	}
}

bool IsRar(const CString& strArchivePath)
{
    FILE* readFile = _tfsopen(strArchivePath, _T("r"), 0x20); // _SH_DENYWR
    if(!readFile) 
        return false; 

    char buffer[4];  
    fgets(buffer,sizeof(buffer),readFile); 
    CString sbuffer = CString(buffer); 
    sbuffer.Trim(); 
    fclose(readFile); 

    return StrStrI(sbuffer, _T("Rar")) != 0; 
}
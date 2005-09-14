/*
the IP to country data is provided by http://ip-to-country.webhosting.info/

"IP2Country uses the IP-to-Country Database
 provided by WebHosting.Info (http://www.webhosting.info),
 available from http://ip-to-country.webhosting.info."

 */

/*

flags are from http://sf.net/projects/flags/

*/

// by Superlexx, based on IPFilter by Bouc7

#include "StdAfx.h"
#include "IP2Country.h"
#include "emule.h"
#include "otherfunctions.h"
#include <flag/resource.h>

//refresh list
#include "serverlist.h"
#include "clientlist.h"

//refresh server list ctrl
#include "emuledlg.h"
#include "serverwnd.h"
#include "serverlistctrl.h"

//Start
#include "kadcontactlistctrl.h"
#include "kademliawnd.h"
//End

//Start
#include "share.h"
#include "HttpDownloadDlg.h"//MORPH - Added by SiRoB, IP2Country auto-updating
#include "ZipFile.h"//MORPH - Added by SiRoB, ZIP File download decompress
//End


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// N/A flag is the first Res, so it should at index zero
#define NO_FLAG 0

CString FirstCharCap(CString target){

	target.TrimRight();//clean out the space at the end, prevent exception for index++
	if(target.IsEmpty()) return _T("");
	target.MakeLower();
	target.SetAt(0, target.Left(1).MakeUpper().GetAt(0));
	for(int index = target.Find(' '); index != -1; index = target.Find(' ', index)){
		index++;//set the character next to space be Upper
		target.SetAt(index, target.Mid(index, 1).MakeUpper().GetAt(0));
	}
	return target;
}

CIP2Country::CIP2Country(){

	m_bRunning = false;

	defaultIP2Country.IPstart = 0;
	defaultIP2Country.IPend = 0;

	defaultIP2Country.ShortCountryName = "N/A";
	defaultIP2Country.MidCountryName = "N/A";
	defaultIP2Country.LongCountryName = "Not Applicable";

	defaultIP2Country.FlagIndex = NO_FLAG;

	EnableIP2Country = false;
	EnableCountryFlag = false;
	
	if(thePrefs.GetIP2CountryNameMode() != IP2CountryName_DISABLE || 
		thePrefs.IsIP2CountryShowFlag()){
		Load();
	}

	AddModLogLine(false, _T("IP2Country uses the IP-to-Country Database provided by WebHosting.Info (http://www.webhosting.info), available from http://ip-to-country.webhosting.info."));

	m_bRunning = true;
}

CIP2Country::~CIP2Country(){

	m_bRunning = false;

	Unload();
}

void CIP2Country::Load(){

	EnableCountryFlag = LoadCountryFlagLib();//flag lib first, so ip range can map to flag
	EnableIP2Country = LoadFromFile();

	if(m_bRunning) Reset();
	
	AddDebugLogLine(false, _T("IP2Country loaded"));
}

void CIP2Country::Unload(){

	EnableIP2Country = false;
	EnableCountryFlag = false;

	if(m_bRunning) Reset();

	RemoveAllIPs();
	RemoveAllFlags();

	AddDebugLogLine(false, _T("IP2Country unloaded"));
}

void CIP2Country::Reset(){
	theApp.serverlist->ResetIP2Country();
	theApp.clientlist->ResetIP2Country();
}

void CIP2Country::Refresh(){
	theApp.emuledlg->serverwnd->serverlistctrl.RefreshAllServer();
	theApp.emuledlg->kademliawnd->RedrawWindow();//KTS � modifier quand list
}

bool CIP2Country::LoadFromFile(){

	char buffer[1024];
	int	lenBuf = 1024;

	LPCTSTR ip2countryCSVfile = thePrefs.GetConfigDir() + _T("ip-to-country.csv");

	FILE* readFile = _tfsopen(ip2countryCSVfile, _T("r"), _SH_DENYWR);

	try{
		if (readFile != NULL) {

			int count = 0;
			bool error = false;

			while (!feof(readFile)) {
				error = false;
				if (fgets(buffer,lenBuf,readFile)==0) break;
				CString	sbuffer;
				sbuffer = buffer;
				/*
				http://ip-to-country.webhosting.info/node/view/54

				This is a sample of how the CSV file is structured:

				"0033996344","0033996351","GB","GBR","UNITED KINGDOM"
				"0050331648","0083886079","US","USA","UNITED STATES"
				"0094585424","0094585439","SE","SWE","SWEDEN"

				FIELD  			DATA TYPE		  	FIELD DESCRIPTION
				IP_FROM 		NUMERICAL (DOUBLE) 	Beginning of IP address range.
				IP_TO			NUMERICAL (DOUBLE) 	Ending of IP address range.
				COUNTRY_CODE2 	CHAR(2)				Two-character country code based on ISO 3166.
				COUNTRY_CODE3 	CHAR(3)				Three-character country code based on ISO 3166.
				COUNTRY_NAME 	VARCHAR(50) 		Country name based on ISO 3166
				*/
				// we assume that the ip-to-country.csv is valid and doesn't cause any troubles
				// get & process IP range
				sbuffer.Remove('"'); // get rid of the " signs
				
				CString tempStr[5];

				int curPos;
				curPos = 0;

				for(int forCount = 0; forCount !=  5; forCount++){
					tempStr[forCount] = sbuffer.Tokenize(_T(","), curPos);
					if(tempStr[forCount].IsEmpty()) {
						if(forCount == 0 || forCount == 1) error = true; //no empty ip field
						//no need to throw an exception, keep reading in next line
						//throw CString(_T("error line in"));
						}
						}
				
				if(error){
					AddModLogLine(false, _T("error line number : %i"), count+1);
					AddModLogLine(false, _T("%s %s"), "possible error line in", ip2countryCSVfile);
						continue;
					}
				//tempStr[4] is full country name, capitalize country name from rayita
				tempStr[4] = FirstCharCap(tempStr[4]);

				count++;
				//Start Unicode
				AddIPRange(_ttoi(tempStr[0]),_ttoi(tempStr[1]), tempStr[2], tempStr[3], tempStr[4]);
				//End Unicode

		}
			fclose(readFile);
		}
		else{
			throw CString(_T("Failed to load in"));
		}
	}
	catch(CString error){
		AddModLogLine(false, _T("%s %s"), error, ip2countryCSVfile);
		RemoveAllIPs();
		return false;
	}
	AddDebugLogLine(false, _T("IP2Countryfile has been loaded"));
	return true;

}

bool CIP2Country::LoadCountryFlagLib(){

	CString ip2countryCountryFlag;

	try{

		//detect windows version
		if(thePrefs.GetWindowsVersion() == _WINVER_XP_){
			//it's XP, we can use beautiful 32bits flags with alpha channel :)
			ip2countryCountryFlag = thePrefs.GetConfigDir()+_T("countryflag32.dll");
		}
		else{
			//oh~ it's not XP, but we still can load the 24bits flags
			ip2countryCountryFlag = thePrefs.GetConfigDir()+_T("countryflag.dll");
		}

		_hCountryFlagDll = LoadLibrary(ip2countryCountryFlag); 
		if (_hCountryFlagDll == NULL) 
		{ 
			throw CString(_T("CountryFlag Disabled, failed to load"));
		} 

		uint16	resID[] = {
			IDI_COUNTRY_FLAG_NOFLAG,//first res in image list should be N/A

			IDI_COUNTRY_FLAG_AD, IDI_COUNTRY_FLAG_AE, IDI_COUNTRY_FLAG_AF, IDI_COUNTRY_FLAG_AG, 
			IDI_COUNTRY_FLAG_AI, IDI_COUNTRY_FLAG_AL, IDI_COUNTRY_FLAG_AM, IDI_COUNTRY_FLAG_AN, 
			IDI_COUNTRY_FLAG_AO, IDI_COUNTRY_FLAG_AR, IDI_COUNTRY_FLAG_AS, IDI_COUNTRY_FLAG_AT, 
			IDI_COUNTRY_FLAG_AU, IDI_COUNTRY_FLAG_AW, IDI_COUNTRY_FLAG_AZ, IDI_COUNTRY_FLAG_BA, 
			IDI_COUNTRY_FLAG_BB, IDI_COUNTRY_FLAG_BD, IDI_COUNTRY_FLAG_BE, IDI_COUNTRY_FLAG_BF, 
			IDI_COUNTRY_FLAG_BG, IDI_COUNTRY_FLAG_BH, IDI_COUNTRY_FLAG_BI, IDI_COUNTRY_FLAG_BJ, 
			IDI_COUNTRY_FLAG_BM, IDI_COUNTRY_FLAG_BN, IDI_COUNTRY_FLAG_BO, IDI_COUNTRY_FLAG_BR, 
			IDI_COUNTRY_FLAG_BS, IDI_COUNTRY_FLAG_BT, IDI_COUNTRY_FLAG_BW, IDI_COUNTRY_FLAG_BY, 
			IDI_COUNTRY_FLAG_BZ, IDI_COUNTRY_FLAG_CA, IDI_COUNTRY_FLAG_CC, IDI_COUNTRY_FLAG_CD, 
			IDI_COUNTRY_FLAG_CF, IDI_COUNTRY_FLAG_CG, IDI_COUNTRY_FLAG_CH, IDI_COUNTRY_FLAG_CI, 
			IDI_COUNTRY_FLAG_CK, IDI_COUNTRY_FLAG_CL, IDI_COUNTRY_FLAG_CM, IDI_COUNTRY_FLAG_CN, 
			IDI_COUNTRY_FLAG_CO, IDI_COUNTRY_FLAG_CR, IDI_COUNTRY_FLAG_CU, IDI_COUNTRY_FLAG_CV, 
			IDI_COUNTRY_FLAG_CX, IDI_COUNTRY_FLAG_CY, IDI_COUNTRY_FLAG_CZ, IDI_COUNTRY_FLAG_DE, 
			IDI_COUNTRY_FLAG_DJ, IDI_COUNTRY_FLAG_DK, IDI_COUNTRY_FLAG_DM, IDI_COUNTRY_FLAG_DO, 
			IDI_COUNTRY_FLAG_DZ, IDI_COUNTRY_FLAG_EC, IDI_COUNTRY_FLAG_EE, IDI_COUNTRY_FLAG_EG, 
			IDI_COUNTRY_FLAG_EH, IDI_COUNTRY_FLAG_ER, IDI_COUNTRY_FLAG_ES, IDI_COUNTRY_FLAG_ET, 
			IDI_COUNTRY_FLAG_FI, IDI_COUNTRY_FLAG_FJ, IDI_COUNTRY_FLAG_FK, IDI_COUNTRY_FLAG_FM, 
			IDI_COUNTRY_FLAG_FO, IDI_COUNTRY_FLAG_FR, IDI_COUNTRY_FLAG_GA, IDI_COUNTRY_FLAG_GB, 
			IDI_COUNTRY_FLAG_GD, IDI_COUNTRY_FLAG_GE, IDI_COUNTRY_FLAG_GG, IDI_COUNTRY_FLAG_GH, 
			IDI_COUNTRY_FLAG_GI, IDI_COUNTRY_FLAG_GK, IDI_COUNTRY_FLAG_GL, IDI_COUNTRY_FLAG_GM, 
			IDI_COUNTRY_FLAG_GN, IDI_COUNTRY_FLAG_GP, IDI_COUNTRY_FLAG_GQ, IDI_COUNTRY_FLAG_GR, 
			IDI_COUNTRY_FLAG_GS, IDI_COUNTRY_FLAG_GT, IDI_COUNTRY_FLAG_GU, IDI_COUNTRY_FLAG_GW, 
			IDI_COUNTRY_FLAG_GY, IDI_COUNTRY_FLAG_HK, IDI_COUNTRY_FLAG_HN, IDI_COUNTRY_FLAG_HR, 
			IDI_COUNTRY_FLAG_HT, IDI_COUNTRY_FLAG_HU, IDI_COUNTRY_FLAG_ID, IDI_COUNTRY_FLAG_IE, 
			IDI_COUNTRY_FLAG_IL, IDI_COUNTRY_FLAG_IM, IDI_COUNTRY_FLAG_IN, IDI_COUNTRY_FLAG_IO, 
			IDI_COUNTRY_FLAG_IQ, IDI_COUNTRY_FLAG_IR, IDI_COUNTRY_FLAG_IS, IDI_COUNTRY_FLAG_IT, 
			IDI_COUNTRY_FLAG_JE, IDI_COUNTRY_FLAG_JM, IDI_COUNTRY_FLAG_JO, IDI_COUNTRY_FLAG_JP, 
			IDI_COUNTRY_FLAG_KE, IDI_COUNTRY_FLAG_KG, IDI_COUNTRY_FLAG_KH, IDI_COUNTRY_FLAG_KI, 
			IDI_COUNTRY_FLAG_KM, IDI_COUNTRY_FLAG_KN, IDI_COUNTRY_FLAG_KP, IDI_COUNTRY_FLAG_KR, 
			IDI_COUNTRY_FLAG_KW, IDI_COUNTRY_FLAG_KY, IDI_COUNTRY_FLAG_KZ, IDI_COUNTRY_FLAG_LA, 
			IDI_COUNTRY_FLAG_LB, IDI_COUNTRY_FLAG_LC, IDI_COUNTRY_FLAG_LI, IDI_COUNTRY_FLAG_LK, 
			IDI_COUNTRY_FLAG_LR, IDI_COUNTRY_FLAG_LS, IDI_COUNTRY_FLAG_LT, IDI_COUNTRY_FLAG_LU, 
			IDI_COUNTRY_FLAG_LV, IDI_COUNTRY_FLAG_LY, IDI_COUNTRY_FLAG_MA, IDI_COUNTRY_FLAG_MC, 
			IDI_COUNTRY_FLAG_MD, IDI_COUNTRY_FLAG_MG, IDI_COUNTRY_FLAG_MH, IDI_COUNTRY_FLAG_MK, 
			IDI_COUNTRY_FLAG_ML, IDI_COUNTRY_FLAG_MM, IDI_COUNTRY_FLAG_MN, IDI_COUNTRY_FLAG_MO, 
			IDI_COUNTRY_FLAG_MP, IDI_COUNTRY_FLAG_MQ, IDI_COUNTRY_FLAG_MR, IDI_COUNTRY_FLAG_MS, 
			IDI_COUNTRY_FLAG_MT, IDI_COUNTRY_FLAG_MU, IDI_COUNTRY_FLAG_MV, IDI_COUNTRY_FLAG_MW, 
			IDI_COUNTRY_FLAG_MX, IDI_COUNTRY_FLAG_MY, IDI_COUNTRY_FLAG_MZ, IDI_COUNTRY_FLAG_NA, 
			IDI_COUNTRY_FLAG_NC, IDI_COUNTRY_FLAG_NE, IDI_COUNTRY_FLAG_NF, IDI_COUNTRY_FLAG_NG, 
			IDI_COUNTRY_FLAG_NI, IDI_COUNTRY_FLAG_NL, IDI_COUNTRY_FLAG_NO, IDI_COUNTRY_FLAG_NP, 
			IDI_COUNTRY_FLAG_NR, IDI_COUNTRY_FLAG_NU, IDI_COUNTRY_FLAG_NZ, IDI_COUNTRY_FLAG_OM, 
			IDI_COUNTRY_FLAG_PA, IDI_COUNTRY_FLAG_PC, IDI_COUNTRY_FLAG_PE, IDI_COUNTRY_FLAG_PF, 
			IDI_COUNTRY_FLAG_PG, IDI_COUNTRY_FLAG_PH, IDI_COUNTRY_FLAG_PK, IDI_COUNTRY_FLAG_PL, 
			IDI_COUNTRY_FLAG_PM, IDI_COUNTRY_FLAG_PN, IDI_COUNTRY_FLAG_PR, IDI_COUNTRY_FLAG_PS, 
			IDI_COUNTRY_FLAG_PT, IDI_COUNTRY_FLAG_PW, IDI_COUNTRY_FLAG_PY, IDI_COUNTRY_FLAG_QA, 
			IDI_COUNTRY_FLAG_RO, IDI_COUNTRY_FLAG_RU, IDI_COUNTRY_FLAG_RW, IDI_COUNTRY_FLAG_SA, 
			IDI_COUNTRY_FLAG_SB, IDI_COUNTRY_FLAG_SC, IDI_COUNTRY_FLAG_SD, IDI_COUNTRY_FLAG_SE, 
			IDI_COUNTRY_FLAG_SG, IDI_COUNTRY_FLAG_SH, IDI_COUNTRY_FLAG_SI, IDI_COUNTRY_FLAG_SK, 
			IDI_COUNTRY_FLAG_SL, IDI_COUNTRY_FLAG_SM, IDI_COUNTRY_FLAG_SN, IDI_COUNTRY_FLAG_SO, 
			IDI_COUNTRY_FLAG_SR, IDI_COUNTRY_FLAG_ST, IDI_COUNTRY_FLAG_SU, IDI_COUNTRY_FLAG_SV, 
			IDI_COUNTRY_FLAG_SY, IDI_COUNTRY_FLAG_SZ, IDI_COUNTRY_FLAG_TC, IDI_COUNTRY_FLAG_TD, 
			IDI_COUNTRY_FLAG_TF, IDI_COUNTRY_FLAG_TG, IDI_COUNTRY_FLAG_TH, IDI_COUNTRY_FLAG_TJ, 
			IDI_COUNTRY_FLAG_TK, IDI_COUNTRY_FLAG_TL, IDI_COUNTRY_FLAG_TM, IDI_COUNTRY_FLAG_TN, 
			IDI_COUNTRY_FLAG_TO, IDI_COUNTRY_FLAG_TR, IDI_COUNTRY_FLAG_TT, IDI_COUNTRY_FLAG_TV, 
			IDI_COUNTRY_FLAG_TW, IDI_COUNTRY_FLAG_TZ, IDI_COUNTRY_FLAG_UA, IDI_COUNTRY_FLAG_UG, 
			IDI_COUNTRY_FLAG_UM, IDI_COUNTRY_FLAG_US, IDI_COUNTRY_FLAG_UY, IDI_COUNTRY_FLAG_UZ, 
			IDI_COUNTRY_FLAG_VA, IDI_COUNTRY_FLAG_VC, IDI_COUNTRY_FLAG_VE, IDI_COUNTRY_FLAG_VG, 
			IDI_COUNTRY_FLAG_VI, IDI_COUNTRY_FLAG_VN, IDI_COUNTRY_FLAG_VU, IDI_COUNTRY_FLAG_WF, 
			IDI_COUNTRY_FLAG_WS, IDI_COUNTRY_FLAG_YE, IDI_COUNTRY_FLAG_YU, IDI_COUNTRY_FLAG_ZA, 
			IDI_COUNTRY_FLAG_ZM, IDI_COUNTRY_FLAG_ZW, 
			IDI_COUNTRY_FLAG_UK, //by tharghan
			IDI_COUNTRY_FLAG_CS, //by propaganda
			IDI_COUNTRY_FLAG_TP, //by commander

			65535//the end
		};

		CString countryID[] = {
			_T("N/A"),//first res in image list should be N/A

			_T("AD"), _T("AE"), _T("AF"), _T("AG"), _T("AI"), _T("AL"), _T("AM"), _T("AN"), _T("AO"), _T("AR"), _T("AS"), _T("AT"), _T("AU"), _T("AW"), _T("AZ"), 
			_T("BA"), _T("BB"), _T("BD"), _T("BE"), _T("BF"), _T("BG"), _T("BH"), _T("BI"), _T("BJ"), _T("BM"), _T("BN"), _T("BO"), _T("BR"), _T("BS"), _T("BT"), 
			_T("BW"), _T("BY"), _T("BZ"), _T("CA"), _T("CC"), _T("CD"), _T("CF"), _T("CG"), _T("CH"), _T("CI"), _T("CK"), _T("CL"), _T("CM"), _T("CN"), _T("CO"), 
			_T("CR"), _T("CU"), _T("CV"), _T("CX"), _T("CY"), _T("CZ"), _T("DE"), _T("DJ"), _T("DK"), _T("DM"), _T("DO"), _T("DZ"), _T("EC"), _T("EE"), _T("EG"), 
			_T("EH"), _T("ER"), _T("ES"), _T("ET"), _T("FI"), _T("FJ"), _T("FK"), _T("FM"), _T("FO"), _T("FR"), _T("GA"), _T("GB"), _T("GD"), _T("GE"), _T("GG"), 
			_T("GH"), _T("GI"), _T("GK"), _T("GL"), _T("GM"), _T("GN"), _T("GP"), _T("GQ"), _T("GR"), _T("GS"), _T("GT"), _T("GU"), _T("GW"), _T("GY"), _T("HK"), 
			_T("HN"), _T("HR"), _T("HT"), _T("HU"), _T("ID"), _T("IE"), _T("IL"), _T("IM"), _T("IN"), _T("IO"), _T("IQ"), _T("IR"), _T("IS"), _T("IT"), _T("JE"), 
			_T("JM"), _T("JO"), _T("JP"), _T("KE"), _T("KG"), _T("KH"), _T("KI"), _T("KM"), _T("KN"), _T("KP"), _T("KR"), _T("KW"), _T("KY"), _T("KZ"), _T("LA"), 
			_T("LB"), _T("LC"), _T("LI"), _T("LK"), _T("LR"), _T("LS"), _T("LT"), _T("LU"), _T("LV"), _T("LY"), _T("MA"), _T("MC"), _T("MD"), _T("MG"), _T("MH"), 
			_T("MK"), _T("ML"), _T("MM"), _T("MN"), _T("MO"), _T("MP"), _T("MQ"), _T("MR"), _T("MS"), _T("MT"), _T("MU"), _T("MV"), _T("MW"), _T("MX"), _T("MY"), 
			_T("MZ"), _T("NA"), _T("NC"), _T("NE"), _T("NF"), _T("NG"), _T("NI"), _T("NL"), _T("NO"), _T("NP"), _T("NR"), _T("NU"), _T("NZ"), _T("OM"), _T("PA"), 
			_T("PC"), _T("PE"), _T("PF"), _T("PG"), _T("PH"), _T("PK"), _T("PL"), _T("PM"), _T("PN"), _T("PR"), _T("PS"), _T("PT"), _T("PW"), _T("PY"), _T("QA"), 
			_T("RO"), _T("RU"), _T("RW"), _T("SA"), _T("SB"), _T("SC"), _T("SD"), _T("SE"), _T("SG"), _T("SH"), _T("SI"), _T("SK"), _T("SL"), _T("SM"), _T("SN"), 
			_T("SO"), _T("SR"), _T("ST"), _T("SU"), _T("SV"), _T("SY"), _T("SZ"), _T("TC"), _T("TD"), _T("TF"), _T("TG"), _T("TH"), _T("TJ"), _T("TK"), _T("TL"), 
			_T("TM"), _T("TN"), _T("TO"), _T("TR"), _T("TT"), _T("TV"), _T("TW"), _T("TZ"), _T("UA"), _T("UG"), _T("UM"), _T("US"), _T("UY"), _T("UZ"), _T("VA"), 
			_T("VC"), _T("VE"), _T("VG"), _T("VI"), _T("VN"), _T("VU"), _T("WF"), _T("WS"), _T("YE"), _T("YU"), _T("ZA"), _T("ZM"), _T("ZW"), 
			_T("UK"), //by tharghan
			_T("CS"), //by propaganda
			_T("TP") //by commander
		};

		HICON iconHandle;

		CountryFlagImageList.DeleteImageList();
		CountryFlagImageList.Create(18,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
		CountryFlagImageList.SetBkColor(CLR_NONE);

		//the res Array have one element to be the STOP
		for(int cur_pos = 0; resID[cur_pos] != 65535; cur_pos++){

			CountryIDtoFlagIndex.SetAt(countryID[cur_pos], cur_pos);

			iconHandle = LoadIcon(_hCountryFlagDll, MAKEINTRESOURCE(resID[cur_pos]));
			if(iconHandle == NULL) throw CString(_T("Invalid resID, maybe you need to upgrade your flag icon Dll,"));
			
			CountryFlagImageList.Add(iconHandle);
		}
	

	}
	catch(CString error){
		AddModLogLine(false, _T("%s in %s"), error, ip2countryCountryFlag);
		RemoveAllFlags();
		//free lib
		if(_hCountryFlagDll != NULL) FreeLibrary(_hCountryFlagDll);
		return false;
	}

	//free lib
	if(_hCountryFlagDll != NULL) FreeLibrary(_hCountryFlagDll);

	AddDebugLogLine(false, _T("Country Flags have been loaded"));
	return true;

}

void CIP2Country::RemoveAllIPs(){

	uint32 key;
	IPRange_Struct2* value;
	POSITION pos1;
	for(POSITION pos = iplist.GetHeadPosition(); pos1 = pos; )
	{
		iplist.GetNextAssoc(pos, key, value);
		delete value;
		iplist.RemoveAt(pos1);
	}
	iplist.RemoveAll();

	AddDebugLogLine(false, _T("IP2Countryfile has been unloaded"));
}

void CIP2Country::RemoveAllFlags(){

	//destory all image
	CountryFlagImageList.DeleteImageList();

	//also clean out the map table
	CountryIDtoFlagIndex.RemoveAll();

	AddDebugLogLine(false, _T("Country Flags have been unloaded"));
}

bool CIP2Country::AddIPRange(uint32 IPfrom,uint32 IPto, CString shortCountryName, CString midCountryName, CString longCountryName){
	IPRange_Struct2* newRange = new IPRange_Struct2();

	newRange->IPstart = IPfrom;
	newRange->IPend = IPto;
	newRange->ShortCountryName = shortCountryName;
	newRange->MidCountryName = midCountryName;
	newRange->LongCountryName = longCountryName;

	if(EnableCountryFlag){

		const CRBMap<CString, uint16>::CPair* pair;
		pair = CountryIDtoFlagIndex.Lookup(shortCountryName);

		if(pair != NULL){
			newRange->FlagIndex = pair->m_value;
		}
		else{
			newRange->FlagIndex = NO_FLAG;
		}
	}
	else{
		//this valuse is useless if the country flag havn't been load up, should be safe I think ...
		//newRange->FlagIndex = 0;
	}
	
	iplist.SetAt(IPfrom, newRange);
	return true;
}

struct IPRange_Struct2* CIP2Country::GetCountryFromIP(uint32 ClientIP){

	if(EnableIP2Country == false){
		return &defaultIP2Country;
	}
	else if (ClientIP == 0){
		//AddDebugLogLine(false, "CIP2Country::GetCountryFromIP doesn't have ip to search for");
		return &defaultIP2Country;
	}
	else if(iplist.IsEmpty()){
		AddDebugLogLine(false, _T("CIP2Country::GetCountryFromIP iplist doesn't exist"));
		return &defaultIP2Country;
	}

	ClientIP = htonl(ClientIP);
	POSITION pos = iplist.FindFirstKeyAfter(ClientIP);
	if(!pos){
		pos = iplist.GetTailPosition();
}
	else{
		iplist.GetPrev(pos);
		}

	while(pos){
		const CRBMap<uint32, IPRange_Struct2*>::CPair* pair = iplist.GetPrev(pos);

		if (ClientIP > pair->m_value->IPend) break;
		if (ClientIP >= pair->m_key && ClientIP <= pair->m_value->IPend) return pair->m_value;
	}
	return &defaultIP2Country;
}

bool CIP2Country::ShowCountryFlag(){

	return 
		//user wanna see flag,
		(thePrefs.IsIP2CountryShowFlag() && 
		//flag have been loaded
		EnableCountryFlag && 
		//ip table have been loaded
		EnableIP2Country);
}

//EastShare End - added by AndCycle, IP to Country

//Commander - Added: IP2Country auto-updating - Start
void CIP2Country::UpdateIP2CountryURL()
{   
	CString sbuffer;
	CString strURL = thePrefs.GetUpdateURLIP2Country();
	TCHAR szTempFilePath[_MAX_PATH];
	_tmakepath(szTempFilePath, NULL, thePrefs.GetConfigDir(), DFLT_IP2COUNTRY_FILENAME, _T("tmp"));

	CHttpDownloadDlg dlgDownload;
	dlgDownload.m_strTitle = _T("Downloading IP2Country version file");
	dlgDownload.m_sURLToDownload = strURL;
	dlgDownload.m_sFileToDownloadInto = szTempFilePath;
	SYSTEMTIME SysTime;
	if (PathFileExists(GetDefaultFilePath()))
		memcpy(&SysTime, thePrefs.GetIP2CountryVersion(), sizeof(SYSTEMTIME));
	else
		memset(&SysTime, 0, sizeof(SYSTEMTIME));
	dlgDownload.m_pLastModifiedTime = &SysTime;

		if (dlgDownload.DoModal() != IDOK)
		{
			_tremove(szTempFilePath);
		AddModLogLine(true, _T("Error downloading %s"), strURL);
			return;
		}
    if (dlgDownload.m_pLastModifiedTime == NULL)
			return;
	
        
		bool bIsZipFile = false;
		bool bUnzipped = false;
		CZIPFile zip;
		if (zip.Open(szTempFilePath))
		{
			bIsZipFile = true;

			CZIPFile::File* zfile = zip.GetFile(DFLT_IP2COUNTRY_FILENAME); // It has to be a zip-file which includes a file called: ip-to-country.csv
			if (zfile)
			{
			CString strTempUnzipFilePath;
			_tmakepath(strTempUnzipFilePath.GetBuffer(_MAX_PATH), NULL, thePrefs.GetConfigDir(), DFLT_IP2COUNTRY_FILENAME, _T(".unzip.tmp"));
			strTempUnzipFilePath.ReleaseBuffer();
			if (zfile->Extract(strTempUnzipFilePath))
				{
					zip.Close();
					zfile = NULL;

					if (_tremove(GetDefaultFilePath()) != 0)
						TRACE("*** Error: Failed to remove default IP to Country file \"%s\" - %s\n", GetDefaultFilePath(), _tcserror(errno));
				if (_trename(strTempUnzipFilePath, GetDefaultFilePath()) != 0)
					TRACE("*** Error: Failed to rename uncompressed IP to Country file \"%s\" to default IP to Country file \"%s\" - %s\n", strTempUnzipFilePath, GetDefaultFilePath(), _tcserror(errno));
					if (_tremove(szTempFilePath) != 0)
						TRACE("*** Error: Failed to remove temporary IP to Country file \"%s\" - %s\n", szTempFilePath, _tcserror(errno));
					bUnzipped = true;
				}
				else
					LogError(LOG_STATUSBAR, _T("Failed to extract IP to Country file from downloaded IP to Country ZIP file \"%s\"."), szTempFilePath);
			}
			else
				LogError(LOG_STATUSBAR, _T("Downloaded IP to Country file \"%s\" is a ZIP file with unexpected content."), szTempFilePath); //File not found inside the zip-file

			zip.Close();
		}
        
		if (!bIsZipFile && !bUnzipped)
		{
			_tremove(GetDefaultFilePath());
			_trename(szTempFilePath, GetDefaultFilePath());
		}

	//Moved up to not retry if archive don't contain the awaited file
	memcpy(thePrefs.GetIP2CountryVersion(), &SysTime, sizeof SysTime);
	thePrefs.Save();

		if(bIsZipFile && !bUnzipped){
			return;
		}

		if(thePrefs.GetIP2CountryNameMode() != IP2CountryName_DISABLE || thePrefs.IsIP2CountryShowFlag()){
			theApp.ip2country->Unload();
			AddModLogLine(false,_T("IP2Country.csv unloaded due to update in progress"));
			theApp.ip2country->Load();
			AddModLogLine(false,_T("IP2Country.csv loaded after successful update"));
	}
}
//Commander - Added: IP2Country auto-updating - End

CString CIP2Country::GetDefaultFilePath() const
{
	return thePrefs.GetConfigDir() + DFLT_IP2COUNTRY_FILENAME;
}
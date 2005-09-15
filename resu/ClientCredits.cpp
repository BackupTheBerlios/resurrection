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
#include "stdafx.h"
#include <math.h>
#include "emule.h"
#include "ClientCredits.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "Opcodes.h"
#include "Sockets.h"
#pragma warning(disable:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#include <crypto.v52.1/base64.h>
#include <crypto.v52.1/osrng.h>
#include <crypto.v52.1/files.h>
#include <crypto.v52.1/sha.h>
#pragma warning(default:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#include "emuledlg.h"
#include "Log.h"
// <CB Mod : Credits>
#include "KnownFile.h"
#include "SharedFileList.h"
#include "UploadQueue.h"
#include "UpDownClient.h"
#include "ClientList.h"
// </CB Mod : Credits>



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CLIENTS_MET_FILENAME	_T("clients.met")

CClientCredits::CClientCredits(CreditStruct* in_credits)
{
	m_pCredits = in_credits;
	InitalizeIdent();
	m_dwUnSecureWaitTime = 0;
	m_dwSecureWaitTime = 0;
	m_dwWaitTimeIP = 0;
	// Creditsystem
	lastscore=1.0f; //lovelace CreditSys
	lastscoretime=0; //lovelace CreditSys
	// Creditsystem
}

CClientCredits::CClientCredits(const uchar* key)
{
	m_pCredits = new CreditStruct;
	MEMSET(m_pCredits, 0, sizeof(CreditStruct));
	md4cpy(m_pCredits->abyKey, key);
	InitalizeIdent();
	m_dwUnSecureWaitTime = ::GetTickCount();
	m_dwSecureWaitTime = ::GetTickCount();
	m_dwWaitTimeIP = 0;
// Credit System
	lastscore=1.0f; //lovelace CreditSys
	lastscoretime=0; //lovelace CreditSys
	// Credit System
}

CClientCredits::~CClientCredits()
{
	delete m_pCredits;
}

void CClientCredits::AddDownloaded(uint32 bytes, uint32 dwForIP) {
	if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
		return;
	}
	//encode
	uint64 current=m_pCredits->nDownloadedHi;
	current=(current<<32)+ m_pCredits->nDownloadedLo + bytes ;

	//recode
	m_pCredits->nDownloadedLo=(uint32)current;
	m_pCredits->nDownloadedHi=(uint32)(current>>32);
}

void CClientCredits::AddUploaded(uint32 bytes, uint32 dwForIP) {
	if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
		return;
	}
	//encode
	uint64 current=m_pCredits->nUploadedHi;
	current=(current<<32)+ m_pCredits->nUploadedLo + bytes ;

	//recode
	m_pCredits->nUploadedLo=(uint32)current;
	m_pCredits->nUploadedHi=(uint32)(current>>32);
}
// Maella -Code Improvement-
// Remark: This method is used very often......
// Credit System
float CClientCredits::GetScoreRatio(uint32 dwForIP) // [TPT] - not const due to lovelace
{
	float result = 0.0f;	
	int temp = thePrefs.GetCreditSystem();
	switch (temp)
{
        case 0: {
		// This is OFFICIAL
		// check the client ident status
		if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
			// bad guy - no credits for you
			return 1.0f;
		}

		// Cache value
		const uint64 downloadTotal = GetDownloadedTotal();

		// Check if this client has any credit (sent >1MB)
		if(downloadTotal < 1000000)
			return 1.0f;

		// Cache value
		const uint64 uploadTotal = GetUploadedTotal();

		// Factor 1
		float result = (uploadTotal == 0) ?
					10.0f : (float)(2*downloadTotal)/(float)uploadTotal;

		// Factor 2
		float trunk = (float)sqrt(2.0 + (double)downloadTotal/1048576.0);
		if(result > trunk)
			result = trunk;

		// Trunk final result 1..10
		if(result < 1.0f)
			return 1.0f;
		else if(result > 10.0f)
			return 10.0f;
		else
			return result;
		break;
	}
		// EastShare - Added by linekin, lovelace Credit
		case 1:
                {
			// check the client ident status
			if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
				// bad guy - no credits for you
				return 0.0f;//lovelace
			}
                        // new creditsystem by [lovelace]
			double cl_up,cl_down; 
			result = 0.0f;
			if (this->m_pCredits)
			{
				if (lastscoretime>GetTickCount()) 
					return lastscore; 

			cl_up = GetUploadedTotal()/(double)1048576;
			cl_down = GetDownloadedTotal()/(double)1048576;
			result=(float)(3.0 * cl_down * cl_down - cl_up * cl_up);

			if (fabs(result)>20000.0f) 
				result*=20000.0f/fabs(result);

			result=100.0f*powf((float)(1-1/(1.0f+expf(result*0.001))),6.6667f);

			if (result<0.1f) 
				result=0.1f;

				if (((m_pCredits->nKeySize == 0)
					|| (GetCurrentIdentState(dwForIP)!=IS_IDENTIFIED))&&(result>10.0f)) 
					result=10.0f;

				lastscoretime= ::GetTickCount()+120000;
				lastscore=result;
				return (result);
}

			return 0.1f;
}
                break;

		// Peace - Added by Jmijie, Peace Credit
		case 2:{
			// new creditsystem by [Jmijie]
				if (theApp.clientcredits->CryptoAvailable())
		switch(GetCurrentIdentState(dwForIP))
		{
			case IS_IDFAILED:
			case IS_IDBADGUY:
			case IS_IDNEEDED:
				return 1.0f; // bad guy - no credits for you
}
 // END rayita

	uint64 DownloadedTotal = GetDownloadedTotal();

	if (DownloadedTotal < 1000000)
		return 1.0f;

	uint64 UploadedTotal = GetUploadedTotal();

	float result = !UploadedTotal ? 10.0f : (float)(2*DownloadedTotal/(float)UploadedTotal);

	float result2 = (float)sqrt(2.0 + (double)DownloadedTotal/1048576.0);

	if (result > result2)
		result = result2;

	return result < 1.0f ? 1.0f : result > 10.0f ? 10.0f : result;
			// end new creditsystem by [Jmijie]
		}break;

		// Sivka - Added by Jmijie, Sivka Credit
		case 3:{
		// new creditsystem by [Jmijie]
		switch(GetCurrentIdentState(dwForIP)){
		case IS_IDNEEDED: if(theApp.clientcredits->CryptoAvailable()) return 0.75f;
		case IS_IDFAILED: return 0.5f;
		case IS_IDBADGUY:
		default: return 0.0f;
}

	if(GetDownloadedTotal() > GetUploadedTotal())
{
		const uint64 diffTransfer = GetDownloadedTotal() - GetUploadedTotal() + 1048576;

		if( diffTransfer >= 1073741824 ) // >= 1024MB
			return 32.0f;
		
		return sqrtf((float)diffTransfer/1048576.0f);
	}
	else
		return 1.0f;
			// end new creditsystem by [Jmijie]
		}break;

		// RT - Added by Jmijie, RT Credit
		case 4:{
			// new creditsystem by [Jmijie]
// check the client ident status
            if ( theApp.clientcredits->CryptoAvailable() ){
                    switch( GetCurrentIdentState(dwForIP) ){
                            case IS_IDFAILED:
                            case IS_IDNEEDED:
                            case IS_IDBADGUY:
                            return 0;
                    }
            }
    
            float UploadedTotalMB = (float)GetUploadedTotal() / 1048576.0;
            float DownloadedTotalMB = (float)GetDownloadedTotal() / 1048576.0;
            if (DownloadedTotalMB <= 1){
                    if (UploadedTotalMB <= 1)
                    return 1;
                    else
                    return (float)(1 / sqrt((double)UploadedTotalMB) );
            }
    
            if (UploadedTotalMB > 1){
                    float Basic = (float)sqrt( (double)(DownloadedTotalMB + 1) );
                    if (DownloadedTotalMB > UploadedTotalMB){
                            return ( Basic + (float)sqrt((double)(DownloadedTotalMB - UploadedTotalMB)) );
                    }
                    else{
                            if ( (UploadedTotalMB - DownloadedTotalMB) <= 1 )   return Basic;
                            float Result = ( Basic / (float)sqrt((double)(UploadedTotalMB - DownloadedTotalMB)) );
                            if (DownloadedTotalMB >= 9){
                                    float Lowest = 0.7f + (Basic / 10);
                                    if (Result < Lowest)   Result = Lowest;
                            }
                            else{
                                    if (Result < 1)   Result = DownloadedTotalMB / 9;
                            }
                            /*RT only
                            if ( (thePrefs.GetMaxCredit1Slot() > 0) && (Result > 1) ){
                            if ( (UploadedTotalMB - DownloadedTotalMB) > (Basic * 2) )   Result = 1;
                    }*/
                            return Result;
                    }
            }
            else
            return DownloadedTotalMB; 
			// end new creditsystem by [Jmijie]
		}break;

		// EastShare - Added by Jmijie, S.W.A.T Credit
		case 5:{
			// new creditsystem by [Jmijie]
	// check the client ident status
	if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
		// bad guy - no credits for you
		return 1;
	}

//	if (GetDownloadedTotal() < 1000000)
	if (GetDownloadedTotal() < 1048576) //pcsl999
		return 1;
	float result = 0;
	if (!GetUploadedTotal())
		result = 10;
	else
//		result = (float)(((double)GetDownloadedTotal()*2.0)/(double)GetUploadedTotal());
		result = (float)(((double)GetDownloadedTotal()*2.2)/(double)GetUploadedTotal()); //pcsl999
	float result2 = 0;
	result2 = (float)GetDownloadedTotal()/1048576.0;
	result2 += 2;
	result2 = (double)sqrt((double)result2);

	if (result > result2)
		result = result2;

	if (result < 1)
		return 1;
	else if (result > 100) //pcsl999
		return 100; //pcsl999
	return result;
			// end new creditsystem by [Jmijie]
		}break;

		//EastShare Start - added by AndCycle, Pawcio credit
		case 6:{	
			//Pawcio: Credits
			if ((GetDownloadedTotal() < 1000000)&&(GetUploadedTotal() > 1000000)){
				result = 1.0f;
				break;
			}
			else if ((GetDownloadedTotal() < 1000000)&&(GetUploadedTotal()<1000000)) {
				result = 3.0f;
				break;
			}
			result = 0;
			if (GetUploadedTotal()<1000000)
				result = 10.0f * GetDownloadedTotal()/1000000.0f;
			else
				result = (float)(GetDownloadedTotal()*3)/GetUploadedTotal();
			if ((GetDownloadedTotal() > 100000000)&&(GetUploadedTotal()<GetDownloadedTotal()+8000000)&&(result<50)) result=50;
			else if ((GetDownloadedTotal() > 50000000)&&(GetUploadedTotal()<GetDownloadedTotal()+5000000)&&(result<25)) result=25;
			else if ((GetDownloadedTotal() > 25000000)&&(GetUploadedTotal()<GetDownloadedTotal()+3000000)&&(result<12)) result=12;
			else if ((GetDownloadedTotal() > 10000000)&&(GetUploadedTotal()<GetDownloadedTotal()+2000000)&&(result<5)) result=5;
			if (result > 100.0f){
				result = 100.0f;
				break;
			}
			if (result < 1.0f){
				result = 1.0f;
				break;
			}
		}break;
		//EastShare End - added by AndCycle, Pawcio credit

		// EastShare Credits
		case 7:
			{
			result = (IdentState == IS_NOTAVAILABLE) ? 80 : 100;
			
			result += (float)((double)GetDownloadedTotal()/174762.67 - (double)GetUploadedTotal()/524288); //Modefied by Pretender - 20040120
			
			if ((double)GetDownloadedTotal() > 1048576) {
				result += 100; 
				if (result<50 && ((double)GetDownloadedTotal()*10 > (double)GetUploadedTotal())) result=50;
				} //Modefied by Pretender - 20040330

			if ( result < 10 ) {
				result = 10;
			}else if ( result > 5000 ) {
				result = 5000;
			}
			result = result / 100;

		}break;
		// EastShare END - Added by TAHO, new Credit System
//Add by Spe64 > Fine Credit --->CiccioBastardo
		case 8:
			{
/* With this type of credits attribution I'm only penalizing those clients that do not upload enough only for non complete files.
				For complete files there is no credits evaluation.
				This menagement is "symmetric", as two clients using it will both not get penalized if the simple rules are respected.
				Moreover unlike the official and the other modification I've seen so far, there's no way to exploit it for credit shaping.
				That is, you get nothing more than what you deserves by waiting in the queue if you upload enough. Uploading more gives you
				no advantages. This prevents using tactics like powersharing unfinished files to get more credits. No tit-for-tat possible.
				Respecting the rules makes the clients competing on the same basis as newbies or those asking for complete files.
				In short, you are always neutral, and fairly treated, untill you are not able to keep U-D gap below the maximum limit possible
				and start being evil. The bigger the gap, the worse the treatment (but you can always wait enough in the queue to download
				something more and make you position still worse).
				There are no promoted good clients as in other CS which can be exploited for that. Nor there's a newbie boost which can be 
				exploited as well by changing the hash every session.

				Moreover the equation is based on an absolute difference so there is no the trick to upload X and be able to download n*X 
				before things start even out, as in the official CS.

				Doubts remain on how to treat clients with no SUI. For now a small penalty is applied.
			*/
			#define UP_DOWN_GAP_LIMIT (4*PARTSIZE)

			CUpDownClient* client = theApp.clientlist->FindClientByIP(dwForIP);
			CKnownFile *currequpfile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());

			if(!currequpfile)
				return 1.0;

			float result = 1.0F;

			if ( (( GetCurrentIdentState(dwForIP) != IS_IDENTIFIED) && theApp.clientcredits->CryptoAvailable()) 
					|| GetCurrentIdentState(dwForIP) == IDS_IDENTNOSUPPORT )
					// unsecure - small disavantage
					result = 0.75F; 

			// only for not complete files
			if (currequpfile->IsPartFile()) {
				int diff = GetUploadedTotal() - GetDownloadedTotal();
				if (diff > UP_DOWN_GAP_LIMIT) {
					result  *= (float)(UP_DOWN_GAP_LIMIT)/diff; // This is surely smaller than 1
					result *= result;
				}
			}
			return result;
			}	break;

// </CB Mod : Credits
		//Add by Spe64
		
		case 9:
			{//TK4 credit formula. Aims punish leechers quickly but be fair to those we want nothing from and reward sharers
		 float result = 10.0F;
		 //if SUI failed then credit starts at 10 as for everyone else but will not go up
		 if((GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable()){
			  CUpDownClient* pClient = theApp.clientlist->FindClientByIP(dwForIP);//Get 'client' so we can get file info
			  float dOwnloadedSessionTotal = (float)pClient->GetTransferredDown();
			  float uPloadedSessionTotal = (float)pClient->GetTransferredUp();
			  float allowance = dOwnloadedSessionTotal/4.0F;
			  if(uPloadedSessionTotal > (float)(dOwnloadedSessionTotal + allowance + 1048576.0F)){
				 CKnownFile* file = theApp.sharedfiles->GetFileByID(pClient->GetUploadFileID());
			     if(file!=NULL){//Are they requesting a file? NULL can be produced when client details calls getscoreratio() without this line eMule will crash.
					            if(file->IsPartFile()){//It's a file we are trying to obtain so we want to give to givers so we may get the file quicker.
							       float MbSqd =sqrt((float)(uPloadedSessionTotal-(dOwnloadedSessionTotal + allowance))/1048576.0F);
							       if(MbSqd > 9.0F) result = 9.0F / MbSqd;  //above 81mb values 1 - 0 9/(9 - x)
							         else		 result = 10.0F - MbSqd; //for the first 81Mb (10 -(0-9))
							      }

				}
			  } 
			return result; //partfile 10 - 0.14 complete 10
		    }
		//float is 1e38 it should be sufficient given 1 Gig is 1e9 hence 1000G is 1e12....
        float dOwnloadedTotal = (float)GetDownloadedTotal();//(Given to us)
		float uPloadedTotal = (float)GetUploadedTotal(); //(Taken from us)
		/* Base allowance for a client that has given us nothing is 1Mb
		   But if someone has give us 100Mb and take 130Mb they should not be penalized as someone who has give 0Mb and taken 30Mb?
		   So if you've given 100Mb and taken 130Mb you will only be penalized for 5Mb*/
		float allowance = dOwnloadedTotal/4.0F; //reward uploaders with 1 Mb allowance for every 4Mb uploaded over what they have uploaded.
		if(uPloadedTotal>(float)(dOwnloadedTotal + allowance + 1048576.0F)) //If they have taken above (1Mb + 'allowance')
		  {/*They may owe us, is it on a file we want or a completed file we are sharing. If it's a completed file progrssively lowering someone score
		     who cannot pay us back could make it very difficult for them to complete the file esp. if it's rare and we hold one of the few complete copies, better for everyone if
		     we share completed files based on time waited + any credit thay have for giving us stuff.
		     If the files a partfile we are trying to get the modifier will start to get smaller -1 to -90Mb range 9 to 1 beyond that 1 to 0 eg: -400Mb = 0.452839 */
 		      CUpDownClient* pClient = theApp.clientlist->FindClientByIP(dwForIP);//Get 'client' so we can get file info
			  CKnownFile* file = theApp.sharedfiles->GetFileByID(pClient->GetUploadFileID());
			  if(file!=NULL){//Are they requesting a file? NULL can be produced when client details calls getscoreratio() without this line eMule will crash.
			                 if(file->IsPartFile()){//It's a file we are trying to obtain so we want to give to givers so we may get the file quicker.
							       float MbSqd =sqrt((float)(uPloadedTotal-(dOwnloadedTotal + allowance))/1048576.0F);
							       if(MbSqd > 9.0F) result = 9.0F / MbSqd;  //above 81mb values 1 - 0 9/(9 - x)
							         else		 result = 10.0F - MbSqd; //for the first 81Mb (10 -(0-9))
							      }
						    }
			} else //We may owe them :o) give a small proportional boost to an uploader
			 	  if(dOwnloadedTotal>uPloadedTotal){ // result =  log(2.72 + (given - taken in Mb * 4)) + given in bytes / 12Mb (eg +1 for every 12Mb +.5  6Mb etc)
						result+=log(2.72F+(float)(dOwnloadedTotal-uPloadedTotal)/262144.0F)+(float)(dOwnloadedTotal/12582912.0F);
						}
		      return result;
           }break;

		case 10: 
			return 1.0f;
		default:
                thePrefs.SetCreditSystem(2);
	        	return GetScoreRatio(dwForIP);

	}

	return result;
	//EastShare END - Added by linekin, CreditSystem 

}
//End

// VQB: ownCredits
float CClientCredits::GetMyScoreRatio(uint32 dwForIP) const
{
	// check the client ident status
	if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
		// bad guy - no credits for... me?
		return 1.0f;
}

	// Cache value
	const uint64 uploadTotal = GetUploadedTotal();

	if (uploadTotal < 1000000)
		return 1.0f;
		
	// Cache value
	const uint64 downloadTotal = GetDownloadedTotal();

	// Factor 1
	float result = (downloadTotal == 0) ?
				   10.0f : (float)(2*uploadTotal)/(float)downloadTotal;

	// Factor 2
	float trunk = (float)sqrt(2.0 + (double)uploadTotal/1048576.0);
	if(result > trunk)
		result = trunk;

	// Trunk final result 1..10
	if(result < 1.0f)
		return 1.0f;
	else if(result > 10.0f)
		return 10.0f;
	else
	return result;

// VQB: ownCredits

}


CClientCreditsList::CClientCreditsList()
{
	m_nLastSaved = ::GetTickCount();
	LoadList();
	
	InitalizeCrypting();
}

CClientCreditsList::~CClientCreditsList()
{
	SaveList();
	CClientCredits* cur_credit;
	CCKey tmpkey(0);
	POSITION pos = m_mapClients.GetStartPosition();
	while (pos){
		m_mapClients.GetNextAssoc(pos, tmpkey, cur_credit);
		delete cur_credit;
	}
	delete m_pSignkey;
}

void CClientCreditsList::LoadList()
{
	CString strFileName = thePrefs.GetConfigDir() + CLIENTS_MET_FILENAME;
	const int iOpenFlags = CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite;
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strFileName, iOpenFlags, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(GetResString(IDS_ERR_LOADCREDITFILE));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
			}
				return;
			}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
	
	try{
		uint8 version = file.ReadUInt8();
		if (version != CREDITFILE_VERSION && version != CREDITFILE_VERSION_29){
			LogWarning(GetResString(IDS_ERR_CREDITFILEOLD));
			file.Close();
			return;
		}

		// everything is ok, lets see if the backup exist...
		CString strBakFileName;
		strBakFileName.Format(_T("%s") CLIENTS_MET_FILENAME _T(".bak"), thePrefs.GetConfigDir());
		
		DWORD dwBakFileSize = 0;
		BOOL bCreateBackup = TRUE;
		
		HANDLE hBakFile = ::CreateFile(strBakFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
										OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hBakFile != INVALID_HANDLE_VALUE)
		{
			// Ok, the backup exist, get the size
			dwBakFileSize = ::GetFileSize(hBakFile, NULL); //debug
			if (dwBakFileSize > (DWORD)file.GetLength())
			{
				// the size of the backup was larger then the org. file, something is wrong here, don't overwrite old backup..
				bCreateBackup = FALSE;
			}
			//else: backup is smaller or the same size as org. file, proceed with copying of file
			::CloseHandle(hBakFile);
		}
		//else: the backup doesn't exist, create it

		if (bCreateBackup)
		{
			file.Close(); // close the file before copying

			if (!::CopyFile(strFileName, strBakFileName, FALSE))
				LogError(GetResString(IDS_ERR_MAKEBAKCREDITFILE));

			// reopen file
			CFileException fexp;
			if (!file.Open(strFileName, iOpenFlags, &fexp)){
				CString strError(GetResString(IDS_ERR_LOADCREDITFILE));
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
					strError += _T(" - ");
					strError += szError;
				}
				LogError(LOG_STATUSBAR, _T("%s"), strError);
				return;
			}
			setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
			file.Seek(1, CFile::begin); //set filepointer behind file version byte
		}

		UINT count = file.ReadUInt32();
		m_mapClients.InitHashTable(count+5000); // TODO: should be prime number... and 20% larger

		const uint32 dwExpired = time(NULL) - 12960000; // today - 150 day
		uint32 cDeleted = 0;
		for (UINT i = 0; i < count; i++){
			CreditStruct* newcstruct = new CreditStruct;
			MEMZERO(newcstruct, sizeof(CreditStruct));
			if (version == CREDITFILE_VERSION_29)
				file.Read(newcstruct, sizeof(CreditStruct_29a));
			else
				file.Read(newcstruct, sizeof(CreditStruct));
			
			if (newcstruct->nLastSeen < dwExpired){
				cDeleted++;
				delete newcstruct;
				continue;
			}

			CClientCredits* newcredits = new CClientCredits(newcstruct);
			m_mapClients.SetAt(CCKey(newcredits->GetKey()), newcredits);
		}
		file.Close();

		if (cDeleted>0)
			AddLogLine(false, GetResString(IDS_CREDITFILELOADED) + GetResString(IDS_CREDITSEXPIRED), count-cDeleted,cDeleted);
		else
			AddLogLine(false, GetResString(IDS_CREDITFILELOADED), count);
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile)
			LogError(LOG_STATUSBAR, GetResString(IDS_CREDITFILECORRUPT));
		else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer, ARRSIZE(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CREDITFILEREAD), buffer);
		}
		error->Delete();
	}
}

void CClientCreditsList::SaveList()
{
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false, _T("Saving clients credit list file \"%s\""), CLIENTS_MET_FILENAME);
	m_nLastSaved = ::GetTickCount();

	CString name = thePrefs.GetConfigDir() + CLIENTS_MET_FILENAME;
	CFile file;// no buffering needed here since we swap out the entire array
	CFileException fexp;
	if (!file.Open(name, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(GetResString(IDS_ERR_FAILED_CREDITSAVE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return;
	}

	uint32 count = m_mapClients.GetCount();
	BYTE* pBuffer = new BYTE[count*sizeof(CreditStruct)];
	CClientCredits* cur_credit;
	CCKey tempkey(0);
	POSITION pos = m_mapClients.GetStartPosition();
	count = 0;
	while (pos)
	{
		m_mapClients.GetNextAssoc(pos, tempkey, cur_credit);
		if (cur_credit->GetUploadedTotal() || cur_credit->GetDownloadedTotal())
		{
			MEMCOPY(pBuffer+(count*sizeof(CreditStruct)), cur_credit->GetDataStruct(), sizeof(CreditStruct));
				count++; 
		}
	}

	try{
		uint8 version = CREDITFILE_VERSION;
		file.Write(&version, 1);
		file.Write(&count, 4);
		file.Write(pBuffer, count*sizeof(CreditStruct));
		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning()))
			file.Flush();
		file.Close();
	}
	catch(CFileException* error){
		CString strError(GetResString(IDS_ERR_FAILED_CREDITSAVE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();
	}

	delete[] pBuffer;
}

CClientCredits* CClientCreditsList::GetCredit(const uchar* key)
{
	CClientCredits* result;
	CCKey tkey(key);
	if (!m_mapClients.Lookup(tkey, result)){
		result = new CClientCredits(key);
		m_mapClients.SetAt(CCKey(result->GetKey()), result);
	}
	result->SetLastSeen();
	return result;
}

void CClientCreditsList::Process()
{
	if (::GetTickCount() - m_nLastSaved > MIN2MS(13))
		SaveList();
}

void CClientCredits::InitalizeIdent()
{
	if (m_pCredits->nKeySize == 0 ){
		MEMZERO(m_abyPublicKey,80); // for debugging
		m_nPublicKeyLen = 0;
		IdentState = IS_NOTAVAILABLE;
	}
	else{
		m_nPublicKeyLen = m_pCredits->nKeySize;
		MEMCOPY(m_abyPublicKey, m_pCredits->abySecureIdent, m_nPublicKeyLen);
		IdentState = IS_IDNEEDED;
	}
	m_dwCryptRndChallengeFor = 0;
	m_dwCryptRndChallengeFrom = 0;
	m_dwIdentIP = 0;
}

void CClientCredits::Verified(uint32 dwForIP)
{
	m_dwIdentIP = dwForIP;
	// client was verified, copy the keyto store him if not done already
	if (m_pCredits->nKeySize == 0){
		m_pCredits->nKeySize = m_nPublicKeyLen; 
		MEMCOPY(m_pCredits->abySecureIdent, m_abyPublicKey, m_nPublicKeyLen);
		if (GetDownloadedTotal() > 0){
			// for security reason, we have to delete all prior credits here
			m_pCredits->nDownloadedHi = 0;
			m_pCredits->nDownloadedLo = 1;
			m_pCredits->nUploadedHi = 0;
			m_pCredits->nUploadedLo = 1; // in order to safe this client, set 1 byte
			if (thePrefs.GetVerbose())
				DEBUG_ONLY(AddDebugLogLine(false, _T("Credits deleted due to new SecureIdent")));
		}
	}
	IdentState = IS_IDENTIFIED;
}

bool CClientCredits::SetSecureIdent(const uchar* pachIdent, uint8 nIdentLen)  // verified Public key cannot change, use only if there is not public key yet
{
	if (MAXPUBKEYSIZE < nIdentLen || m_pCredits->nKeySize != 0 )
		return false;
	MEMCOPY(m_abyPublicKey,pachIdent, nIdentLen);
	m_nPublicKeyLen = nIdentLen;
	IdentState = IS_IDNEEDED;
	return true;
}

EIdentState	CClientCredits::GetCurrentIdentState(uint32 dwForIP) const
{
	if (IdentState != IS_IDENTIFIED)
		return IdentState;
	else{
		if (dwForIP == m_dwIdentIP)
			return IS_IDENTIFIED;
		else
			return IS_IDBADGUY; 
			// mod note: clients which just reconnected after an IP change and have to ident yet will also have this state for 1-2 seconds
			//		 so don't try to spam such clients with "bad guy" messages (besides: spam messages are always bad)
	}
}

using namespace CryptoPP;

void CClientCreditsList::InitalizeCrypting()
{
	m_nMyPublicKeyLen = 0;
	MEMZERO(m_abyMyPublicKey,80); // not really needed; better for debugging tho
	m_pSignkey = NULL;
	if (!thePrefs.IsSecureIdentEnabled())
		return;
	// check if keyfile is there
	bool bCreateNewKey = false;
	HANDLE hKeyFile = ::CreateFile(thePrefs.GetConfigDir() + _T("cryptkey.dat"), GENERIC_READ, FILE_SHARE_READ, NULL,
								   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hKeyFile != INVALID_HANDLE_VALUE)
	{
		if (::GetFileSize(hKeyFile, NULL) == 0)
			bCreateNewKey = true;
		::CloseHandle(hKeyFile);
	}
	else
		bCreateNewKey = true;
	if (bCreateNewKey)
		CreateKeyPair();
	
	// load key
	try{
		// load private key
		FileSource filesource(CStringA(thePrefs.GetConfigDir() + _T("cryptkey.dat")), true,new Base64Decoder);
		m_pSignkey = new RSASSA_PKCS1v15_SHA_Signer(filesource);
		// calculate and store public key
		RSASSA_PKCS1v15_SHA_Verifier pubkey(*m_pSignkey);
		ArraySink asink(m_abyMyPublicKey, 80);
		pubkey.DEREncode(asink);
		m_nMyPublicKeyLen = (uint8)asink.TotalPutLength();
		asink.MessageEnd();
	}
	catch(...)
	{
		if (m_pSignkey){
			delete m_pSignkey;
			m_pSignkey = NULL;
		}
		LogError(LOG_STATUSBAR, GetResString(IDS_CRYPT_INITFAILED));
		ASSERT(0);
	}
	//Debug_CheckCrypting();
}

bool CClientCreditsList::CreateKeyPair()
{
	try{
		AutoSeededRandomPool rng;
		InvertibleRSAFunction privkey;
		privkey.Initialize(rng,RSAKEYSIZE);

		Base64Encoder privkeysink(new FileSink(CStringA(thePrefs.GetConfigDir() + _T("cryptkey.dat"))));
		privkey.DEREncode(privkeysink);
		privkeysink.MessageEnd();

		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("Created new RSA keypair"));
	}
	catch(...)
	{
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Failed to create new RSA keypair"));
		ASSERT ( false );
		return false;
	}
	return true;
}

uint8 CClientCreditsList::CreateSignature(CClientCredits* pTarget, uchar* pachOutput, uint8 nMaxSize, 
										  uint32 ChallengeIP, uint8 byChaIPKind, 
										  CryptoPP::RSASSA_PKCS1v15_SHA_Signer* sigkey)
{
	// sigkey param is used for debug only
	if (sigkey == NULL)
		sigkey = m_pSignkey;

	// create a signature of the public key from pTarget
	ASSERT( pTarget );
	ASSERT( pachOutput );
	uint8 nResult;
	if ( !CryptoAvailable() )
		return 0;
	try{
		
		SecByteBlock sbbSignature(sigkey->SignatureLength());
		AutoSeededRandomPool rng;
		byte abyBuffer[MAXPUBKEYSIZE+9];
		uint32 keylen = pTarget->GetSecIDKeyLen();
		MEMCOPY(abyBuffer,pTarget->GetSecureIdent(),keylen);
		// 4 additional bytes random data send from this client
		uint32 challenge = pTarget->m_dwCryptRndChallengeFrom;
		ASSERT ( challenge != 0 );
		PokeUInt32(abyBuffer+keylen, challenge);
		uint16 ChIpLen = 0;
		if ( byChaIPKind != 0){
			ChIpLen = 5;
			PokeUInt32(abyBuffer+keylen+4, ChallengeIP);
			PokeUInt8(abyBuffer+keylen+4+4, byChaIPKind);
		}
		sigkey->SignMessage(rng, abyBuffer ,keylen+4+ChIpLen , sbbSignature.begin());
		ArraySink asink(pachOutput, nMaxSize);
		asink.Put(sbbSignature.begin(), sbbSignature.size());
		nResult = (uint8)asink.TotalPutLength();			
	}
	catch(...)
	{
		ASSERT ( false );
		nResult = 0;
	}
	return nResult;
}

bool CClientCreditsList::VerifyIdent(CClientCredits* pTarget, const uchar* pachSignature, uint8 nInputSize, 
									 uint32 dwForIP, uint8 byChaIPKind)
{
	ASSERT( pTarget );
	ASSERT( pachSignature );
	if ( !CryptoAvailable() ){
		pTarget->IdentState = IS_NOTAVAILABLE;
		return false;
	}
	bool bResult;
	try{
		StringSource ss_Pubkey((byte*)pTarget->GetSecureIdent(),pTarget->GetSecIDKeyLen(),true,0);
		RSASSA_PKCS1v15_SHA_Verifier pubkey(ss_Pubkey);
		// 4 additional bytes random data send from this client +5 bytes v2
		byte abyBuffer[MAXPUBKEYSIZE+9];
		MEMCOPY(abyBuffer,m_abyMyPublicKey,m_nMyPublicKeyLen);
		uint32 challenge = pTarget->m_dwCryptRndChallengeFor;
		ASSERT ( challenge != 0 );
		PokeUInt32(abyBuffer+m_nMyPublicKeyLen, challenge);
		
		// v2 security improvments (not supported by 29b, not used as default by 29c)
		uint8 nChIpSize = 0;
		if (byChaIPKind != 0){
			nChIpSize = 5;
			uint32 ChallengeIP = 0;
			switch (byChaIPKind){
				case CRYPT_CIP_LOCALCLIENT:
					ChallengeIP = dwForIP;
					break;
				case CRYPT_CIP_REMOTECLIENT:
					if (theApp.serverconnect->GetClientID() == 0 || theApp.serverconnect->IsLowID()){
						if (thePrefs.GetLogSecureIdent())
							AddDebugLogLine(false, _T("Warning: Maybe SecureHash Ident fails because LocalIP is unknown"));
						ChallengeIP = theApp.serverconnect->GetLocalIP();
					}
					else
						ChallengeIP = theApp.serverconnect->GetClientID();
					break;
				case CRYPT_CIP_NONECLIENT: // maybe not supported in future versions
					ChallengeIP = 0;
					break;
			}
			PokeUInt32(abyBuffer+m_nMyPublicKeyLen+4, ChallengeIP);
			PokeUInt8(abyBuffer+m_nMyPublicKeyLen+4+4, byChaIPKind);
		}
		//v2 end

		bResult = pubkey.VerifyMessage(abyBuffer, m_nMyPublicKeyLen+4+nChIpSize, pachSignature, nInputSize);
	}
	catch(...)
	{
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Error: Unknown exception in %hs"), __FUNCTION__);
		//ASSERT(0);
		bResult = false;
	}
	if (!bResult){
		if (pTarget->IdentState == IS_IDNEEDED)
			pTarget->IdentState = IS_IDFAILED;
	}
	else{
		pTarget->Verified(dwForIP);
	}
	return bResult;
}

bool CClientCreditsList::CryptoAvailable()
{
	return (m_nMyPublicKeyLen > 0 && m_pSignkey != 0 && thePrefs.IsSecureIdentEnabled() );
}


#ifdef _DEBUG
bool CClientCreditsList::Debug_CheckCrypting()
{
	// create random key
	AutoSeededRandomPool rng;

	RSASSA_PKCS1v15_SHA_Signer priv(rng, 384);
	RSASSA_PKCS1v15_SHA_Verifier pub(priv);

	byte abyPublicKey[80];
	ArraySink asink(abyPublicKey, 80);
	pub.DEREncode(asink);
	uint8 PublicKeyLen = (uint8)asink.TotalPutLength();
	asink.MessageEnd();
	uint32 challenge = rand();
	// create fake client which pretends to be this emule
	CreditStruct* newcstruct = new CreditStruct;
	MEMZERO(newcstruct, sizeof(CreditStruct));
	CClientCredits* newcredits = new CClientCredits(newcstruct);
	newcredits->SetSecureIdent(m_abyMyPublicKey,m_nMyPublicKeyLen);
	newcredits->m_dwCryptRndChallengeFrom = challenge;
	// create signature with fake priv key
	uchar pachSignature[200];
	MEMZERO(pachSignature,200);
	uint8 sigsize = CreateSignature(newcredits,pachSignature,200,0,false, &priv);


	// next fake client uses the random created public key
	CreditStruct* newcstruct2 = new CreditStruct;
	MEMZERO(newcstruct2, sizeof(CreditStruct));
	CClientCredits* newcredits2 = new CClientCredits(newcstruct2);
	newcredits2->m_dwCryptRndChallengeFor = challenge;

	// if you uncomment one of the following lines the check has to fail
	//abyPublicKey[5] = 34;
	//m_abyMyPublicKey[5] = 22;
	//pachSignature[5] = 232;

	newcredits2->SetSecureIdent(abyPublicKey,PublicKeyLen);

	//now verify this signature - if it's true everything is fine
	bool bResult = VerifyIdent(newcredits2,pachSignature,sigsize,0,0);

	delete newcredits;
	delete newcredits2;

	return bResult;
}
#endif
uint32 CClientCredits::GetSecureWaitStartTime(uint32 dwForIP)
{
	if (m_dwUnSecureWaitTime == 0 || m_dwSecureWaitTime == 0)
		SetSecWaitStartTime(dwForIP);

	if (m_pCredits->nKeySize != 0){	// this client is a SecureHash Client
		if (GetCurrentIdentState(dwForIP) == IS_IDENTIFIED){ // good boy
			return m_dwSecureWaitTime;
		}
		else{	// not so good boy
			if (dwForIP == m_dwWaitTimeIP){
				return m_dwUnSecureWaitTime;
			}
			else{	// bad boy
				// this can also happen if the client has not identified himself yet, but will do later - so maybe he is not a bad boy :) .
				CString buffer2, buffer;
				/*for (uint16 i = 0;i != 16;i++){
					buffer2.Format("%02X",this->m_pCredits->abyKey[i]);
					buffer+=buffer2;
				}
				if (thePrefs.GetLogSecureIdent())
					AddDebugLogLine(false,"Warning: WaitTime resetted due to Invalid Ident for Userhash %s",buffer.GetBuffer());*/
				
				m_dwUnSecureWaitTime = ::GetTickCount();
				m_dwWaitTimeIP = dwForIP;
				return m_dwUnSecureWaitTime;
			}	
		}
	}
	else{	// not a SecureHash Client - handle it like before for now (no security checks)
		return m_dwUnSecureWaitTime;
	}
}

void CClientCredits::SetSecWaitStartTime(uint32 dwForIP)
{
	m_dwUnSecureWaitTime = ::GetTickCount()-1;
	m_dwSecureWaitTime = ::GetTickCount()-1;
	m_dwWaitTimeIP = dwForIP;
}

void CClientCredits::ClearWaitStartTime()
{
	m_dwUnSecureWaitTime = 0;
	m_dwSecureWaitTime = 0;
}
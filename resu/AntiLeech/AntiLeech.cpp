//////////////////////////////////////////////////////////////////////////
//				  AntiLeechClass v1 B3t4 by WiZaRd						//
//----------------------------------------------------------------------//
//																		//
//	this class is currently under development, please do NOT change		//
//	anything here until it has final state!								//
//	Please give credit to the author(s) if you use it and it'd be nice	//
//	if you'd drop a line to The_Wizard_of_DOS@web.de if you have any	//
//	suggestions/improvements.											//
//////////////////////////////////////////////////////////////////////////
//
//AntiLeechClass currently contains:
//- AntiNickThief v2.3
//- AntiModThief v2.2
//- BadUserName Check v1.0
//- BanModName Check v1.0
//for details see below!
//
//
//>>>>>>>>>>>>>>>>>>>>>>> AntiNickThief start <<<<<<<<<<<<<<<<<<<<<<<<<<//
//The whole feature is not about detecting clients with the same name but
//clients using a nickthief.
//Though it is annoying, this is not harmful in any way to us!
//BUT clients using a nickthief will for 99% also use other bad "features"
//like a very low sessionupload, fakeranks or even no ratio/0 UL
//
//That's why I want to detect (and ban) them! And the nickthief is very
//easy to detect!
//
//The AntiNickThief sends a random string to each user, if the user will
//adapt that string to his name and send it back, we can (safely) ban him
//
//
//Revision History:
//v2.3: added random brackets to the lure tag
//v2.2: added a small fix to always get the mirroring clients
//		even shortly after the 1-update-per-day
//
//v2.1: use one string for all users saving lots of RAM
//		also only update once per day saving a few CPU cycles
//		and also fixed a bug causing garbage and eating up CPU (mea culpa)
//		I know that there will be a short period when we will not detect
//		nickthieves (i.e. because of the 1-update-per-day) but sooner or later
//		they will run into it ;)
//
//v2:	send random string of random length
//		this should take care of leechers simply cutting the anti-tag
//		if it has exactly 6 chars
//
//v1:	send random string to each user
//>>>>>>>>>>>>>>>>>>>>>>> AntiNickThief end <<<<<<<<<<<<<<<<<<<<<<<<<<<<//
//
//
//>>>>>>>>>>>>>>>>>>>>>>> AntiModThief start <<<<<<<<<<<<<<<<<<<<<<<<<<<//
//The whole feature is not about detecting clients with the same modstring but
//clients using a modthief.
//Though it is annoying, this is not harmful in any way to us!
//BUT clients using a modthief will for 99% also use other bad "features"
//like a very low sessionupload, fakeranks or even no ratio/0 UL
//
//That's why I want to detect (and ban) them! And the modthief is very
//easy to detect! Though, it will only detect stupid modthieves, the number
//of bans is impressive... :(
//
//The AntiModThief checks if a user is mirroring our string but not in correct case
//(which is supposed to be an easy way to avoid banning, but it isn't...) or using
//the modsting with a different eMule version
//
//
//Revision History:
//v2.2: added another possibility to detect modfakers
//		Note that you have to change the VERSION_BUILD definition in Version.h!
//
//v2.1:	Also check for correct length (this is to ban clients appending s.thing to
//		the modstring before sending it (might be Community-Tags?)
//
//v2:	Also check for uncorrect case
//
//v1:	Check for uncorrect eMule version
//>>>>>>>>>>>>>>>>>>>>>>>> AntiModThief end <<<<<<<<<<<<<<<<<<<<<<<<<<<<//
//
//
//>>>>>>>>>>>>>>>>>>>>>>>> BadUserName start <<<<<<<<<<<<<<<<<<<<<<<<<<<//
//The whole feature is not about detecting clients with usernames you don't
//like but clients using certain mods with specific static usernames.
//The username itself is not harmful in any way to us!
//BUT clients using such a mod will for 99% also use other bad "features"
//like a very low sessionupload, fakeranks or even no ratio/0 UL
//
//That's why I want to detect (and ban) them!
//
//
//Revision History:
//v1:	Basic implemetation
//>>>>>>>>>>>>>>>>>>>>>>>>> BadUserName end <<<<<<<<<<<<<<<<<<<<<<<<<<<<//
//
//
//>>>>>>>>>>>>>>>>>>>>>>>> BadModName start <<<<<<<<<<<<<<<<<<<<<<<<<<<//
//The whole feature is not about detecting clients with modnames you don't
//like but clients using certain mods with specific static modnames.
//The modname itself is not harmful in any way to us!
//BUT clients using such a mod will for 99% also use other bad "features"
//like a very low sessionupload, fakeranks or even no ratio/0 UL
//
//That's why I want to detect (and ban) them!
//
//
//Revision History:
//v1:	Basic implemetation
//>>>>>>>>>>>>>>>>>>>>>>>>> BadModName end <<<<<<<<<<<<<<<<<<<<<<<<<<<<//

#include "stdafx.h"
#include "AntiLeech.h"
#include "updownclient.h"
#include "Version.h" //for VERSION tags
#include "Opcodes.h" //for MOD_VERSION
#include "Preferences.h" //for GetUserNick()
#include "log.h"// for Modlog
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

CAntiLeech theAntiLeechClass;

//>>> AntiNickThief
#define MIN_LENGTH	4						//min chars to use
#define MAX_LENGTH	8						//max chars to use
#if (MIN_LENGTH > MAX_LENGTH)
#error MAX_LENGTH MUST BE GREATER THAN OR EQUAL TO MIN_LENGTH
#endif
#define MAX_ADD		(MAX_LENGTH-MIN_LENGTH) //max chars to add
#define MAX_VALID	(24*60*60*1000)			//1 day expiration
#define MAX_RECHECK MIN2MS(30)				//~1 reask time
CString	CAntiLeech::m_sAntiNickThiefTag;
uint32	CAntiLeech::m_uiAntiNickThiefCreateTimer = NULL;
CString	CAntiLeech::m_sAntiNickThiefUpdate = NULL;
//<<< AntiNickThief
CString	CAntiLeech::m_sMyVersion; //>>> AntiModThief
//>>> CRYPTO
CStringList CAntiLeech::strTrustedPubKeys;
//<<< CRYPTO
//>>> AntiLeech.dat File Support
CRBMap<CString, CBaninfo> CAntiLeech::m_BadVerList;
CRBMap<CString, CBaninfo> CAntiLeech::m_BadModList;
CRBMap<CString, CBaninfo> CAntiLeech::m_BadNameList;
//<<< AntiLeech.dat File Support

//>>> Global functions
void	CAntiLeech::Init()
{
	CreateAntiNickThiefTag();
	m_sMyVersion.Format(_T("eMule v%u.%u%c"), VERSION_MJR, VERSION_MIN, _T('a') + VERSION_UPDATE);
	LoadFromFile(); //>>> AntiLeech.dat File Support
//>>> CRYPTO
	strTrustedPubKeys.AddTail(_T("30819D300D06092A864886F70D010101050003818B0030818702818100DEA0C24A24E93641D06A997B047F0CC13FD17535B0EBAC1BE2C32B35273B6543A6E41F55D94D00FC99D8F3A826B7697B5F53BDB3D184EC3B8F4232F4C82B0672D9AD1CF49A3CAE397DB587B47BA7D8E620C0704C7825090C9136E38F111A5BD2D65B603612AEC68208D4BCC4A83B399C991FECFE3606068E2ACA37D56EC742D7020111"));
	CheckKeys();
//<<< CRYPTO
}

void	CAntiLeech::UnInit()
{
	SaveToFile();
}
//<<< Global functions
//>>> AntiNickThief
void CAntiLeech::CreateAntiNickThiefTag()
{
	//if we have an old string, cache it
	if(!m_sAntiNickThiefTag.IsEmpty())
		m_sAntiNickThiefUpdate = m_sAntiNickThiefTag;

	CString nick = _T("");
	uint8 maxchar = MIN_LENGTH+rand()%MAX_ADD; //BuGFiX: d'oh - stupid me!!
	for(uint8 i = 0; i < maxchar; ++i)
	{
		if(rand()%2)
			nick.AppendFormat(_T("%c"), _T('A')+rand()%25);
		else
			nick.AppendFormat(_T("%c"), _T('a')+rand()%25);
	}
	m_sAntiNickThiefTag.Format(_T("%s"), nick);
	m_uiAntiNickThiefCreateTimer = ::GetTickCount()+MAX_VALID;
}

CString	CAntiLeech::GetAntiNickThiefNick()
{
	if(::GetTickCount() > m_uiAntiNickThiefCreateTimer)
		CreateAntiNickThiefTag();
	CString ret = thePrefs.GetUserNick();

	switch(rand()%3)
	{
		case 0:
			ret.AppendFormat(_T(" (%s)"), m_sAntiNickThiefTag);
			break;
		case 1:
			ret.AppendFormat(_T(" [%s]"), m_sAntiNickThiefTag);
			break;
		case 2:
			ret.AppendFormat(_T(" {%s}"), m_sAntiNickThiefTag);
			break;
		default:
			ret.AppendFormat(_T(" [%s]"), m_sAntiNickThiefTag);
			break;
	}
	return ret;
}

void CAntiLeech::Check4NickThief(CUpDownClient* client)
{
	if(!thePrefs.GetAntiNickThief() || !client->GetUserName() || CString(client->GetUserName()).IsEmpty())
		return; //just a sanity check

	if(FindOurTagIn(client->GetUserName()))
	{
		if(!client->IsBanned() && thePrefs.IsLeecherSecureLog())
			AddModLogLine(false, _T("Ban: (%s) used a NickThief -> banned!"), client->GetUserName());
		CString msg;
		msg.Format(_T("%s was banned! Reason: NickThief!"), client->GetUserName());
		client->Ban(msg);
	}
}

bool CAntiLeech::FindOurTagIn(const CString& tocomp)
{
	//is he mirroring our current tag?
	if(tocomp.Find(m_sAntiNickThiefTag) != -1)
		return true;

	//if we are below the timelimit, then also check for the old string!
	if(m_uiAntiNickThiefCreateTimer > ::GetTickCount() //should always be true...
		&& !m_sAntiNickThiefUpdate.IsEmpty() //just to be sure... :)
		//is usually+1Day, let's check if we have changed the string below MAX_RECHECK
		&& m_uiAntiNickThiefCreateTimer - GetTickCount() > MAX_VALID-MAX_RECHECK
		//and we find the old string...
		&& tocomp.Find(m_sAntiNickThiefUpdate) != -1)
		return true;

	//else he is a nice guy ;)
	return false;
}
//<<< AntiNickThief
//>>> AntiModThief
void CAntiLeech::Check4ModThief(CUpDownClient* client)
{
	if(!thePrefs.IsAntiModIdFaker() || client->GetClientModVer().IsEmpty())
		return;

	CString OurMod = MOD_VERSION; //cache it
	if(StrStrI(client->GetClientModVer(), OurMod) //uses our string
		&& (client->GetClientModVer().GetLength() != OurMod.GetLength() //but not the correct length
		|| !StrStr(client->GetClientModVer(), OurMod) //but not in correct case
		|| client->GetClientSoftVer() != m_sMyVersion //or wrong version
//removed because GetVersion() works different than in my own version and does not return the "real" version...
		//|| client->GetVersion() != ((CemuleApp::m_nVersionMjr<<17)|(CemuleApp::m_nVersionMin<<10)|(CemuleApp::m_nVersionUpd<<7)|(CemuleApp::m_nVersionBld))
		))
	{
		if(!client->IsBanned() && thePrefs.IsLeecherSecureLog())
			AddModLogLine(false, _T("Ban: (%s) uses a ModThief"), client->GetUserName());
		CString msg;
		msg.Format(_T("%s was banned! Reason: ModThief"), client->GetUserName());
		client->Ban(msg);
	}
}
//<<< AntiModThief
//>>> Common AntiLeech
void CAntiLeech::Check4BadVer(CUpDownClient* client)
{
	if(!thePrefs.IsLeecherSecure() || client->GetClientSoftVer().IsEmpty())
		return; //just a sanity check

	if ( FindInBadVers(client->GetClientSoftVer()) )
	{
		if(!client->IsBanned() && thePrefs.IsLeecherSecureLog())
			AddModLogLine(false, _T("Ban: (%s) uses a leecher mod [%s]"), client->GetUserName(), client->GetClientModVer());
		CString msg;
		msg.Format(_T("%s was banned! Reason: leecher mod [%s]"), client->GetUserName(), client->GetClientModVer());
		client->Ban(msg);
	}
}

void CAntiLeech::Check4BadMod(CUpDownClient* client)
{
	if(!thePrefs.IsLeecherSecure() || client->GetClientModVer().IsEmpty())
		return; //just a sanity check

	if ( FindInBadMods(client->GetClientModVer()))
	{
		if(!client->IsBanned() && thePrefs.IsLeecherSecureLog())
			AddModLogLine(false, _T("Ban: (%s) uses a leecher mod [%s]"), client->GetUserName(), client->GetClientModVer());
		CString msg;
		msg.Format(_T("%s was banned! Reason: leecher mod [%s]"), client->GetUserName(), client->GetClientModVer());
		client->Ban(msg);
	}
}

void CAntiLeech::Check4BadName(CUpDownClient* client)
{
	if(!thePrefs.IsLeecherSecure() || !client->GetUserName() || CString(client->GetUserName()).IsEmpty())
		return; //just a sanity check

	if (FindInBadNames(client->GetUserName()))
	{
		if(!client->IsBanned() && thePrefs.IsLeecherSecureLog())
			AddModLogLine(false, _T("Ban: (%s) uses a leecher name"), client->GetUserName());
		CString msg;
		msg.Format(_T("%s was banned! Reason: leecher name!"), client->GetUserName());
		client->Ban(msg);
	}
}
//<<< Common AntiLeech
//>>> AntiLeech.dat File Support
CBaninfo::CBaninfo()
{
	strReason = GetResString(IDS_UNKNOWN);
	bBan = true;
	bCaseSensitive = false;
}

CBaninfo::CBaninfo(const CString& s, const bool& b, const bool& c)
{
	strReason = s;
	bBan = b;
	bCaseSensitive = c;
}

bool CAntiLeech::SaveToFile(const CString& path)
{
	CString open;
	if(path.IsEmpty())
		open = GetDefaultFilePath();
	else
		open = path;

	int iDot = open.ReverseFind(_T('.'));
	if (iDot == -1)
		return false;
	const CString prefpath = open.Mid(0, iDot)+_T(".pref");
	CStdioFile pref;
	if (!pref.Open(prefpath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeText))
		return false;

	for(POSITION pos = m_BadVerList.GetHeadPosition(); pos; m_BadVerList.GetNext(pos))
	{
		CBaninfo info = m_BadVerList.GetValueAt(pos);
		CString msg;

		msg.Format(_T("%s,%s,%s\n"),
			info.GetReason(),
			info.Ban()?_T("1"):_T("0"),
			info.UseCase()?_T("1"):_T("0"));
		pref.WriteString(msg);
	}

	for(POSITION pos = m_BadModList.GetHeadPosition(); pos; m_BadModList.GetNext(pos))
	{
		CBaninfo info = m_BadModList.GetValueAt(pos);
		CString msg;

		msg.Format(_T("%s,%s,%s\n"),
			info.GetReason(),
			info.Ban()?_T("1"):_T("0"),
			info.UseCase()?_T("1"):_T("0"));
		pref.WriteString(msg);
	}

	for(POSITION pos = m_BadNameList.GetHeadPosition(); pos; m_BadNameList.GetNext(pos))
	{
		CBaninfo info = m_BadNameList.GetValueAt(pos);
		CString msg;

		msg.Format(_T("%s,%s,%s\n"),
			info.GetReason(),
			info.Ban()?_T("1"):_T("0"),
			info.UseCase()?_T("1"):_T("0"));
		pref.WriteString(msg);
	}

	pref.Close();

	return true;
}

bool CAntiLeech::LoadFromFile(const CString& path, const bool& bMerge)
{
	CString open;
	if(path.IsEmpty())
		open = GetDefaultFilePath();
	else
		open = path;

	CStdioFile f;
	if (!f.Open(open, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
		return false;

	int iDot = open.ReverseFind(_T('.'));
	if (iDot == -1)
		return false;
	const CString prefpath = open.Mid(0, iDot)+_T(".pref");
	CStdioFile pref;
	if (!pref.Open(prefpath, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
		return false;

	const uint32 startMesure = ::GetTickCount();
	uint32 leechcounter = 0;
	uint32 iDuplicate = 0;
	uint32 iMerged = 0;

	if(!bMerge)
	{
		RemoveAllBadVers();
		RemoveAllBadMods();
		RemoveAllBadNames();
	}

	CString strLine;
	uint32 linecounter = 0;

	while(f.ReadString(strLine))
	{
		linecounter++;
		strLine.Trim();

		// ignore comments
		if (strLine.GetAt(0) == _T('#')
			|| strLine.Left(2) == _T("//")
			|| strLine.IsEmpty())
			continue;

		CString tmpstrLine = strLine;

		int pos = tmpstrLine.Find(_T(','));
		if (pos == -1)
		{
			DebugLogError(_T("AntiLeech: Line %u of \"%s\" is invalid (%s) - please check it!"), linecounter, open, strLine);
			continue;
		}

		const uint8 kind = _tstoi(tmpstrLine.Left(pos));
		tmpstrLine = tmpstrLine.Mid(pos+1);

		CString toban;
		pos = tmpstrLine.Find(_T(','));
		if (pos == -1)
			toban = tmpstrLine;
		else
			toban = tmpstrLine.Left(pos);

		//now read in pref file
		CString reason = GetResString(IDS_UNKNOWN);
		bool bBan = true;
		bool bCaseSensitive = false;
		if(pref.ReadString(strLine))
		{
			strLine.Trim();
			CString tmpstrLine = strLine;
			int pos = tmpstrLine.Find(_T(','));
			if (pos == -1)
				reason = tmpstrLine;
			else
			{
                reason = tmpstrLine.Left(pos);
				if(reason.IsEmpty())
					reason = GetResString(IDS_UNKNOWN);
				tmpstrLine = tmpstrLine.Mid(pos+1);
				pos = tmpstrLine.Find(_T(','));

				if (pos == -1)
					bBan = _tstoi(tmpstrLine)==1;
				else
				{
					bBan = _tstoi(tmpstrLine.Left(pos))==1;
					bCaseSensitive = _tstoi(tmpstrLine.Mid(pos+1))==1;
				}
			}
		}

		if(toban.IsEmpty())
		{
			DebugLogError(_T("AntiLeech: Line %u of \"%s\" is invalid (%s) - please check it!"), linecounter, open, strLine);
			continue;
		}

		bool bBanned = false;
		bool bAdded = false;

		switch(kind)
		{
			case 0:
				bBanned = IsBadName(toban);
				bAdded = AddBadName(toban, reason, bBan, bCaseSensitive);
				break;
			case 1:
				bBanned = IsBadMod(toban);
				bAdded = AddBadMod(toban, reason, bBan, bCaseSensitive);
				break;
			case 2:
				bBanned = IsBadVer(toban);
				bAdded = AddBadVer(toban, reason, bBan);
				break;
			default:
				continue; //something wrong...
		}

		leechcounter++;
		if(bBanned)
		{
			if(!bAdded) //if not added duplicate
				iDuplicate++;
			else //else merged to our list
				iMerged++;
		}
	}

	f.Close();
	pref.Close();

	AddModLogLine((m_BadModList.GetCount() == 0 ? LOG_WARNING : LOG_SUCCESS), _T("AntiLeech: Loaded %u known bad mods + %u known bad names + %u known bad versions in %s, Reference(s): %u, Duplicate: %u, Merged: %u"),
		m_BadModList.GetCount(), m_BadNameList.GetCount(), m_BadVerList.GetCount(), CastSecondsToHM((::GetTickCount()-startMesure)/1000), leechcounter, iDuplicate, iMerged);

	return true;
}

CString CAntiLeech::GetDefaultFilePath()
{
	return thePrefs.GetConfigDir() + DFLT_ANTILEECH_FILENAME;
}

void CAntiLeech::RemoveAllBadVers()
{
	m_BadVerList.RemoveAll();
}

void CAntiLeech::RemoveAllBadMods()
{
	m_BadModList.RemoveAll();
}

void CAntiLeech::RemoveAllBadNames()
{
	m_BadNameList.RemoveAll();
}

bool CAntiLeech::IsBadVer(const CString& s)
{
	CBaninfo reason;
	return m_BadVerList.Lookup(s, reason);
}

bool CAntiLeech::IsBadMod(const CString& s)
{
	CBaninfo reason;
	return m_BadModList.Lookup(s, reason);
}

bool CAntiLeech::IsBadName(const CString& s)
{
	CBaninfo reason;
	return m_BadNameList.Lookup(s, reason);
}

bool CAntiLeech::AddBadVer(const CString& s, const CString& reason, const bool& b)
{
	CBaninfo treason;
	if(m_BadVerList.Lookup(s, treason))
		return (reason == treason.GetReason());

	treason.strReason = reason;
	treason.bBan = b;
	treason.bCaseSensitive = false;
	m_BadVerList.SetAt(s, treason);
	return true;
}

bool CAntiLeech::AddBadMod(const CString& s, const CString& reason, const bool& b, const bool& c)
{
	CBaninfo treason;
	if(m_BadModList.Lookup(s, treason))
		return (reason == treason.GetReason());

	treason.strReason = reason;
	treason.bBan = b;
	treason.bCaseSensitive = c;
	m_BadModList.SetAt(s, treason);
	return true;
}

bool CAntiLeech::AddBadName(const CString& s, const CString& reason, const bool& b, const bool& c)
{
	CBaninfo treason;
	if(m_BadNameList.Lookup(s, treason))
		return (reason == treason.GetReason());

	treason.strReason = reason;
	treason.bBan = b;
	treason.bCaseSensitive = c;
	m_BadNameList.SetAt(s, treason);
	return true;
}

//I know the finding is pretty slow but it equals the speed of a static list... so who cares
bool CAntiLeech::FindInBadVers(const CString& s)
{
	for(POSITION pos = m_BadVerList.GetHeadPosition(); pos; m_BadVerList.GetNext(pos))
	{
		CBaninfo info = m_BadVerList.GetValueAt(pos);
		if(info.Ban())
		{
			/*if(info.UseCase())
			{
				if(StrStr(s, m_BadVerList.GetKeyAt(pos)))
					return true;
			}
			else*/
			{
				if(StrStrI(s, m_BadVerList.GetKeyAt(pos)))
					return true;
			}
		}
	}
	return false;
}

bool CAntiLeech::FindInBadMods(const CString& s)
{
	for(POSITION pos = m_BadModList.GetHeadPosition(); pos; m_BadModList.GetNext(pos))
	{
		CBaninfo info = m_BadModList.GetValueAt(pos);
		if(info.Ban())
		{
			if(info.UseCase())
			{
				if(StrStr(s, m_BadModList.GetKeyAt(pos)))
					return true;
			}
			else
			{
				if(StrStrI(s, m_BadModList.GetKeyAt(pos)))
					return true;
			}
		}
	}
	return false;
}

bool CAntiLeech::FindInBadNames(const CString& s)
{
	for(POSITION pos = m_BadNameList.GetHeadPosition(); pos; m_BadNameList.GetNext(pos))
	{
		CBaninfo info = m_BadNameList.GetValueAt(pos);
		if(info.Ban())
		{
			if(info.UseCase())
			{
				if(StrStr(s, m_BadNameList.GetKeyAt(pos)))
					return true;
			}
			else
			{
				if(StrStrI(s, m_BadNameList.GetKeyAt(pos)))
					return true;
			}
		}
	}
	return false;
}
//>>> CRYPTO
//Codeparts taken from http://cryptopp.sourceforge.net/docs/ref/test_cpp-source.html
void CAntiLeech::CheckKeys()
{
	try
	{
		const CStringA sigfilename = CStringA(thePrefs.GetConfigDir()+_T("antileech.sig"));
		const CStringA tocheck = CStringA(GetDefaultFilePath());

		bool bVerified = false;
		FileSource signatureFile(sigfilename, true, new Base64Decoder);
		for(POSITION pos = strTrustedPubKeys.GetHeadPosition(); pos && !bVerified;)
		{
			CString tmp = strTrustedPubKeys.GetNext(pos);
			uchar abyMyPublicKey[1024];
			MEMSET(abyMyPublicKey, 0, 1024);
			DecodeBase16(tmp, tmp.GetLength(), abyMyPublicKey, ARRSIZE(abyMyPublicKey));
			StringSource pubFile(abyMyPublicKey, tmp.GetLength()/2U, true, 0);
			RSASSA_PKCS1v15_SHA_Verifier pub(pubFile);

			if (signatureFile.MaxRetrievable() == pub.SignatureLength())
			{
				SecByteBlock signature(pub.SignatureLength());
				signatureFile.Get(signature.begin(), signature.size());

				VerifierFilter *verifierFilter = new VerifierFilter(pub);
				verifierFilter->Put(signature.begin(), signature.size());
				FileSource f(tocheck, true, verifierFilter);

				byte result = 0;
				f.Get(result);
				bVerified = (result == 1);
			}
		}

		if(bVerified)
			AddModLogLine(true, _T("AntiLeech: Public key verified"));
		else
		{
			AddModLogLine(true, _T("AntiLeech: Verification of public key failed!"));
			AddModLogLine(true, _T("AntiLeech: Disabled"));
			//No banning in this case...
			RemoveAllBadVers();
			RemoveAllBadMods();
			RemoveAllBadNames();
		}
	}
	catch(...)
	{
		AddModLogLine(LOG_ERROR, _T("AntiLeech: Failed to verify key! (%s)"), GetErrorMessage(GetLastError()));
			AddModLogLine(LOG_WARNING, _T("AntiLeech: Disabled"));
		//No banning in this case...
		RemoveAllBadVers();
		RemoveAllBadMods();
		RemoveAllBadNames();
	}
}
//<<< CRYPTO
//<<< AntiLeech.dat File Support
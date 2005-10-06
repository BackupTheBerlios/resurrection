//PLEASE see notes in AntiLeech.cpp!

#pragma once
//>>> CRYPTO
#pragma warning(disable:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#include <crypto.v52.1/rsa.h>
#include <crypto.v52.1/base64.h>
#include <crypto.v52.1/osrng.h>
#include <crypto.v52.1/files.h>
#include <crypto.v52.1/sha.h>
#pragma warning(default:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
//<<< CRYPTO

class CUpDownClient;
//>>> AntiLeech.dat File Support
class CBaninfo
{
public:
	CBaninfo();
	CBaninfo(const CString& s, const bool& b = true, const bool& c = false);

	bool	UseCase() const	{return bCaseSensitive;}
	bool	Ban() const		{return bBan;}
	const CString& GetReason() const	{return strReason;}

	CString strReason;
	bool	bBan;
	bool	bCaseSensitive;
};
//<<< AntiLeech.dat File Support

class CAntiLeech
{
//>>> Global functions
public:
	static	void	Init();
	static	void	UnInit();
//<<< Global functions
//>>> AntiNickThief
public:
	//this creates a new string or returns the current one
	static	CString	GetAntiNickThiefNick();

	//performs the nickthief checks
	static	void	Check4NickThief(CUpDownClient* client);

private:
	//this returns if we have a nickthief
	static	bool	FindOurTagIn(const CString& tocomp);

	//this will be automatically called
	static	void	CreateAntiNickThiefTag();

	//the string
	static	CString m_sAntiNickThiefTag;

	//the update timer
	static	uint32	m_uiAntiNickThiefCreateTimer;

	//a second string
	static	CString	m_sAntiNickThiefUpdate;
//<<< AntiNickThief
//>>> AntiModThief
public:
	//this returns if we have a modthief
	static	void	Check4ModThief(CUpDownClient* client);

private:
	static	CString m_sMyVersion;
//<<< AntiModThief
//>>> Common AntiLeech
public: 
	static	void	Check4BadVer(CUpDownClient* client);
	static	void	Check4BadMod(CUpDownClient* client);
	static	void	Check4BadName(CUpDownClient* client);
//<<< Common AntiLeech
//>>> AntiLeech.dat File Support
//>>> CRYPTO
	static	void	CheckKeys();
	static	CStringList	strTrustedPubKeys;
//<<< CRYPTO

	static	CRBMap<CString, CBaninfo> m_BadVerList;
	static	CRBMap<CString, CBaninfo> m_BadModList;
	static	CRBMap<CString, CBaninfo> m_BadNameList;
#define	DFLT_ANTILEECH_FILENAME	_T("antileech.dat")
	static	CString GetDefaultFilePath();	
	static	bool	LoadFromFile(const CString& path = _T(""), const bool& bMerge = false);
	static	bool	SaveToFile(const CString& path = _T(""));
	static	void	RemoveAllBadVers();
	static	void	RemoveAllBadMods();
	static	void	RemoveAllBadNames();
	static	bool	FindInBadVers(const CString& s);
	static	bool	FindInBadMods(const CString& s);
	static	bool	FindInBadNames(const CString& s);
	static	bool	IsBadVer(const CString& s);
	static	bool	IsBadMod(const CString& s);
	static	bool	IsBadName(const CString& s);
	static	bool	AddBadVer(const CString& s, const CString& reason, const bool& b = true);
	static	bool	AddBadMod(const CString& s, const CString& reason, const bool& b = true, const bool& c = false);
	static	bool	AddBadName(const CString& s, const CString& reason, const bool& b = true, const bool& c = false);
//<<< AntiLeech.dat File Support
};

extern CAntiLeech theAntiLeechClass;
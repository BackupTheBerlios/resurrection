#include "stdafx.h"
#include "MD4.h"
#include "SHA.h"
#include <crypto.v52.1/sha.h>
#include <crypto.v52.1/md4.h>

bool CheckHashingImplementations()
{
	static const BYTE _szData[] = "eMule Hash Verification Test Data";

	// Create SHA hash with Crypto Library
	BYTE crypto_sha1_digest[20];
	{
		CryptoPP::SHA1 sha;
		sha.Update(_szData, sizeof _szData);
		sha.Final(crypto_sha1_digest);
	}

	// Create SHA hash with our code
	{
		CSHA sha;
		sha.Add(_szData, sizeof _szData);
		sha.Finish();
		SHA1 digest;
		sha.GetHash(&digest);
		if (memcmp(digest.b, crypto_sha1_digest, sizeof crypto_sha1_digest) != 0)
		{
			AfxMessageBox(_T("Fatal Error: Implementation of SHA hashing is corrupted!"));
			return false;
		}
	}

	// Create MD4 hash with Crypto Library
	BYTE crypto_md4_digest[16];
	{
		CryptoPP::MD4 md4;
		md4.Update(_szData, sizeof _szData);
		md4.Final(crypto_md4_digest);
	}

	// Create MD4 hash with our code
	{
		CMD4 md4;
		md4.Add(_szData, sizeof _szData);
		md4.Finish();
		if (memcmp(md4.GetHash(), crypto_md4_digest, sizeof crypto_md4_digest) != 0)
		{
			AfxMessageBox(_T("Fatal Error: Implementation of MD4 hashing is corrupted!"));
			return false;
		}
	}

	return true;
}

bool SelfTest()
{
	if (!CSHA::VerifyImplementation()){
		AfxMessageBox(_T("Fatal Error: SHA hash implementation is corrupted!"));
		return false; // DO *NOT* START !!!
	}

	if (!CMD4::VerifyImplementation()){
		AfxMessageBox(_T("Fatal Error: MD4 hash implementation is corrupted!"));
		return false; // DO *NOT* START !!!
	}

	if (!CheckHashingImplementations()){
		return false; // DO *NOT* START !!!
	}

	return true;
}

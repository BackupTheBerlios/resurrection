//--- xrmb:funnynick ---
#pragma once

// ==> FunnyNick Tag - Stulle
enum FnTagSelection {
	CS_NONE = 0,
	CS_SHORT,
	CS_FULL,
	CS_CUST
};
// <== FunnyNick Tag - Stulle

class CFunnyNick
{
public:
	CFunnyNick();
	TCHAR *gimmeFunnyNick(const uchar *id);

private:
	CStringList p;
	CStringList s;
	CString tag; // FunnyNick Tag - Stulle
};

extern CFunnyNick funnyNick;

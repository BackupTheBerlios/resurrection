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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
#include "BarShader.h"

class CSafeMemFile; //KTS hideOS
class CKnownFile;//KTS hideOS
//KTS- Hideos 

class CStatisticFile
{
	friend class CKnownFile;
	friend class CPartFile;
public:
	CStatisticFile()
	{
		requested = 0;
		transferred = 0;
		accepted = 0;
		alltimerequested = 0;
		alltimetransferred = 0;
		alltimeaccepted = 0;
//<<-- ADDED STORMIT - Morph: PowerShare, Reduce SpreadBar CPU consumption //
		InChangedSpreadSortValue = false;
		InChangedFullSpreadCount = false;
		InChangedSpreadBar = false;
		lastSpreadSortValue = 0;;
		lastFullSpreadCount = 0;
//<<-- ADDED STORMIT - Morph: PowerShare, Reduce SpreadBar CPU consumption //


	}

	void	MergeFileStats( CStatisticFile* toMerge );
	void	AddRequest();
	void	AddAccepted();
	//KTS+ Spreadbars
	void	AddTransferred(uint32 start, uint32 bytes);
	void	AddBlockTransferred(uint32 start, uint32 end, uint32 count);
	void	DrawSpreadBar(CDC* dc, RECT* rect, bool bFlat) /*const*/;
	float	GetSpreadSortValue() /*const*/;
	float	GetFullSpreadCount() /*const*/;
//<<-- ADDED STORMIT - Morph: PowerShare - SLUGFILLER: Spreadbars //
void	ResetSpreadBar();
	//KTS- Spreadbars
	//KTS+ Hideos
	//bool	HideOvershares(CSafeMemFile* file, CUpDownClient* client); 
	//KTS- Hideos

	UINT	GetRequests() const				{return requested;}
	UINT	GetAccepts() const				{return accepted;}
	uint64	GetTransferred() const			{return transferred;}
	UINT	GetAllTimeRequests() const		{return alltimerequested;}
	UINT	GetAllTimeAccepts() const		{return alltimeaccepted;}
	uint64	GetAllTimeTransferred() const	{return alltimetransferred;}
	CKnownFile* fileParent;

private:
//<<-- ADDED STORMIT - Morph: PowerShare // SLUGFILLER: Spreadbars
	CRBMap<uint64, uint64> spreadlist;
	static CBarShader s_SpreadBar;
	bool	InChangedSpreadSortValue;
	bool	InChangedFullSpreadCount;
	bool	InChangedSpreadBar;
	CBitmap m_bitmapSpreadBar;
	int		lastSize;
	bool	lastbFlat;
	float	lastSpreadSortValue;
	float	lastFullSpreadCount;
//<<-- ADDED STORMIT - Morph: PowerShare //

	uint32 requested;
	uint32 accepted;
	uint64 transferred;
	uint32 alltimerequested;
	uint64 alltimetransferred;
	uint32 alltimeaccepted;
};

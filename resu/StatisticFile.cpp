// parts of this file are based on work from pan One (http://home-3.tiscali.nl/~meost/pms/)
//this file is part of eMule
//Copyright (C)2002-2004 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "StatisticFile.h"
#include "emule.h"
#include "KnownFileList.h"
#include "SharedFileList.h"
#include "KnownFile.h" //AcKroNiC MoD - Aggiunto da .:Barbas - The Demon Of Time:. - PowerRelease
#include "Uploadqueue.h" //MORPH START - Added by SiRoB, Equal Chance For Each File
#include "Statistics.h" //MORPH START - Added by SiRoB, Equal Chance For Each File
#include "Preferences.h" //MORPH START - Added by SiRoB, Equal Chance For Each File
//KTS+ Hideos / Spreadbar
#include "Preferences.h"
#include "knownfile.h"
//KTS- Hideos / Spreadbar

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//KTS- Hideos / Spreadbar
class CKnownFile;
//KTS- Hideos / Spreadbar
void CStatisticFile::MergeFileStats( CStatisticFile *toMerge )
{
	requested += toMerge->GetRequests();
	accepted += toMerge->GetAccepts();
	transferred += toMerge->GetTransferred();
	alltimerequested += toMerge->GetAllTimeRequests();
	alltimetransferred += toMerge->GetAllTimeTransferred();
	alltimeaccepted += toMerge->GetAllTimeAccepts();
 //<<-- ADDED STORMIT - Morph: PowerShare //
	// SLUGFILLER: Spreadbars
	if (!toMerge->spreadlist.IsEmpty()) {
		POSITION pos = toMerge->spreadlist.GetHeadPosition();
		uint32 start = toMerge->spreadlist.GetKeyAt(pos);
		uint32 count = toMerge->spreadlist.GetValueAt(pos);
		toMerge->spreadlist.GetNext(pos);
		while (pos){
			uint32 end = toMerge->spreadlist.GetKeyAt(pos);
			if (count)
				AddBlockTransferred(start, end, count);
			start = end;
			count = toMerge->spreadlist.GetValueAt(pos);
			toMerge->spreadlist.GetNext(pos);
		}
	}
//<<-- ADDED STORMIT - Morph: PowerShare //
	
}

void CStatisticFile::AddRequest(){
	requested++;
	alltimerequested++;
	theApp.knownfiles->requested++;
	theApp.sharedfiles->UpdateFile(fileParent);
}
	
void CStatisticFile::AddAccepted(){
	accepted++;
	alltimeaccepted++;
	theApp.knownfiles->accepted++;
	theApp.sharedfiles->UpdateFile(fileParent);
}
	
void CStatisticFile::AddTransferred(uint32 start, uint32 bytes){ //KTS Spreadbars
	transferred += bytes;
	alltimetransferred += bytes;
	theApp.knownfiles->transferred += bytes;
AddBlockTransferred(start, start+bytes+1, 1);	//KTS Spreadbars
	theApp.sharedfiles->UpdateFile(fileParent);
}
//KTS+ Spreadbars
void CStatisticFile::AddBlockTransferred(uint32 start, uint32 end, uint32 count){
	if (start >= end || !count)
		return;

	//MORPH	Start	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	if(fileParent->GetSpreadbarSetStatus() == 0 || (fileParent->GetSpreadbarSetStatus() == -1 && thePrefs.GetSpreadbarSetStatus() == 0))
		return;
	//MORPH	End	- Added by AndCycle, SLUGFILLER: Spreadbars - per file

	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	InChangedSpreadSortValue = false;
	InChangedFullSpreadCount = false;
	InChangedSpreadBar = false;
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption

		if (spreadlist.IsEmpty())
		spreadlist.SetAt(0, 0);

	POSITION endpos = spreadlist.FindFirstKeyAfter(end+1);

	if (endpos)
		spreadlist.GetPrev(endpos);
	else
		endpos = spreadlist.GetTailPosition();

	ASSERT(endpos != NULL);

	uint32 endcount = spreadlist.GetValueAt(endpos);
	endpos = spreadlist.SetAt(end, endcount);

	POSITION startpos = spreadlist.FindFirstKeyAfter(start+1);

	for (POSITION pos = startpos; pos != endpos; spreadlist.GetNext(pos)) {
		spreadlist.SetValueAt(pos, spreadlist.GetValueAt(pos)+count);
		}

	spreadlist.GetPrev(startpos);

	ASSERT(startpos != NULL);

	uint32 startcount = spreadlist.GetValueAt(startpos)+count;
	startpos = spreadlist.SetAt(start, startcount);

	POSITION prevpos = startpos;
	spreadlist.GetPrev(prevpos);
	if (prevpos && spreadlist.GetValueAt(prevpos) == startcount)
		spreadlist.RemoveAt(startpos);

	prevpos = endpos;
	spreadlist.GetPrev(prevpos);
	if (prevpos && spreadlist.GetValueAt(prevpos) == endcount)
		spreadlist.RemoveAt(endpos);
			}

CBarShader CStatisticFile::s_SpreadBar(16);

//MORPH START - Changed by SiRoB, Reduce SpreadBar CPU consumption
void CStatisticFile::DrawSpreadBar(CDC* dc, RECT* rect, bool bFlat) /*const*/
{
	int iWidth=rect->right - rect->left;
	if (iWidth <= 0)	return;
	int iHeight=rect->bottom - rect->top;
	uint32 filesize = fileParent->GetFileSize()?fileParent->GetFileSize():1;
	if (m_bitmapSpreadBar == (HBITMAP)NULL)
		VERIFY(m_bitmapSpreadBar.CreateBitmap(1, 1, 1, 8, NULL)); 
	CDC cdcStatus;
	HGDIOBJ hOldBitmap;
	cdcStatus.CreateCompatibleDC(dc);

	if(!InChangedSpreadBar || lastSize!=iWidth || lastbFlat!= bFlat){
		InChangedSpreadBar = true;
		lastSize=iWidth;
		lastbFlat=bFlat;
			m_bitmapSpreadBar.DeleteObject();
		m_bitmapSpreadBar.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
		m_bitmapSpreadBar.SetBitmapDimension(iWidth,  iHeight); 
		hOldBitmap = cdcStatus.SelectObject(m_bitmapSpreadBar);
			
		s_SpreadBar.SetHeight(iHeight);
		s_SpreadBar.SetWidth(iWidth);
			
		s_SpreadBar.SetFileSize(filesize);
		s_SpreadBar.Fill(RGB(0, 0, 0));

		for(POSITION pos = spreadlist.GetHeadPosition(); pos; ){
			uint32 count = spreadlist.GetValueAt(pos);
			uint32 start = spreadlist.GetKeyAt(pos);
			spreadlist.GetNext(pos);
			if (!pos)
				break;
			uint32 end = spreadlist.GetKeyAt(pos);
			if (count)
				s_SpreadBar.FillRange(start, end, RGB(0,
				(232<22*count)? 0:232-22*count
				,255));
		}
		s_SpreadBar.Draw(&cdcStatus, 0, 0, bFlat);
		}
	else
		hOldBitmap = cdcStatus.SelectObject(m_bitmapSpreadBar);
	dc->BitBlt(rect->left, rect->top, iWidth, iHeight, &cdcStatus, 0, 0, SRCCOPY);
	cdcStatus.SelectObject(hOldBitmap);
		}
//MORPH END  - Changed by SiRoB, Reduce SpreadBar CPU consumption
float CStatisticFile::GetSpreadSortValue() /*const*/
{
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	if (InChangedSpreadSortValue) return lastSpreadSortValue;
	InChangedSpreadSortValue=true;
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	float avg, calc;
	uint32 total = 0;
	uint32 filesize = fileParent->GetFileSize();

	if (!filesize || spreadlist.IsEmpty())
		return 0;

	POSITION pos = spreadlist.GetHeadPosition();
	uint32 start = spreadlist.GetKeyAt(pos);
	uint32 count = spreadlist.GetValueAt(pos);
	spreadlist.GetNext(pos);
	while (pos){
		uint32 end = spreadlist.GetKeyAt(pos);
		total += (end-start)*count;
		start = end;
		count = spreadlist.GetValueAt(pos);
		spreadlist.GetNext(pos);
	}

	avg = (float)total/filesize;
	calc = 0;
	pos = spreadlist.GetHeadPosition();
	start = spreadlist.GetKeyAt(pos);
	count = spreadlist.GetValueAt(pos);
		spreadlist.GetNext(pos);
	while (pos){
		uint32 end = spreadlist.GetKeyAt(pos);
		if ((float)count > avg)
			calc += avg*(end-start);
		else
			calc += count*(end-start);
		start = end;
		count = spreadlist.GetValueAt(pos);
		spreadlist.GetNext(pos);
		}
	calc /= filesize;
	//MORPH START - Added by SiRoB, Reduce Spread CPU consumption
	lastSpreadSortValue = calc;
	return calc;
	//MORPH START - Added by SiRoB, Reduce Spread CPU consumption
	}

float CStatisticFile::GetFullSpreadCount() /*const*/
{
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	if (InChangedFullSpreadCount) return lastFullSpreadCount;
	InChangedFullSpreadCount=true;
	//MORPH START - Added by SiRoB, Reduce SpreedBar CPU consumption
	float next;
	uint32 min;
	uint32 filesize = fileParent->GetFileSize();

	if (!filesize || spreadlist.IsEmpty())
		return 0;

	POSITION pos = spreadlist.GetHeadPosition();
	min = spreadlist.GetValueAt(pos);
	spreadlist.GetNext(pos);
	while (pos && spreadlist.GetKeyAt(pos) < filesize){
		uint32 count = spreadlist.GetValueAt(pos);
		if (min > count)
			min = count;
		spreadlist.GetNext(pos);
	}

	next = 0;
	pos = spreadlist.GetHeadPosition();
	uint32 start = spreadlist.GetKeyAt(pos);
	uint32 count = spreadlist.GetValueAt(pos);
		spreadlist.GetNext(pos);
	while (pos){
		uint32 end = spreadlist.GetKeyAt(pos);
		if (count > min)
			next += end-start;
		start = end;
		count = spreadlist.GetValueAt(pos);
		spreadlist.GetNext(pos);
	}
	next /= filesize;
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	return lastFullSpreadCount = min+next;
	//MORPH END   - Added by SiRoB, Reduce SpreadBar CPU consumption
	}
void CStatisticFile::ResetSpreadBar()
{
	spreadlist.RemoveAll();
	spreadlist.SetAt(0, 0);
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	InChangedSpreadSortValue = false;
	InChangedFullSpreadCount = false;
	InChangedSpreadBar = false;
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	return;
}
//KTS- Spreadbars

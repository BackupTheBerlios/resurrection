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
#include "SettingsSaver.h"
#include "PartFile.h"
#include "emule.h"
#include "preferences.h"
#include "log.h"

CSettingsSaver::CSettingsSaver(void)
{
}

CSettingsSaver::~CSettingsSaver(void)
{
}

void CSettingsSaver::DeleteFile(CPartFile* file)
{
	CString datafilepath;
	datafilepath.Format(_T("%s\\%s.ahl"), thePrefs.GetTempDir(), file->GetPartMetFileName());
	if (_tremove(datafilepath)) if (errno != ENOENT)
		AddLogLine(true, _T("Failed to delete %s, you will need to do this by hand"), datafilepath);    
}

void CSettingsSaver::LoadSettings(CPartFile* file)
{
	SettingsList daten;
	CString datafilepath;
	
	datafilepath.Format(_T("%s\\%s.ahl"), thePrefs.GetTempDir(), file->GetPartMetFileName());

	CString strLine;
	CStdioFile f;
	if (!f.Open(datafilepath, CFile::modeReadWrite | CFile::typeText))
		return;
	while(f.ReadString(strLine))
	{
		if (strLine.GetAt(0) == '#')
			continue;
		int pos = strLine.Find(_T('\0'));
		if (pos == -1)
			continue;
		CString strData = strLine.Left(pos);
		CSettingsData* newdata = new CSettingsData(_tstol(strData));
		daten.AddTail(newdata);
	}
	f.Close();

	if(daten.GetCount() < 3){
		while (!daten.IsEmpty()) 
			delete daten.RemoveHead();
		return;
	}

	POSITION pos = daten.GetHeadPosition();
	if(!pos)
		return;

	file->SetFileHardLimit(((CSettingsData*)daten.GetAt(pos))->dwData);
	daten.GetNext(pos); 
	file->SetUseAutoHL(((CSettingsData*)daten.GetAt(pos))->dwData!=0);
	daten.GetNext(pos);
	//thePrefs.m_iAutoHLUpdateTimer = ((CSettingsData*)daten.GetAt(pos))->dwData;
	//daten.GetNext(pos);
	while (!daten.IsEmpty()) 
		delete daten.RemoveHead();
}

void CSettingsSaver::SaveSettings(CPartFile* file)
{
	CString datafilepath;
	datafilepath.Format(_T("%s\\%s.ahl"), thePrefs.GetTempDir(), file->GetPartMetFileName());
	
	CString strLine;
	CStdioFile f;

	if (!f.Open(datafilepath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return;

	f.WriteString(_T("#Sivka File Settings:\n"));
	strLine.Format(_T("%ld\n"), file->GetFileHardLimit());
	f.WriteString(strLine);
	strLine.Format(_T("%ld\n"), file->UseAutoHL());
	f.WriteString(strLine);
	//strLine.Format(_T("%ld\n"), thePrefs.GetAutoHLUpdateTimer());
	//f.WriteString(strLine);
	
	f.Close();
}
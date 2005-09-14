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

#include "stdafx.h"
#include "emule.h"
#include "SivkaFileSettings.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CSivkaFileSettings, CDialog)
CSivkaFileSettings::CSivkaFileSettings()
	: CDialog(CSivkaFileSettings::IDD, 0)
{
}

CSivkaFileSettings::~CSivkaFileSettings()
{
}

void CSivkaFileSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSivkaFileSettings, CDialog)
	ON_BN_CLICKED(IDC_AHL_TIMERLABEL, OnBnClickedEnableAutoHL)
	ON_BN_CLICKED(IDC_TAKEOVER, OnBnClickedTakeOver)
	ON_BN_CLICKED(IDC_DEFAULT_BUTTON, OnBnClickedSwitch)
	ON_BN_CLICKED(IDC_HARDLIMIT_TAKEOVER, OnBnClickedMaxSourcesPerFileTakeOver)
	ON_BN_CLICKED(IDC_AHL_TIMERLABEL_TAKEOVER, OnBnClickedEnableAutoHLTakeOver)
	ON_BN_CLICKED(IDC_AHL_TIMER_TAKEOVER, OnBnClickedAutoHL_TimerTakeOver)
END_MESSAGE_MAP()

BOOL CSivkaFileSettings::OnInitDialog()
{
	thePrefs.m_MaxSourcesPerFileTakeOver = false;
	thePrefs.m_EnableAutoHLTakeOver = false;
	thePrefs.m_AutoHL_TimerTakeOver = false;

	m_RestoreDefault = false;
	m_bGlobalLimit = false;
	thePrefs.m_TakeOverFileSettings = false;
	
	CDialog::OnInitDialog();
	LoadSettings();
	Localize();

	return TRUE;
}

void CSivkaFileSettings::LoadSettings(void)
{
	if(m_hWnd)
	{
		CString strBuffer;
		
		strBuffer.Format(_T("%u"), thePrefs.m_MaxSourcesPerFileTemp);
		GetDlgItem(IDC_HARDLIMIT)->SetWindowText(strBuffer);
		CheckDlgButton(IDC_HARDLIMIT_TAKEOVER, thePrefs.m_MaxSourcesPerFileTakeOver);
		OnBnClickedMaxSourcesPerFileTakeOver();
		
		CheckDlgButton(IDC_AHL_TIMERLABEL, thePrefs.m_EnableAutoHLTemp);
		CheckDlgButton(IDC_AHL_TIMERLABEL_TAKEOVER, thePrefs.m_EnableAutoHLTakeOver);
		strBuffer.Format(_T("%u"), thePrefs.m_AutoHL_TimerTemp);
		GetDlgItem(IDC_AHL_TIMER)->SetWindowText(strBuffer);
		OnBnClickedEnableAutoHLTakeOver();
	}
}

void CSivkaFileSettings::OnBnClickedTakeOver()
{
	CString buffer;

	thePrefs.m_MaxSourcesPerFileTakeOver = IsDlgButtonChecked(IDC_HARDLIMIT_TAKEOVER)!=0;
	if(GetDlgItem(IDC_HARDLIMIT)->GetWindowTextLength() 
		&& thePrefs.m_MaxSourcesPerFileTakeOver)
	{
		GetDlgItem(IDC_HARDLIMIT)->GetWindowText(buffer);
		thePrefs.m_MaxSourcesPerFileTemp = max(_tstoi(buffer), 1);
	}
	thePrefs.m_EnableAutoHLTakeOver = IsDlgButtonChecked(IDC_AHL_TIMERLABEL_TAKEOVER)!=0;
	if(thePrefs.m_EnableAutoHLTakeOver)
		thePrefs.m_EnableAutoHLTemp = IsDlgButtonChecked(IDC_AHL_TIMERLABEL)!=0;
	thePrefs.m_AutoHL_TimerTakeOver = IsDlgButtonChecked(IDC_AHL_TIMER_TAKEOVER)!=0;
	if(GetDlgItem(IDC_AHL_TIMER)->GetWindowTextLength() && thePrefs.m_AutoHL_TimerTakeOver && thePrefs.m_EnableAutoHLTemp)
	{
		GetDlgItem(IDC_AHL_TIMER)->GetWindowText(buffer);
		if (_tstoi(buffer) >= 10 && _tstoi(buffer) <= 600)
			thePrefs.m_AutoHL_TimerTemp = _tstoi(buffer);
	}

	LoadSettings();

	thePrefs.m_TakeOverFileSettings = true;
	if (m_RestoreDefault 
		&& thePrefs.m_MaxSourcesPerFileTakeOver 
		&& thePrefs.m_EnableAutoHLTakeOver 
		&& thePrefs.m_EnableAutoHLTemp == thePrefs.IsUseAutoHL()
		&& thePrefs.m_MaxSourcesPerFileTemp == thePrefs.maxsourceperfile 
		&& !thePrefs.m_EnableAutoHLTemp)
		m_bGlobalLimit = true;
	else
		m_bGlobalLimit = false;
//	CDialog::OnOK();
}

void CSivkaFileSettings::OnBnClickedSwitch()
{
	if(m_RestoreDefault)
	{
		LoadSettings();
		GetDlgItem(IDC_DEFAULT_BUTTON)->SetWindowText(GetResString(IDS_DEFAULT));
		m_RestoreDefault = false;
	}
	else
	{
		SetWithDefaultValues();
		GetDlgItem(IDC_DEFAULT_BUTTON)->SetWindowText(GetResString(IDS_MAIN_POPUP_RESTORE));
		m_RestoreDefault = true;
	}
}

void CSivkaFileSettings::Localize(void)
{	
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_SIVKAFILESETTINGS));
		GetDlgItem(IDC_HARDLIMIT_LABEL)->SetWindowText(GetResString(IDS_PW_MAXSOURCES));
		GetDlgItem(IDC_AHL_TIMERLABEL)->SetWindowText(GetResString(IDS_AHL_TIMERLABEL));
		GetDlgItem(IDC_TAKEOVER)->SetWindowText(GetResString(IDS_TAKEOVER));
		GetDlgItem(IDC_DEFAULT_BUTTON)->SetWindowText(GetResString(IDS_DEFAULT));
		GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_FD_CLOSE));
	}
}

void CSivkaFileSettings::SetWithDefaultValues()
{
	if(m_hWnd)
	{
		CString strBuffer;

		GetDlgItem(IDC_HARDLIMIT_LABEL)->EnableWindow(true);
		GetDlgItem(IDC_HARDLIMIT)->EnableWindow(true);
		strBuffer.Format(_T("%u"), thePrefs.GetMaxSourcePerFile());
		GetDlgItem(IDC_HARDLIMIT)->SetWindowText(strBuffer);
		CheckDlgButton(IDC_HARDLIMIT_TAKEOVER, true);
		
		GetDlgItem(IDC_AHL_TIMER)->EnableWindow(thePrefs.IsUseAutoHL());
		GetDlgItem(IDC_AHL_TIMERLABEL)->EnableWindow(true);
		CheckDlgButton(IDC_AHL_TIMERLABEL, thePrefs.IsUseAutoHL());
		CheckDlgButton(IDC_AHL_TIMERLABEL_TAKEOVER, true);
		strBuffer.Format(_T("%u"), thePrefs.GetAutoHLUpdateTimer());
		GetDlgItem(IDC_AHL_TIMER)->SetWindowText(strBuffer);
		GetDlgItem(IDC_AHL_TIMER_TAKEOVER)->EnableWindow(thePrefs.IsUseAutoHL());
		CheckDlgButton(IDC_AHL_TIMER_TAKEOVER, thePrefs.IsUseAutoHL());

	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void CSivkaFileSettings::OnBnClickedMaxSourcesPerFileTakeOver()
{
	GetDlgItem(IDC_HARDLIMIT)->EnableWindow(IsDlgButtonChecked(IDC_HARDLIMIT_TAKEOVER));
	GetDlgItem(IDC_HARDLIMIT_LABEL)->EnableWindow(IsDlgButtonChecked(IDC_HARDLIMIT_TAKEOVER));
}
void CSivkaFileSettings::OnBnClickedEnableAutoHLTakeOver()
{
	GetDlgItem(IDC_AHL_TIMERLABEL)->EnableWindow(IsDlgButtonChecked(IDC_AHL_TIMERLABEL_TAKEOVER));
	OnBnClickedEnableAutoHL();
}

void CSivkaFileSettings::OnBnClickedEnableAutoHL()
{
	GetDlgItem(IDC_AHL_TIMER_TAKEOVER)->EnableWindow(IsDlgButtonChecked(IDC_AHL_TIMERLABEL) && IsDlgButtonChecked(IDC_AHL_TIMERLABEL_TAKEOVER));
	CheckDlgButton(IDC_AHL_TIMER_TAKEOVER, IsDlgButtonChecked(IDC_AHL_TIMERLABEL) && IsDlgButtonChecked(IDC_AHL_TIMERLABEL_TAKEOVER) && thePrefs.m_AutoHL_TimerTakeOver);
	OnBnClickedAutoHL_TimerTakeOver();
}

void CSivkaFileSettings::OnBnClickedAutoHL_TimerTakeOver()
{
	GetDlgItem(IDC_AHL_TIMER)->EnableWindow(IsDlgButtonChecked(IDC_AHL_TIMERLABEL) && IsDlgButtonChecked(IDC_AHL_TIMERLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_AHL_TIMER_TAKEOVER));
}

//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "PPgAntiLeech.h"
#include "AntiLeech.h"
#include "UserMsgs.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

CString GetYesNo(const bool& b)	
{
	if(b)
		return GetResString(IDS_YES);
	return GetResString(IDS_NO);
}

IMPLEMENT_DYNAMIC(CPPgAntiLeech, CDialog)
BEGIN_MESSAGE_MAP(CPPgAntiLeech, CResizableDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ANTILEECH_LIST, OnLvnItemchangedAntileechList)
	ON_BN_CLICKED(IDC_ANTILEECH_BAN, OnSettingsChange)
	ON_BN_CLICKED(IDC_ANTILEECH_CASE, OnSettingsChange)
	ON_EN_CHANGE(IDC_ANTILEECH_REASON, OnSettingsChange)
	ON_BN_CLICKED(IDC_APPLY, OnApplyBtn)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CPPgAntiLeech::CPPgAntiLeech()
: CResizableDialog(CPPgAntiLeech::IDD)
{
	m_bChanged = false;
}

CPPgAntiLeech::~CPPgAntiLeech()
{
	m_lAntiLeech.DeleteAllItems();
}

void CPPgAntiLeech::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANTILEECH_LIST, m_lAntiLeech);
}

BOOL CPPgAntiLeech::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);

	// TODO: Add extra initialization for CListCtrl here
	m_lAntiLeech.SetExtendedStyle(m_lAntiLeech.GetStyle()|LVS_EX_CHECKBOXES);
	m_lAntiLeech.InsertColumn(0,_T("ToBan"), LVCFMT_LEFT, 150);
	m_lAntiLeech.InsertColumn(1,_T("Kind"), LVCFMT_LEFT, 100);
	m_lAntiLeech.InsertColumn(2,_T("Reason"), LVCFMT_LEFT, 200);
	m_lAntiLeech.InsertColumn(3,_T("Banned"), LVCFMT_LEFT, 50);
	m_lAntiLeech.InsertColumn(4,_T("Case"), LVCFMT_LEFT, 50);

	AddAnchor(IDC_APPLY, BOTTOM_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);

	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgAntiLeech::LoadSettings(void)
{
#define VERLIST theAntiLeechClass.m_BadVerList
	for(POSITION pos = VERLIST.GetHeadPosition(); pos; VERLIST.GetNext(pos))
	{
		int nIndex = m_lAntiLeech.InsertItem(0, VERLIST.GetKeyAt(pos));
		m_lAntiLeech.SetItemText(nIndex, 1, _T("Ver"));

		CBaninfo info = VERLIST.GetValueAt(pos);
		m_lAntiLeech.SetItemText(nIndex, 2, info.GetReason());
		m_lAntiLeech.SetItemText(nIndex, 3, GetYesNo(info.Ban()));
		m_lAntiLeech.SetItemText(nIndex, 4, GetYesNo(info.UseCase()));
		//m_lAntiLeech.SetCheck(nIndex, info.Ban());
		m_lAntiLeech.SetCheck(nIndex, false);
	}

#define MODLIST theAntiLeechClass.m_BadModList
	for(POSITION pos = MODLIST.GetHeadPosition(); pos; MODLIST.GetNext(pos))
	{
		int nIndex = m_lAntiLeech.InsertItem(0, MODLIST.GetKeyAt(pos));
		m_lAntiLeech.SetItemText(nIndex, 1, _T("Mod"));
		
		CBaninfo info = MODLIST.GetValueAt(pos);
		m_lAntiLeech.SetItemText(nIndex, 2, info.GetReason());
		m_lAntiLeech.SetItemText(nIndex, 3, GetYesNo(info.Ban()));
		m_lAntiLeech.SetItemText(nIndex, 4, GetYesNo(info.UseCase()));
		//m_lAntiLeech.SetCheck(nIndex, info.Ban());
		m_lAntiLeech.SetCheck(nIndex, false);
	}

#define NAMELIST theAntiLeechClass.m_BadNameList
	for(POSITION pos = NAMELIST.GetHeadPosition(); pos; NAMELIST.GetNext(pos))
	{
		int nIndex = m_lAntiLeech.InsertItem(0, NAMELIST.GetKeyAt(pos));
		m_lAntiLeech.SetItemText(nIndex, 1, _T("Name"));

		CBaninfo info = NAMELIST.GetValueAt(pos);
		m_lAntiLeech.SetItemText(nIndex, 2, info.GetReason());
		m_lAntiLeech.SetItemText(nIndex, 3, GetYesNo(info.Ban()));
		m_lAntiLeech.SetItemText(nIndex, 4, GetYesNo(info.UseCase()));
		//m_lAntiLeech.SetCheck(nIndex, info.Ban());
		m_lAntiLeech.SetCheck(nIndex, false);
	}
}

BOOL CPPgAntiLeech::OnApply()
{
	if(!IsModified())
		//return CResizableDialog::OnApply();
		return FALSE;
	PreSetModified(0);

	for(int i = 0; i < m_lAntiLeech.GetItemCount(); ++i)
	{		
		if(m_lAntiLeech.GetCheck(i)!=0)
		{
			m_lAntiLeech.SetCheck(i, FALSE);
			const CString name = m_lAntiLeech.GetItemText(i, 0);		
			int bMod = 0;
			if(m_lAntiLeech.GetItemText(i, 1)==_T("Mod"))
				bMod = 1;
			else if(m_lAntiLeech.GetItemText(i, 1)==_T("Ver"))
				bMod = 2;
			//const CString reason = m_lAntiLeech.GetItemText(i, 2);
			//const bool bBan = m_lAntiLeech.GetItemText(i, 3)==GetResString(IDS_YES);
			//const bool bCase = m_lAntiLeech.GetItemText(i, 4)==GetResString(IDS_YES);
			const bool bBan = IsDlgButtonChecked(IDC_ANTILEECH_BAN)!=0;
			m_lAntiLeech.SetItemText(i, 3, GetYesNo(bBan));			
			const bool bCase = IsDlgButtonChecked(IDC_ANTILEECH_CASE)!=0;			
			m_lAntiLeech.SetItemText(i, 4, GetYesNo(bCase));
			CString reason;
			GetDlgItem(IDC_ANTILEECH_REASON)->GetWindowText(reason);
			CBaninfo info(reason, bBan, bCase);
			if(reason.IsEmpty())
				info.strReason = m_lAntiLeech.GetItemText(i, 2); //keep old val
			else
				m_lAntiLeech.SetItemText(i, 2, reason);
			if(bMod == 1)
				MODLIST.SetAt(name, info);
			else if(bMod == 2)
				VERLIST.SetAt(name, info);
			else
				NAMELIST.SetAt(name, info);
		}
	}

	//return CResizableDialog::OnApply();
	return TRUE;
}

void CPPgAntiLeech::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(_T("AntiLeech"));
		SetDlgItemText(IDC_APPLY,_T("Apply"));
		SetDlgItemText(IDOK,GetResString(IDS_FD_CLOSE));
	}
}

void CPPgAntiLeech::OnDestroy()
{
	CResizableDialog::OnDestroy();
}

void CPPgAntiLeech::OnLvnItemchangedAntileechList(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	OnSettingsChange();
	*pResult = 0;
}

BOOL CPPgAntiLeech::PreTranslateMessage(MSG* pMsg)
{
	//This should allow us to overtake settings by ENTER
	if(pMsg->message == WM_KEYDOWN)
	{
		UINT nChar = pMsg->wParam;
		if (nChar == 'A' && ::GetAsyncKeyState(VK_CONTROL)<0)
		{
			for(int i = 0; i < m_lAntiLeech.GetItemCount(); ++i)
				m_lAntiLeech.SetCheck(i, TRUE);
		}
		else if (nChar == 'N' && ::GetAsyncKeyState(VK_CONTROL)<0)
		{
			for(int i = 0; i < m_lAntiLeech.GetItemCount(); ++i)
				m_lAntiLeech.SetCheck(i, FALSE);
		}
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

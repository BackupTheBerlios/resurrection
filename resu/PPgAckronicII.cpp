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
#include "emule.h"
#include "emuleDlg.h"
#include "PPgAckronicII.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "Opcodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgAckronicII, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgAckronicII, CPropertyPage)
	ON_WM_HELPINFO()
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CPPgAckronicII::CPPgAckronicII()
	: CPropertyPage(CPPgAckronicII::IDD)
{
	m_iFileBufferSize = 0;//Buffer Size
	m_iBufferTimeLimit = 0; //FrankyFive: Buffer time limit
	UpDataratePerClient = 0;//Ackronic - Upload data rate
}

CPPgAckronicII::~CPPgAckronicII()
{
}

void CPPgAckronicII::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTL, m_ctlBufferTimeLimit); //FrankyFive: Buffer Time Limit
	DDX_Control(pDX, IDC_FILEBUFFERSIZE, m_ctlFileBuffSize);//BufferSize
	DDX_Control(pDX, IDC_UP_SLOTS_SLIDER, m_ctlUpDataratePerClient);
}

BOOL CPPgAckronicII::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
//Ackronic - Upload data rate
	uint32 MaxUpSpeed = (thePrefs.maxupload==UNLIMITED)?thePrefs.maxGraphUploadRate:thePrefs.maxupload;
	UpDataratePerClient = thePrefs.UpDataratePerClient;
	m_ctlUpDataratePerClient.SetRange(2, 12, TRUE);
	m_ctlUpDataratePerClient.SetPos(UpDataratePerClient);
//Ackronic END - Upload data rate

	//FrankyFive: Buffer Time Limit - start
	m_iBufferTimeLimit = thePrefs.m_iBufferTimeLimit;
	m_ctlBufferTimeLimit.SetRange(1, 30, TRUE);
	m_ctlBufferTimeLimit.SetPos(m_iBufferTimeLimit);
	m_ctlBufferTimeLimit.SetTicFreq(1);
	m_ctlBufferTimeLimit.SetPageSize(1);
    //FrankyFive: Buffer Time Limit - end

	m_iFileBufferSize = thePrefs.m_iFileBufferSize;
	m_ctlFileBuffSize.SetRange(16, /*1024+512*/10*1024, TRUE);//Ackronic - Modificato da Aenarion[ITA] - Ott.Opzioni
	int iMin, iMax;
	m_ctlFileBuffSize.GetRange(iMin, iMax);
	m_ctlFileBuffSize.SetPos(m_iFileBufferSize/1024);
	int iPage = 256;//Ackronic - Modificato da Aenarion[ITA] - Ott.Opzioni
	for (int i = ((iMin+iPage-1)/iPage)*iPage; i < iMax; i += iPage)
		m_ctlFileBuffSize.SetTic(i);
	m_ctlFileBuffSize.SetPageSize(iPage);

	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
void CPPgAckronicII::LoadSettings(void)
{
		
}

BOOL CPPgAckronicII::OnApply()
{
	if (!UpdateData())
		return FALSE;

///////////
	thePrefs.UpDataratePerClient = UpDataratePerClient;
///////////

	thePrefs.m_iBufferTimeLimit = m_iBufferTimeLimit; //FrankyFive: Buffer Time Limit
	thePrefs.m_iFileBufferSize = m_iFileBufferSize;//BufferSize

	LoadSettings();

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgAckronicII::Localize(void)
{
	if (m_hWnd)
	{

		//Upload slots
		GetDlgItem(IDC_UP_SLOTS_LABEL)->SetWindowText(GetResString(IDS_UP_SLOTS));
		GetDlgItem(IDC_UP_DR_PER_CLIENT_LABEL)->SetWindowText(GetResString(IDS_UP_SPEED_PER_CLIENT));		
		//end up slots
		
		CString temp;
		uint32 MaxUpSpeed = (thePrefs.maxupload==UNLIMITED)?thePrefs.maxGraphUploadRate:thePrefs.maxupload;
		//FrankyFive: Buffer Time Limit - start
		temp.Format(_T("%s: %i min"), GetResString(IDS_BTL_TEXT), m_iBufferTimeLimit);
		GetDlgItem(IDC_BTL_TEXT)->SetWindowText(temp);
		//FrankyFive: Buffer Time Limit - end
		temp.Format(_T("%s: %s"), GetResString(IDS_FILEBUFFERSIZE), CastItoXBytes(m_iFileBufferSize, false, false));
		GetDlgItem(IDC_FILEBUFFERSIZE_STATIC)->SetWindowText(temp);
		
		temp.Format(_T("%u"), UpDataratePerClient/*((CSliderCtrl*)GetDlgItem(IDC_UP_SLOTS_SLIDER))->GetPos()*/);
		GetDlgItem(IDC_UP_SLOTS)->SetWindowText(temp);
		temp.Format(_T("%u"),(MaxUpSpeed*1024)/UpDataratePerClient/*((CSliderCtrl*)GetDlgItem(IDC_UP_SLOTS_SLIDER))->GetPos()*/); 
		GetDlgItem(IDC_UP_DR_PER_CLIENT)->SetWindowText(temp);

	
	}
}
void CPPgAckronicII::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{//Quando vuoi col mouse gli indicatori delle barre succede questo:
	SetModified(TRUE);
	
	//FrankyFive: Buffer Time Limit - start
	 if (pScrollBar->GetSafeHwnd() == m_ctlBufferTimeLimit.m_hWnd) 
	{
		m_iBufferTimeLimit = ((CSliderCtrl*)pScrollBar)->GetPos();
	        CString temp;
		temp.Format(_T("%s: %i min"), GetResString(IDS_BTL_TEXT), m_iBufferTimeLimit);
		GetDlgItem(IDC_BTL_TEXT)->SetWindowText(temp);
		SetModified(TRUE);
        }
	//FrankyFive: Buffer Time Limit - end
	else if (pScrollBar->GetSafeHwnd() == m_ctlFileBuffSize.m_hWnd)
	{
		m_iFileBufferSize = m_ctlFileBuffSize.GetPos() * 1024;
        CString temp;
		temp.Format(_T("%s: %s"), GetResString(IDS_FILEBUFFERSIZE), CastItoXBytes(m_iFileBufferSize, false, false));
		GetDlgItem(IDC_FILEBUFFERSIZE_STATIC)->SetWindowText(temp);
		SetModified(TRUE);
	}
	else if (pScrollBar->GetSafeHwnd() == m_ctlUpDataratePerClient.m_hWnd)
	{
		UpDataratePerClient = m_ctlUpDataratePerClient.GetPos();//((CSliderCtrl*)pScrollBar)->GetPos();;
		CString temp;
		uint32 MaxUpSpeed = (thePrefs.maxupload==UNLIMITED)?thePrefs.maxGraphUploadRate:thePrefs.maxupload;
		temp.Format(_T("%u"),UpDataratePerClient /*((CSliderCtrl*)GetDlgItem(IDC_UP_SLOTS_SLIDER))->GetPos()*/);
		GetDlgItem(IDC_UP_SLOTS)->SetWindowText(temp);
		temp.Format(_T("%u"),(MaxUpSpeed*1024)/UpDataratePerClient /*((CSliderCtrl*)GetDlgItem(IDC_UP_SLOTS_SLIDER))->GetPos()*/); // edited by DkD
		GetDlgItem(IDC_UP_DR_PER_CLIENT)->SetWindowText(temp);
		SetModified(TRUE);
	}


	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);

}



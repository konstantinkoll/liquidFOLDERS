
// CJournalButton.cpp: Implementierung der Klasse CJournalButton
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CJournalUI
//

CJournalUI::CJournalUI()
	: CCmdUI()
{
	m_Enabled = FALSE;
}

void CJournalUI::Enable(BOOL bOn)
{
	m_Enabled = bOn;
}


// CJournalButton
//

void CJournalButton::OnClickButton(INT Index) const
{
	GetOwner()->PostMessage(WM_COMMAND, m_BarItems[Index].Command);
}


BEGIN_MESSAGE_MAP(CJournalButton, CBackstageBar)
	ON_WM_CREATE()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

INT CJournalButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	AddItem(ID_NAV_BACK, BACKSTAGEICON_ARROWLEFT);
	AddItem(ID_NAV_FORWARD, BACKSTAGEICON_ARROWRIGHT);

	return 0;
}

void CJournalButton::OnIdleUpdateCmdUI()
{
	BOOL Update = FALSE;

	for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
	{
		CJournalUI cmdUI;
		cmdUI.m_nID = m_BarItems[a].Command;
		cmdUI.DoUpdate(GetOwner(), FALSE);

		Update |= (cmdUI.m_Enabled!=m_BarItems[a].Enabled);
		m_BarItems[a].Enabled = cmdUI.m_Enabled;
	}

	if (Update)
		Invalidate();
}

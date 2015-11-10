
// CBackstageWidgets.cpp: Implementierung der Klasse CBackstageWidgets
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CBackstageWidgets
//

CBackstageWidgets::CBackstageWidgets()
	: CBackstageBar(TRUE)
{
}

void CBackstageWidgets::AddWidgetSize(LPSIZE lpSize) const
{
	ASSERT(lpSize);

	lpSize->cy += m_ButtonSize+2;

	if (IsWindow(m_hWnd))
	{
		lpSize->cx += GetPreferredWidth()-1;

		if (IsCtrlThemed())
			lpSize->cx += BACKSTAGERADIUS-4;
	}
}

void CBackstageWidgets::DrawItem(CDC& dc, CRect& rectItem, UINT Index, UINT State, BOOL Themed) const
{
	INT IconID = m_BarItems.m_Items[Index].IconID;

	if ((IconID==BACKSTAGEICON_MAXIMIZE) || (IconID==BACKSTAGEICON_RESTORE))
		m_BarItems.m_Items[Index].IconID = (GetParent()->GetStyle() & WS_MAXIMIZE) ? BACKSTAGEICON_RESTORE : BACKSTAGEICON_MAXIMIZE;

	CBackstageBar::DrawItem(dc, rectItem, Index, State, Themed);
}

void CBackstageWidgets::OnClickButton(INT Index) const
{
	UINT Command = m_BarItems.m_Items[Index].Command;

	if ((Command==SC_MAXIMIZE) && (GetParent()->GetStyle() & WS_MAXIMIZE))
		Command = SC_RESTORE;

	GetOwner()->PostMessage(WM_SYSCOMMAND, Command);
}


BEGIN_MESSAGE_MAP(CBackstageWidgets, CBackstageBar)
	ON_WM_CREATE()
END_MESSAGE_MAP()

INT CBackstageWidgets::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	const DWORD dwStyle = GetParent()->GetStyle();

	if (dwStyle & WS_MINIMIZEBOX)
		AddItem(SC_MINIMIZE, BACKSTAGEICON_MINIMIZE);

	if (dwStyle & WS_MAXIMIZEBOX)
		AddItem(SC_MAXIMIZE, BACKSTAGEICON_MAXIMIZE);

	AddItem(SC_CLOSE, BACKSTAGEICON_CLOSE, 0, TRUE);
	AdjustLayout();

	return 0;
}

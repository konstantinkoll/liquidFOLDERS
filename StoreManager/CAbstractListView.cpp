
// CAbstractListView.cpp: Implementierung der Klasse CAbstractListView
//

#include "stdafx.h"
#include "CAbstractListView.h"
#include "Resource.h"
#include "StoreManager.h"
#include "LFCore.h"


// CAbstractListView

CAbstractListView::CAbstractListView()
{
}

CAbstractListView::~CAbstractListView()
{
}

void CAbstractListView::SelectItem(INT n, BOOL select, BOOL InternalCall)
{
	if (InternalCall)
		m_FileList.ItemChanged = 1;

	m_FileList.SetItemState(n, select ? LVIS_SELECTED : 0, LVIS_SELECTED);

	if (InternalCall)
		m_FileList.ItemChanged &= 2;
}

INT CAbstractListView::GetFocusItem()
{
	return m_FileList.GetNextItem(-1, LVNI_FOCUSED);
}

INT CAbstractListView::GetSelectedItem()
{
	return m_FileList.GetNextItem(-1, LVNI_FOCUSED | LVNI_SELECTED);
}

INT CAbstractListView::GetNextSelectedItem(INT n)
{
	return m_FileList.GetNextItem(n, LVNI_SELECTED);
}

void CAbstractListView::EditLabel(INT n)
{
	m_FileList.EditLabel(n);
}

BOOL CAbstractListView::IsSelected(INT n)
{
	return m_FileList.GetItemState(n, LVIS_SELECTED);
}

BOOL CAbstractListView::IsEditing()
{
	return m_FileList.Editing;
}

BOOL CAbstractListView::HasCategories()
{
	return !m_FileList.OwnerData;
}


BEGIN_MESSAGE_MAP(CAbstractListView, CFileView)
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(ID_TOGGLE_ATTRIBUTE, ID_TOGGLE_ATTRIBUTE+LFAttributeCount-1, OnToggleAttribute)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TOGGLE_ATTRIBUTE, ID_TOGGLE_ATTRIBUTE+LFAttributeCount-1, OnUpdateToggleAttribute)
	ON_COMMAND(ID_VIEW_AUTOSIZECOLUMNS, OnAutosizeColumns)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_AUTOSIZECOLUMNS, ID_VIEW_CHOOSEDETAILS, OnUpdateCommands)
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

void CAbstractListView::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);
	m_FileList.SetFocus();
}

void CAbstractListView::OnToggleAttribute(UINT nID)
{
	theApp.ToggleAttribute(pViewParameters, nID-ID_TOGGLE_ATTRIBUTE);
	theApp.UpdateViewOptions(ActiveContextID);
}

void CAbstractListView::OnUpdateToggleAttribute(CCmdUI* pCmdUI)
{
	UINT nID = pCmdUI->m_nID-ID_TOGGLE_ATTRIBUTE;

	pCmdUI->SetCheck(m_ViewParameters.ColumnWidth[nID]>0);
	pCmdUI->Enable(!theApp.m_Attributes[nID]->AlwaysVisible);
}

void CAbstractListView::OnAutosizeColumns()
{
	for (UINT a=0; a<m_FileList.ColumnCount; a++)
	{
		m_FileList.SetColumnWidth(a, LVSCW_AUTOSIZE);
		pViewParameters->ColumnWidth[m_FileList.ColumnMapping[a]] = m_FileList.GetColumnWidth(a);
	}

	// Hier kein Broadcast an andere Fenster
}

void CAbstractListView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;
	switch (pCmdUI->m_nID)
	{
	case ID_VIEW_AUTOSIZECOLUMNS:
	case ID_VIEW_CHOOSEDETAILS:
		b = (m_FileList.GetView()==LV_VIEW_DETAILS);
		break;
	}

	pCmdUI->Enable(b);
}

void CAbstractListView::OnSysColorChange()
{
	BOOL Themed = IsCtrlThemed();
	COLORREF back = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	COLORREF text = Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT);

	m_FileList.SetBkColor(back);
	m_FileList.SetTextBkColor(back);
	m_FileList.SetTextColor(text);

	if (theApp.OSVersion==OS_XP)
	{
		LVGROUPMETRICS metrics;
		ZeroMemory(&metrics, sizeof(LVGROUPMETRICS));
		metrics.cbSize = sizeof(LVGROUPMETRICS);
		metrics.mask = LVGMF_TEXTCOLOR;
		metrics.crHeader = metrics.crLeft = text;
		m_FileList.SetGroupMetrics(&metrics);
	}
}


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

void CAbstractListView::SelectItem(int n, BOOL select, BOOL InternalCall)
{
	if (InternalCall)
		m_FileList.ItemChanged = 1;

	m_FileList.SetItemState(n, select ? LVIS_SELECTED : 0, LVIS_SELECTED);

	if (InternalCall)
		m_FileList.ItemChanged &= 2;
}

int CAbstractListView::GetFocusItem()
{
	return m_FileList.GetNextItem(-1, LVNI_FOCUSED);
}

int CAbstractListView::GetSelectedItem()
{
	return m_FileList.GetNextItem(-1, LVNI_FOCUSED | LVNI_SELECTED);
}

int CAbstractListView::GetNextSelectedItem(int n)
{
	return m_FileList.GetNextItem(n, LVNI_SELECTED);
}

void CAbstractListView::EditLabel(int n)
{
	m_FileList.EditLabel(n);
}

BOOL CAbstractListView::IsSelected(int n)
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
	ON_COMMAND(ID_VIEW_CATEGORIES, OnToggleCategories)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_CATEGORIES, ID_VIEW_AUTOSIZECOLUMNS, OnUpdateCommands)
END_MESSAGE_MAP()

void CAbstractListView::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);
	m_FileList.SetFocus();
}

void CAbstractListView::OnToggleAttribute(UINT nID)
{
	theApp.ToggleAttribute(pViewParameters, nID-ID_TOGGLE_ATTRIBUTE);
	OnViewOptionsChanged();
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

	OnViewOptionsChanged(TRUE);
}

void CAbstractListView::OnToggleCategories()
{
	pViewParameters->ShowCategories = !pViewParameters->ShowCategories;
	OnViewOptionsChanged();
}

void CAbstractListView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;
	switch (pCmdUI->m_nID)
	{
	case ID_VIEW_CATEGORIES:
		b = (m_FileList.GetView()!=LV_VIEW_LIST);
		if (result)
			b &= (result->m_HasCategories==true);
		pCmdUI->SetCheck(m_ViewParameters.ShowCategories);
		break;
	case ID_VIEW_AUTOSIZECOLUMNS:
		b = (m_FileList.GetView()==LV_VIEW_DETAILS);
		break;
	}

	pCmdUI->Enable(b);
}

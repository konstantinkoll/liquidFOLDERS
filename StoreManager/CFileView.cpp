
// CFileView.cpp: Implementierung der Klasse CFileView
//

#include "stdafx.h"
#include "CFileView.h"
#include "StoreManager.h"


// CFileView
//

CFileView::CFileView()
	: CWnd()
{
	ActiveContextID = LFContextDefault;
	ViewID = LFViewAutomatic;
	result = NULL;
	FocusItem = -1;
}

CFileView::~CFileView()
{
}

void CFileView::Create(LFSearchResult* _result, UINT _ViewID)
{
	OnUpdateViewOptions(_result->m_ContextView, _ViewID, TRUE);
	OnUpdateSearchResult(_result, 0);
}

void CFileView::OnUpdateViewOptions(int _ActiveContextID, int _ViewID, BOOL Force)
{
	if (_ActiveContextID>=0)
		ActiveContextID = _ActiveContextID;
	pViewParameters = &theApp.m_Views[ActiveContextID];

	if (_ViewID<0)
		_ViewID = ViewID;

	if (Force || (m_ViewParameters.Background!=pViewParameters->Background))
	{
		ModifyStyleEx(WS_EX_CLIENTEDGE, (pViewParameters->Background!=ChildBackground_Ribbon) ? WS_EX_CLIENTEDGE : 0);
		::SetWindowPos(GetSafeHwnd(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}

	SetViewOptions(_ViewID, Force);

	m_ViewParameters = *pViewParameters;
	RibbonColor = theApp.m_nAppLook;
	ViewID = _ViewID;
}

void CFileView::OnUpdateSearchResult(LFSearchResult* _result, int _FocusItem)
{
	SetCursor(theApp.LoadStandardCursor(_result ? IDC_ARROW : IDC_WAIT));
	result = _result;

	if (_result)
	{
		if (_result->m_Context!=LFContextStores)
		{
			m_DropTarget.Register(this);
		}
		else
		{
			m_DropTarget.Revoke();
		}

		ActiveContextID = _result->m_ContextView;
		pViewParameters = &theApp.m_Views[ActiveContextID];
		m_ViewParameters.SortBy = pViewParameters->SortBy;

		if (_FocusItem>(int)_result->m_Count-1)
			_FocusItem = (int)_result->m_Count-1;
		FocusItem = _FocusItem;
	}
	else
	{
		m_DropTarget.Revoke();
		FocusItem = -1;
	}

	SetSearchResult(_result);
}

void CFileView::SelectItem(int /*n*/, BOOL /*select*/, BOOL /*InternalCall*/)
{
}

int CFileView::GetFocusItem()
{
	return FocusItem;
}

int CFileView::GetSelectedItem()
{
	return -1;
}

int CFileView::GetNextSelectedItem(int /*n*/)
{
	return -1;
}


void CFileView::SetViewOptions(UINT /*_ViewID*/, BOOL /*Force*/)
{
}

void CFileView::SetSearchResult(LFSearchResult* /*_result*/)
{
}

BOOL CFileView::IsSelected(int /*n*/)
{
	return FALSE;
}

int CFileView::ItemAtPosition(CPoint /*point*/)
{
	return -1;
}

void CFileView::EditLabel(int /*n*/)
{
}

BOOL CFileView::IsEditing()
{
	return FALSE;
}

BOOL CFileView::HasCategories()
{
	return FALSE;
}

CMenu* CFileView::GetContextMenu()
{
	return NULL;
}

void CFileView::AppendContextMenu(CMenu* menu)
{
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_GRANNY));
	menu->AppendMenu(MF_STRING, ID_VIEW_GRANNY, tmpStr);
	menu->AppendMenu(MF_SEPARATOR);
	ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_SORTOPTIONS));
	menu->AppendMenu(MF_STRING, ID_APP_SORTOPTIONS, tmpStr);
	ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_VIEWOPTIONS));
	menu->AppendMenu(MF_STRING, ID_APP_VIEWOPTIONS, tmpStr);

	if (result)
		if (result->m_Context==LFContextStores)
		{
			menu->AppendMenu(MF_SEPARATOR);
			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_STORE_NEW));
			menu->AppendMenu(MF_STRING, ID_STORE_NEW, tmpStr);
		}
}

void CFileView::OnContextMenu(CPoint point)
{
	CMenu* menu = GetContextMenu();
	CMenu* popup;

	if (!menu)
	{
		popup = new CMenu();
		popup->CreatePopupMenu();
	}
	else
	{
		popup = menu->GetSubMenu(0);
	}

	AppendContextMenu(popup);

	CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu();
	pPopupMenu->Create(this, point.x, point.y, (HMENU)*popup);

	if (menu)
	{
		delete menu;
	}
	else
	{
		delete popup;
	}
}

void CFileView::OnItemContextMenu(int idx, CPoint point)
{
	UINT nID = 0;
	UINT cmdDefault = 0;
	LFItemDescriptor* f = result->m_Files[idx];
	switch (f->Type & LFTypeMask)
	{
	case LFTypeDrive:
		nID = IDM_DRIVE;
		cmdDefault = ID_STORE_NEWDRIVE;
		break;
	case LFTypeStore:
		nID = IDM_STORE;
		cmdDefault = ID_NAV_STARTNAVIGATION;
		break;
	case LFTypeVirtual:
		nID = (f->IconID==IDI_FLD_Back ? IDM_BACK : m_ViewParameters.Mode==LFViewGlobe ? IDM_VIRTUAL_GLOBE : IDM_VIRTUAL);
		cmdDefault = ID_NAV_STARTNAVIGATION;
		break;
	}

	if (nID)
	{
		CMenu menu;
		menu.LoadMenu(nID);
		ASSERT_VALID(&menu);

		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu();
		pPopupMenu->Create(this, point.x, point.y, (HMENU)(*menu.GetSubMenu(0)));
		pPopupMenu->SetDefaultItem(cmdDefault);
	}
}

void CFileView::OnViewOptionsChanged(BOOL LocalSettings)
{
	theApp.SaveViewOptions(ActiveContextID);
	if (!LocalSettings)
		theApp.UpdateViewOptions(ActiveContextID);
}

BOOL CFileView::HandleDefaultKeys(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	switch(nChar)
	{
	case 'A':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
		{
			GetParentFrame()->SendMessage(WM_COMMAND, ID_VIEW_SELECTALL);
			return TRUE;
		}
		break;
	case 'N':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
		{
			GetParentFrame()->SendMessage(WM_COMMAND, ID_VIEW_SELECTNONE);
			return TRUE;
		}
		break;
	case VK_SPACE:
		if (GetKeyState(VK_SHIFT)>=0)
		{
			SelectItem(GetFocusItem(), FALSE);
			return TRUE;
		}
		break;
	case VK_F2:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
		{
			EditLabel(GetFocusItem());
			return TRUE;
		}
		break;
	case VK_BACK:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
		{
			GetParentFrame()->SendMessage(WM_COMMAND, ID_NAV_BACK);
			return TRUE;
		}
		break;
	case VK_EXECUTE:
	case VK_RETURN:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
		{
			GetParentFrame()->SendMessage(WM_COMMAND, ID_NAV_STARTNAVIGATION);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

int CFileView::GetFontHeight(BOOL GrannyMode)
{
	LOGFONT lf;
	theApp.m_Fonts[FALSE][GrannyMode].GetLogFont(&lf);
	if (lf.lfHeight<0)
		lf.lfHeight = -lf.lfHeight;

	return lf.lfHeight;
}


BEGIN_MESSAGE_MAP(CFileView, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_VIEW_GRANNY, OnToggleGrannyMode)
	ON_COMMAND(ID_VIEW_SELECTALL, OnSelectAll)
	ON_COMMAND(ID_VIEW_SELECTNONE, OnSelectNone)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_GRANNY, ID_VIEW_SELECTNONE, OnUpdateCommands)
END_MESSAGE_MAP()

void CFileView::OnLButtonDown(UINT nFlags, CPoint point)
{
	int n = ItemAtPosition(point);
	if (n!=-1)
	{
		FocusItem = n;

		if (nFlags & MK_CONTROL)
		{
			SelectItem(n, !IsSelected(n));
		}
		else
		{
			for (UINT a=0; a<result->m_Count; a++)
				SelectItem(a, (int)a==n, TRUE);

			Invalidate();
			GetParentFrame()->SendMessage(WM_COMMAND, ID_APP_UPDATESELECTION);
		}
	}
	else
	{
		if (GetFocus()!=this)
			SetFocus();
	}
}

void CFileView::OnLButtonUp(UINT nFlags, CPoint point)
{
	int n = ItemAtPosition(point);
	if (n!=-1)
	{
		if (GetFocus()!=this)
			SetFocus();
	}
	else
	{
		if (!(nFlags & MK_CONTROL))
			GetParentFrame()->SendMessage(WM_COMMAND, ID_VIEW_SELECTNONE);
	}
}

void CFileView::OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/)
{
	GetParentFrame()->SendMessage(WM_COMMAND, ID_NAV_STARTNAVIGATION);
}

void CFileView::OnRButtonDown(UINT nFlags, CPoint point)
{
	int n = ItemAtPosition(point);
	if (n!=-1)
	{
		if (!(nFlags & MK_CONTROL))
			if (!IsSelected(n))
			{
				FocusItem = n;

				for (UINT a=0; a<result->m_Count; a++)
					SelectItem(a, (int)a==n, TRUE);

				Invalidate();
			}
			else
				if (FocusItem!=n)
				{
					FocusItem = n;
					Invalidate();
				}
	}
	else
	{
		if (GetFocus()!=this)
			SetFocus();
	}
}

void CFileView::OnRButtonUp(UINT nFlags, CPoint point)
{
	int n = ItemAtPosition(point);
	if (n!=-1)
	{
		if (GetFocus()!=this)
			SetFocus();

		if (IsSelected(n))
		{
			ClientToScreen(&point);
			OnItemContextMenu(n, point);
			return;
		}
	}

	if (!(nFlags & MK_CONTROL))
		GetParentFrame()->SendMessage(WM_COMMAND, ID_VIEW_SELECTNONE);

	ClientToScreen(&point);
	OnContextMenu(point);
}

void CFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!HandleDefaultKeys(nChar, nRepCnt, nFlags))
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CFileView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	SetCursor(theApp.LoadStandardCursor(result ? IDC_ARROW : IDC_WAIT));
	return TRUE;
}

void CFileView::OnToggleGrannyMode()
{
	pViewParameters->GrannyMode = !pViewParameters->GrannyMode;
	OnViewOptionsChanged();
}

void CFileView::OnSelectAll()
{
	if (result)
		for (UINT a=0; a<result->m_Count; a++)
			SelectItem(a, TRUE, a<result->m_Count-1);
}

void CFileView::OnSelectNone()
{
	if (result)
		for (UINT a=0; a<result->m_Count; a++)
			SelectItem(a, FALSE, a<result->m_Count-1);
}

void CFileView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;
	switch (pCmdUI->m_nID)
	{
	case ID_VIEW_GRANNY:
		pCmdUI->SetCheck(m_ViewParameters.GrannyMode);
		b = TRUE;
		break;
	case ID_VIEW_SELECTALL:
	case ID_VIEW_SELECTNONE:
		if (result)
			b = (result->m_Count>0);
		break;
	}

	pCmdUI->Enable(b);
}

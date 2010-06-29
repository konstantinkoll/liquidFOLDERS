
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
	NcDividerLineY = 0;
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
	ASSERT(_ViewID>LFViewAutomatic);

	pViewParameters->Background = theApp.m_Background[_ViewID];

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
		m_DropTarget.Register(this, _result->m_StoreID);

		ActiveContextID = _result->m_ContextView;
		pViewParameters = &theApp.m_Views[ActiveContextID];
		m_ViewParameters.SortBy = pViewParameters->SortBy;

		if (_FocusItem>(int)_result->m_ItemCount-1)
			_FocusItem = (int)_result->m_ItemCount-1;
		FocusItem = _FocusItem;
	}
	else
	{
		m_DropTarget.Revoke();
		m_pDropTarget = NULL;

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

	UINT Mode = ((CMainFrame*)GetParentFrame())->SelectViewMode(m_ViewParameters.Mode);
	if ((Mode==LFViewDetails) || (Mode==LFViewCalendarDay))
	{
		ENSURE(tmpStr.LoadString(ID_VIEW_AUTOSIZECOLUMNS));
		menu->AppendMenu(MF_BYPOSITION | MF_STRING, ID_VIEW_AUTOSIZECOLUMNS, tmpStr);
		menu->AppendMenu(MF_SEPARATOR);
	}

	ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_SORTOPTIONS));
	menu->AppendMenu(MF_STRING, ID_APP_SORTOPTIONS, tmpStr);
	ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_AUTODIRS));
	menu->AppendMenu(MF_STRING, ID_VIEW_AUTODIRS, tmpStr);
	ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_VIEWOPTIONS));
	menu->AppendMenu(MF_STRING, ID_APP_VIEWOPTIONS, tmpStr);

	if (result)
		switch (result->m_Context)
		{
		case LFContextStores:
			menu->AppendMenu(MF_SEPARATOR);
			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_STORE_NEW));
			menu->AppendMenu(MF_STRING, ID_STORE_NEW, tmpStr);
			break;
		case LFContextTrash:
			menu->AppendMenu(MF_SEPARATOR);
			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_TRASH_RESTOREALL));
			menu->AppendMenu(MF_STRING, ID_TRASH_RESTOREALL, tmpStr);
			break;
		case LFContextHousekeeping:
			menu->AppendMenu(MF_SEPARATOR);
			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_UNKNOWN_REGISTER));
			menu->AppendMenu(MF_STRING, ID_UNKNOWN_REGISTER, tmpStr);
			break;
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
	LFItemDescriptor* f = result->m_Items[idx];
	switch (f->Type & LFTypeMask)
	{
	case LFTypeVirtual:
		nID = (f->IconID==IDI_FLD_Back ? IDM_BACK : m_ViewParameters.Mode==LFViewGlobe ? IDM_VIRTUAL_GLOBE : (ActiveContextID==LFContextTrash) ? IDM_VIRTUAL_TRASH : (ActiveContextID==LFContextStoreHome) ? IDM_DOMAIN : IDM_VIRTUAL);
		cmdDefault = ID_ITEMS_OPEN;
		break;
	case LFTypeDrive:
		nID = IDM_DRIVE;
		cmdDefault = ID_STORE_NEWDRIVE;
		break;
	case LFTypeStore:
		nID = IDM_STORE;
		cmdDefault = ID_ITEMS_OPEN;
		break;
	case LFTypeFile:
		nID = (ActiveContextID==LFContextTrash) ? IDM_FILE_TRASH : IDM_FILE;
		cmdDefault = ID_ITEMS_OPEN;
		break;
	}

	if (nID)
	{
		CMenu menu;
		menu.LoadMenu(nID);
		ASSERT_VALID(&menu);

		if ((nID==IDM_FILE) && (ActiveContextID==LFContextHousekeeping))
		{
			CString tmpStr;
			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_UNKNOWN_REGISTER));

			menu.GetSubMenu(0)->AppendMenu(MF_SEPARATOR);
			menu.GetSubMenu(0)->AppendMenu(MF_STRING, ID_UNKNOWN_REGISTER, tmpStr);
		}

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
			GetParentFrame()->SendMessage(WM_COMMAND, ID_ITEMS_OPEN);
			return TRUE;
		}
		break;
	case VK_DELETE:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
		{
			GetParentFrame()->SendMessage(WM_COMMAND, ID_ITEMS_DELETE);
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

void CFileView::SetNcDividerLine(int y)
{
	NcDividerLineY = (y>0) ? y+1 : 0;
}

BEGIN_MESSAGE_MAP(CFileView, CWnd)
	ON_WM_NCPAINT()
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
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->ItemsDropped, OnItemsDropped)
END_MESSAGE_MAP()

void CFileView::OnNcPaint()
{
	if (GetExStyle() & WS_EX_CLIENTEDGE)
	{
		CWindowDC dc(this);

		CRect rect;
		GetWindowRect(rect);

		rect.bottom -= rect.top;
		rect.right -= rect.left;
		rect.left = rect.top = 0;

		COLORREF col = GetSysColor(COLOR_3DFACE+1);
		dc.Draw3dRect(rect, col, col);

		COLORREF back;
		theApp.GetBackgroundColors(pViewParameters->Background, &back);
		rect.DeflateRect(1, 1);
		dc.Draw3dRect(rect, back, back);

		if (NcDividerLineY)
		{
			dc.SetPixel(1, NcDividerLineY, col);
			dc.SetPixel(rect.Width(), NcDividerLineY, col);
		}
	}
}

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
			for (UINT a=0; a<result->m_ItemCount; a++)
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
	GetParentFrame()->SendMessage(WM_COMMAND, ID_ITEMS_OPEN);
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

				for (UINT a=0; a<result->m_ItemCount; a++)
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
			Invalidate();
			GetParentFrame()->SendMessage(WM_COMMAND, ID_APP_UPDATESELECTION);

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
		for (UINT a=0; a<result->m_ItemCount; a++)
			SelectItem(a, TRUE, a<result->m_ItemCount-1);
}

void CFileView::OnSelectNone()
{
	if (result)
		for (UINT a=0; a<result->m_ItemCount; a++)
			SelectItem(a, FALSE, a<result->m_ItemCount-1);
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
			b = (result->m_ItemCount>0);
		break;
	}

	pCmdUI->Enable(b);
}

LRESULT CFileView::OnItemsDropped(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (result)
		if (result->m_Context!=LFContextStores)
			GetParentFrame()->SendMessage(WM_COMMAND, ID_NAV_RELOAD);

	return NULL;
}

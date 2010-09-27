
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
	FocusItem = HoverItem = SelectionAnchor = -1;
	MouseInView = FALSE;
}

CFileView::~CFileView()
{
}

void CFileView::Create(LFSearchResult* _result, UINT _ViewID, int _FocusItem, BOOL _EnableHover, BOOL _EnableShiftSelection)
{
	EnableHover = _EnableHover;
	EnableShiftSelection = _EnableShiftSelection;

	OnUpdateViewOptions(_result->m_ContextView, _ViewID, TRUE);
	OnUpdateSearchResult(_result, _FocusItem);
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

	SetViewOptions(_ViewID, Force);

	m_ViewParameters = *pViewParameters;
	RibbonColor = theApp.m_nAppLook;
	ViewID = _ViewID;
}

void CFileView::OnUpdateSearchResult(LFSearchResult* _result, int _FocusItem)
{
	if (_result)
	{
		m_DropTarget.Register(this, _result->m_StoreID);

		ActiveContextID = _result->m_ContextView;
		pViewParameters = &theApp.m_Views[ActiveContextID];
		m_ViewParameters.SortBy = pViewParameters->SortBy;

		if (_FocusItem>(int)_result->m_ItemCount-1)
			_FocusItem = (int)_result->m_ItemCount-1;
		FocusItem = _FocusItem;
		HideFileExt = theApp.HideFileExt();
	}
	else
	{
		m_DropTarget.Revoke();
		m_pDropTarget = NULL;

		FocusItem = -1;
	}

	SetSearchResult(_result);

	SetCursor(LoadCursor(NULL, _result ? IDC_ARROW : IDC_WAIT));
	result = _result;
}

void CFileView::SelectItem(int /*n*/, BOOL /*select*/, BOOL /*InternalCall*/)
{
}

void CFileView::SetFocusItem(int _FocusItem, BOOL ShiftSelect)
{
	if (ShiftSelect && EnableShiftSelection)
	{
		if (SelectionAnchor==-1)
			SelectionAnchor = FocusItem;

		for (UINT a=0; a<result->m_ItemCount; a++)
			SelectItem(a, (((int)a>=_FocusItem) && ((int)a<=SelectionAnchor)) || (((int)a>=SelectionAnchor) && ((int)a<=_FocusItem)), TRUE);
	}
	else
	{
		SelectionAnchor = -1;

		for (UINT a=0; a<result->m_ItemCount; a++)
			SelectItem(a, (int)a==_FocusItem, TRUE);
	}

	FocusItem = _FocusItem;
	Invalidate();
	GetParentFrame()->SendMessage(WM_COMMAND, ID_APP_UPDATESELECTION);
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

void CFileView::InvalidateItem(int /*n*/)
{
	Invalidate();
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
		ENSURE(tmpStr.LoadString(ID_VIEW_CHOOSEDETAILS));
		menu->AppendMenu(MF_BYPOSITION | MF_STRING, ID_VIEW_CHOOSEDETAILS, tmpStr);
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

int CFileView::GetFontHeight()
{
	LOGFONT lf;
	theApp.m_DefaultFont.GetLogFont(&lf);

	return abs(lf.lfHeight);
}


BEGIN_MESSAGE_MAP(CFileView, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_VIEW_SELECTALL, OnSelectAll)
	ON_COMMAND(ID_VIEW_SELECTNONE, OnSelectNone)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_SELECTALL, ID_VIEW_SELECTNONE, OnUpdateCommands)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->ItemsDropped, OnItemsDropped)
END_MESSAGE_MAP()

void CFileView::OnLButtonDown(UINT nFlags, CPoint point)
{
	int n = ItemAtPosition(point);
	if (n!=-1)
	{
		if (nFlags & MK_CONTROL)
		{
			FocusItem = n;
			SelectItem(n, !IsSelected(n));
		}
		else
		{
			SetFocusItem(n, nFlags & MK_SHIFT);
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
		if (!(nFlags & (MK_SHIFT | MK_CONTROL)))
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
					std::swap(FocusItem, n);
					InvalidateItem(n);
					InvalidateItem(FocusItem);
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

void CFileView::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (EnableHover)
	{
		if (!MouseInView)
		{
			TRACKMOUSEEVENT tme;
			ZeroMemory(&tme, sizeof(tme));
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = GetSafeHwnd();
			TrackMouseEvent(&tme);

			MouseInView = TRUE;
		}

		int idx = ItemAtPosition(point);
		if (idx!=HoverItem)
		{
			std::swap(idx, HoverItem);
			InvalidateItem(idx);
			InvalidateItem(HoverItem);
		}
	}
}

void CFileView::OnMouseLeave()
{
	if (HoverItem!=-1)
	{
		int idx = HoverItem;
		HoverItem = -1;
		InvalidateItem(idx);
	}

	MouseInView = FALSE;
}

void CFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!HandleDefaultKeys(nChar, nRepCnt, nFlags))
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CFileView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	SetCursor(LoadCursor(NULL, result ? IDC_ARROW : IDC_WAIT));
	return TRUE;
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

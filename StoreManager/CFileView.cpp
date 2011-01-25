
// CFileView.cpp: Implementierung der Klasse CFileView
//

#include "stdafx.h"
#include "CFileView.h"
#include "StoreManager.h"


BOOL AttributeSortableInView(UINT Attr, UINT ViewMode)
{
	BOOL b = theApp.m_Attributes[Attr]->Sortable;
	switch (ViewMode)
	{
	case LFViewCalendar:
		b &= (theApp.m_Attributes[Attr]->Type==LFTypeTime);
		break;
	case LFViewGlobe:
		b &= ((Attr==LFAttrLocationIATA) || (theApp.m_Attributes[Attr]->Type==LFTypeGeoCoordinates));
		break;
	}
	return b;
}


// CFileView
//

#define GetItemData(idx)     ((FVItemData*)(m_ItemData+idx*m_DataSize))
#define IsSelected(idx)      GetItemData(idx)->Selected
#define ChangedItem(idx)     { InvalidateItem(idx); GetParent()->SendMessage(WM_UPDATESELECTION); }
#define ChangedItems()       { Invalidate(); GetParent()->SendMessage(WM_UPDATESELECTION); }

CFileView::CFileView(UINT DataSize, BOOL EnableScrolling, BOOL EnableHover, BOOL EnableTooltip, BOOL EnableShiftSelection, BOOL EnableLabelEdit)
	: CWnd()
{
	ASSERT(DataSize>=sizeof(FVItemData));

	p_Result = NULL;
	p_Edit = NULL;
	m_ItemData = NULL;
	m_ItemDataAllocated = 0;
	m_FocusItem = m_HotItem = m_SelectionAnchor = m_EditLabel = m_Context = -1;
	m_Context = LFContextDefault;
	m_HeaderHeight = m_FontHeight[0] = m_FontHeight[1] = m_HScrollMax = m_VScrollMax = m_HScrollPos = m_VScrollPos = 0;
	m_DataSize = DataSize;
	m_Hover = m_ShowFocusRect = FALSE;
	hThemeList = NULL;

	m_EnableScrolling = EnableScrolling;
	m_EnableHover = EnableHover;
	m_EnableTooltip = EnableTooltip;
	m_EnableShiftSelection = EnableShiftSelection;
	m_EnableLabelEdit = EnableLabelEdit;
}

CFileView::~CFileView()
{
	DestroyEdit();

	if (m_ItemData)
		free(m_ItemData);
}

BOOL CFileView::Create(CWnd* pParentWnd, UINT nID, LFSearchResult* Result, INT FocusItem, UINT nClassStyle)
{
	CString className = AfxRegisterWndClass(nClassStyle, LoadCursor(NULL, IDC_ARROW));

	DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	if (m_EnableScrolling)
		dwStyle |= WS_HSCROLL | WS_VSCROLL;

	CRect rect;
	rect.SetRectEmpty();
	if (!CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID))
		return FALSE;

	UpdateViewOptions(Result ? Result->m_Context : LFContextDefault, TRUE);
	UpdateSearchResult(Result, FocusItem);
	return TRUE;
}

BOOL CFileView::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (p_Edit)
			switch (pMsg->wParam)
			{
			case VK_EXECUTE:
			case VK_RETURN:
				DestroyEdit(TRUE);
			case VK_ESCAPE:
				DestroyEdit();
				return TRUE;
			}
		break;
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		if (p_Edit)
			return TRUE;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
		m_TooltipCtrl.Deactivate();
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CFileView::UpdateViewOptions(INT Context, BOOL Force)
{
	DestroyEdit();

	if (Context>=0)
		m_Context = Context;
	p_ViewParameters = &theApp.m_Views[m_Context];

	m_EditLabel = -1;
	SetViewOptions(Force);

	BOOL Arrange = (m_ViewParameters.Mode!=p_ViewParameters->Mode);
	m_ViewParameters = *p_ViewParameters;

	if (Arrange)
	{
		AdjustLayout();
	}
	else
	{
		Invalidate();
	}
}

void CFileView::UpdateSearchResult(LFSearchResult* Result, INT FocusItem)
{
	DestroyEdit();

	void* Victim = m_ItemData;
	UINT VictimAllocated = m_ItemDataAllocated;

	if (Result)
	{
		size_t sz = Result->m_ItemCount*m_DataSize;
		m_ItemData = (BYTE*)malloc(sz);
		m_ItemDataAllocated = Result->m_ItemCount;
		ZeroMemory(m_ItemData, sz);

		if ((VictimAllocated) && (Result->m_Context!=LFContextClipboard))
			for (UINT a=0; a<min(VictimAllocated, Result->m_ItemCount); a++)
			{
				FVItemData* d = GetItemData(a);

				BYTE* v = (BYTE*)Victim;
				v += ((BYTE*)d)-((BYTE*)m_ItemData);

				d->Selected = ((FVItemData*)v)->Selected;
			}

		m_DropTarget.Register(this, Result->m_StoreID);

		m_Context = Result->m_Context;
		p_ViewParameters = &theApp.m_Views[m_Context];
		m_ViewParameters.SortBy = p_ViewParameters->SortBy;

		m_FocusItem = min(FocusItem, (INT)Result->m_ItemCount-1);
		m_HotItem = -1;
		m_HideFileExt = theApp.HideFileExt();
	}
	else
	{
		m_ItemData = NULL;
		m_ItemDataAllocated = 0;

		m_DropTarget.Revoke();
		m_pDropTarget = NULL;

		m_FocusItem = m_HotItem = -1;
		m_HScrollPos = m_VScrollPos = 0;

		m_ShowFocusRect = FALSE;
	}

	m_EditLabel = -1;
	SetSearchResult(Result);

	p_Result = Result;
	free(Victim);

	if (p_Result)
	{
		BOOL NeedNewFocusItem = !GetItemData(m_FocusItem)->Valid;

		for (UINT a=0; a<p_Result->m_ItemCount; a++)
		{
			FVItemData* d = GetItemData(a);
			d->Selected &= d->Valid;

			if (NeedNewFocusItem && d->Valid)
			{
				m_FocusItem = a;
				NeedNewFocusItem = FALSE;
			}
		}

		AdjustLayout();
		EnsureVisible(m_FocusItem);
	}
	else
	{
		Invalidate();
	}

	SetCursor(theApp.LoadStandardCursor(Result ? IDC_ARROW : IDC_WAIT));
}

void CFileView::SetViewOptions(BOOL /*Force*/)
{
}

void CFileView::SetSearchResult(LFSearchResult* /*Result*/)
{
}

void CFileView::AdjustLayout()
{
	AdjustScrollbars();
	Invalidate();
}

INT CFileView::GetFocusItem()
{
	if (p_Result)
	{
		FVItemData* d = GetItemData(m_FocusItem);
		return (d->Valid) ? m_FocusItem : -1;
	}

	return -1;
}

INT CFileView::GetSelectedItem()
{
	if (p_Result)
	{
		FVItemData* d = GetItemData(m_FocusItem);
		return (d->Selected && d->Valid) ? m_FocusItem : -1;
	}

	return -1;
}

INT CFileView::GetNextSelectedItem(INT idx)
{
	if (p_Result)
	{
		ASSERT(idx>=-1);

		while (++idx<(INT)p_Result->m_ItemCount)
		{
			FVItemData* d = GetItemData(idx);
			if (d->Selected && d->Valid)
				return idx;
		}
	}

	return -1;
}

void CFileView::SelectItem(INT idx, BOOL Select, BOOL InternalCall)
{
	if (p_Result)
	{
		ASSERT(idx<(INT)p_Result->m_ItemCount);

		FVItemData* d = GetItemData(idx);
		if (d->Valid)
		{
			d->Selected = Select;
			if (!InternalCall)
				ChangedItem(idx);
		}
	}
}

void CFileView::EnsureVisible(INT idx)
{
	if (!m_EnableScrolling)
		return;

	CRect rect;
	GetClientRect(&rect);

	RECT rectItem = GetItemRect(idx);

	SCROLLINFO si;
	INT nInc;

	// Vertikal
	nInc = 0;
	if (rectItem.bottom>rect.Height())
		nInc = rectItem.bottom-rect.Height();
	if (rectItem.top<nInc+(INT)m_HeaderHeight)
		nInc = rectItem.top-(INT)m_HeaderHeight;

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindowEx(0, -nInc, NULL, NULL, NULL, NULL, SW_INVALIDATE);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);
	}

	// Horizontal
	if (m_ViewParameters.Mode!=LFViewDetails)
	{
		nInc = 0;
		if (rectItem.right>rect.Width())
			nInc = rectItem.right-rect.Width();
		if (rectItem.left<nInc)
			nInc = rectItem.left;

		nInc = max(-m_HScrollPos, min(nInc, m_HScrollMax-m_HScrollPos));
		if (nInc)
		{
			m_HScrollPos += nInc;
			ScrollWindowEx(-nInc, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE);
	
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS;
			si.nPos = m_HScrollPos;
			SetScrollInfo(SB_HORZ, &si);
		}
	}
}

void CFileView::SetFocusItem(INT FocusItem, BOOL ShiftSelect)
{
	if (ShiftSelect && m_EnableShiftSelection)
	{
		if (m_SelectionAnchor==-1)
			m_SelectionAnchor = m_FocusItem;

		for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
			SelectItem(a, ((a>=FocusItem) && (a<=m_SelectionAnchor)) || ((a>=m_SelectionAnchor) && (a<=FocusItem)), TRUE);
	}
	else
	{
		m_SelectionAnchor = -1;

		for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
			SelectItem(a, a==FocusItem, TRUE);
	}

	m_FocusItem = FocusItem;
	m_EditLabel = -1;

	ChangedItems();
	EnsureVisible(m_FocusItem);
}

RECT CFileView::GetItemRect(INT idx)
{
	RECT rect = { 0, 0, 0, 0 };

	if (p_Result)
		if ((idx>=0) && (idx<(INT)p_Result->m_ItemCount))
		{
			rect = GetItemData(idx)->Rect;
			OffsetRect(&rect, -m_HScrollPos, -m_VScrollPos+m_HeaderHeight);
		}

	return rect;
}

RECT CFileView::GetLabelRect(INT idx)
{
	return GetItemRect(idx);
}

INT CFileView::ItemAtPosition(CPoint point)
{
	if (p_Result)
	{
		point.Offset(m_HScrollPos, m_VScrollPos-m_HeaderHeight);

		for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
			if (PtInRect(&GetItemData(a)->Rect, point))
				return a;
	}

	return -1;
}

void CFileView::InvalidateItem(INT idx)
{
	if (p_Result)
		if ((idx>=0) && (idx<(INT)p_Result->m_ItemCount))
		{
			RECT rect = GetItemRect(idx);
			InvalidateRect(&rect);
		}
}

CMenu* CFileView::GetBackgroundContextMenu()
{
	CMenu* pMenu = new CMenu();

	switch (m_Context)
	{
	case LFContextStores:
		pMenu->LoadMenu(IDM_STORES);
		break;
	case LFContextStoreHome:
		pMenu->LoadMenu(IDM_HOME);
		break;
	case LFContextHousekeeping:
		pMenu->LoadMenu(IDM_HOUSEKEEPING);
		break;
	case LFContextTrash:
		pMenu->LoadMenu(IDM_TRASH);
		break;
	default:
		pMenu->CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)CreateMenu(), _T("POPUP"));
	}

	return pMenu;
}

CMenu* CFileView::GetItemContextMenu(INT idx)
{
	CMenu* pMenu = new CMenu();

	LFItemDescriptor* item = p_Result->m_Items[idx];
	switch (item->Type & LFTypeMask)
	{
	case LFTypeDrive:
		pMenu->LoadMenu(IDM_DRIVE);
		break;
	case LFTypeStore:
		pMenu->LoadMenu(IDM_STORE);
		pMenu->GetSubMenu(0)->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);
		break;
	case LFTypeVirtual:
		if ((item->FirstAggregate==-1) || (item->LastAggregate==-1))
			break;
	case LFTypeFile:
		pMenu->LoadMenu((m_Context==LFContextTrash) ? IDM_FILE_TRASH : IDM_FILE);
		break;
	}

	if (!IsMenu(*pMenu))
	{
		pMenu->CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)CreateMenu(), _T("POPUP"));
	}

	CMenu* pPopup = pMenu->GetSubMenu(0);
	ASSERT_VALID(pPopup);

	if ((item->Type & LFTypeMask)!=LFTypeDrive)
	{
		CString tmpStr;

		if (((item->Type & LFTypeMask)==LFTypeFile) || (((item->Type & LFTypeMask)==LFTypeVirtual) && (item->FirstAggregate!=-1) && (item->LastAggregate!=-1)))
			if (m_Context==LFContextClipboard)
			{
				ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_REMOVE));
				pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_FILE_REMOVE, tmpStr);
					pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);
			}
			else
				if (m_Context!=LFContextTrash)
				{
					ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_REMEMBER));
					pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_FILE_REMEMBER, tmpStr);
					pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);
				}

		if ((item->Type & LFTypeMask)==LFTypeFile)
		{
			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPENWITH));
			pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_FILE_OPENWITH, tmpStr);
		}

		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPEN));
		pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_ITEM_OPEN, tmpStr);
	}

	pPopup->SetDefaultItem(0, TRUE);
	return pMenu;
}

void CFileView::EditLabel(INT idx)
{
	m_EditLabel = -1;

	if ((m_EnableLabelEdit) && (p_Result))
	{
		LFItemDescriptor* item = p_Result->m_Items[idx];
		if (((item->Type & LFTypeMask)==LFTypeStore) || ((item->Type & LFTypeMask)==LFTypeFile))
		{
			m_EditLabel = idx;
			InvalidateItem(idx);
			EnsureVisible(idx);

			CRect rect(GetLabelRect(idx));
			if (rect.Height()>m_FontHeight[0]+4)
			{
				rect.top += (rect.Height()-m_FontHeight[0]-4)/2;
				rect.bottom = rect.top+m_FontHeight[0]+4;
			}

			p_Edit = new CEdit();
			p_Edit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, this, 2);
			p_Edit->SetWindowText(item->CoreAttributes.FileName);
			p_Edit->SetSel(0, (INT)wcslen(item->CoreAttributes.FileName));
			p_Edit->SetFont(&theApp.m_DefaultFont);
			p_Edit->SetFocus();
		}
	}
}

BOOL CFileView::IsEditing()
{
	return (p_Edit!=NULL);
}

void CFileView::DrawItemBackground(CDC& dc, LPRECT rectItem, INT idx, BOOL Themed)
{
	BOOL Hot = (m_HotItem==idx);
	BOOL Selected = IsSelected(idx);

	if (hThemeList)
	{
		dc.SetTextColor((p_Result->m_Items[idx]->CoreAttributes.Flags & LFFlagMissing) ? 0x0000FF : 0x000000);

		if (Hot | Selected)
		{
			const INT StateIDs[4] = { LISS_NORMAL, LISS_HOT, GetFocus()!=this ? LISS_SELECTEDNOTFOCUS : LISS_SELECTED, LISS_HOTSELECTED };
			UINT State = 0;
			if (Hot)
				State |= 1;
			if (Selected)
				State |= 2;
				theApp.zDrawThemeBackground(hThemeList, dc, LVP_LISTITEM, StateIDs[State], rectItem, rectItem);
		}

		if ((GetFocus()==this) && (m_FocusItem==idx))
			switch (theApp.OSVersion)
			{
			case OS_Seven:
				if (!Selected)
				{
					CRect rect(rectItem);
					rect.bottom--;
					rect.right--;

					Graphics g(dc);
					g.SetSmoothingMode(SmoothingModeAntiAlias);

					GraphicsPath path;
					CreateRoundRectangle(rect, 2, path);

					Pen pen(Color(0xFF, 0x7D, 0xA2, 0xCE));
					g.DrawPath(&pen, &path);
				}
				break;
			case OS_Vista:
				if (m_ShowFocusRect)
				{
					CRect rect(rectItem);
					rect.DeflateRect(1, 1);

					dc.SetBkColor(0xFFFFFF);
					dc.DrawFocusRect(rect);
				}
				break;
			}
	}
	else
	{
		if (Selected)
		{
			dc.FillSolidRect(rectItem, GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHT : COLOR_3DFACE));
			dc.SetTextColor(GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
			dc.SetBkColor(0x000000);
		}
		else
		{
			dc.SetTextColor(Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
			dc.SetBkColor(0xFFFFFF);
		}

		if ((idx==m_FocusItem) && (GetFocus()==this))
			dc.DrawFocusRect(rectItem);

		if ((!Selected) && (p_Result->m_Items[idx]->CoreAttributes.Flags & LFFlagMissing))
			dc.SetTextColor(0x0000FF);
	}
}

void CFileView::DrawCategory(CDC& dc, LPRECT rectCategory, ItemCategory* ic, BOOL Themed)
{
	CRect rect(rectCategory);
	rect.DeflateRect(0, CategoryPadding);
	rect.left += CategoryPadding;

	CFont* pOldFont = dc.SelectObject(&theApp.m_LargeFont);
	dc.SetTextColor(Themed ? 0x993300 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(ic->Caption, -1, rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

	CRect rectLine(rect);
	dc.DrawText(ic->Caption, -1, rectLine, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT);
	rectLine.right += 2*CategoryPadding;

	if (rectLine.right<=rect.right)
	{
		CPen pen(PS_SOLID, 1, 0xE2E2E2);

		CPen* pOldPen = dc.SelectObject(&pen);
		dc.MoveTo(rectLine.right, rect.top+(m_FontHeight[1]+1)/2);
		dc.LineTo(rect.right, rect.top+(m_FontHeight[1]+1)/2);
		dc.SelectObject(pOldPen);
	}

	if (Themed)
		dc.SetTextColor(((dc.GetTextColor()>>1) & 0x7F7F7F) | 0x808080);

	if (ic->Hint[0]!=L'\0')
	{
		rect.top += m_FontHeight[1];
		dc.SelectObject(&theApp.m_DefaultFont);
		dc.DrawText(ic->Hint, -1, rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
	}

	dc.SelectObject(pOldFont);
}

void CFileView::PrepareFormatData(INT idx)
{
	if ((p_Result->m_Items[idx]->Type & LFTypeMask)!=LFTypeFile)
		return;

	theApp.PrepareFormatData(p_Result->m_Items[idx]->CoreAttributes.FileFormat);
}

void CFileView::ResetScrollbars()
{
	if (m_EnableScrolling)
	{
		ScrollWindowEx(0, m_VScrollPos, NULL, NULL, NULL, NULL, SW_INVALIDATE);
		ScrollWindowEx(m_HScrollPos, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);
		m_VScrollPos = m_HScrollPos = 0;
		SetScrollPos(SB_VERT, m_VScrollPos, TRUE);
		SetScrollPos(SB_HORZ, m_HScrollPos, TRUE);
	}
}

void CFileView::AdjustScrollbars()
{
	if (!m_EnableScrolling)
		return;

	CRect rect;
	GetWindowRect(&rect);

	BOOL HScroll = FALSE;
	if (m_ScrollWidth>rect.Width())
	{
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
		HScroll = TRUE;
	}
	if (m_ScrollHeight>rect.Height()-(INT)m_HeaderHeight)
		rect.right -= GetSystemMetrics(SM_CXVSCROLL);
	if ((m_ScrollWidth>rect.Width()) && (!HScroll))
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);

	INT oldVScrollPos = m_VScrollPos;
	m_VScrollMax = max(0, m_ScrollHeight-rect.Height()+(INT)m_HeaderHeight);
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height()-m_HeaderHeight;
	si.nMin = 0;
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	INT oldHScrollPos = m_HScrollPos;
	m_HScrollMax = max(0, m_ScrollWidth-rect.Width());
	m_HScrollPos = min(m_HScrollPos, m_HScrollMax);

	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Width();
	si.nMin = 0;
	si.nMax = m_ScrollWidth-1;
	si.nPos = m_HScrollPos;
	SetScrollInfo(SB_HORZ, &si);

	if ((oldVScrollPos!=m_VScrollPos) || (oldHScrollPos!=m_HScrollPos))
		Invalidate();
}

CString CFileView::GetLabel(LFItemDescriptor* i)
{
	CString label = i->CoreAttributes.FileName;
	if ((i->Type & LFTypeMask)==LFTypeFile)
		if ((!m_HideFileExt) && (i->CoreAttributes.FileFormat[0]!='\0'))
		{
			label += _T(".");
			label += i->CoreAttributes.FileFormat;
		}

	return label;
}

void CFileView::AppendAttribute(LFItemDescriptor* i, UINT attr, CString& str)
{
	wchar_t tmpStr[256];
	LFAttributeToString(i, attr, tmpStr, 256);

	if (tmpStr[0]!=L'\0')
	{
		if (!str.IsEmpty())
			str += _T("\n");
		if ((attr!=LFAttrComment) && (attr!=LFAttrDescription))
		{
			str += theApp.m_Attributes[attr]->Name;
			str += _T(": ");
		}

		str += tmpStr;
	}
}

CString CFileView::GetHint(LFItemDescriptor* i)
{
	CString hint;

	switch (i->Type & LFTypeMask)
	{
	case LFTypeDrive:
		AppendAttribute(i, LFAttrDescription, hint);
		break;
	case LFTypeStore:
		AppendAttribute(i, LFAttrComment, hint);
		AppendAttribute(i, LFAttrDescription, hint);
		AppendAttribute(i, LFAttrCreationTime, hint);
		AppendAttribute(i, LFAttrFileTime, hint);
		break;
	case LFTypeVirtual:
		AppendAttribute(i, LFAttrComment, hint);
		AppendAttribute(i, LFAttrDescription, hint);
		if (i->CoreAttributes.FileSize>0)
			AppendAttribute(i, LFAttrFileSize, hint);
		break;
	case LFTypeFile:
		AppendAttribute(i, LFAttrComment, hint);
		AppendAttribute(i, LFAttrArtist, hint);
		AppendAttribute(i, LFAttrTitle, hint);
		AppendAttribute(i, LFAttrRecordingTime, hint);
		AppendAttribute(i, LFAttrDuration, hint);
		AppendAttribute(i, LFAttrTags, hint);
		AppendAttribute(i, LFAttrPages, hint);
		AppendAttribute(i, LFAttrWidth, hint);
		AppendAttribute(i, LFAttrHeight, hint);
		AppendAttribute(i, LFAttrRecordingEquipment, hint);
		AppendAttribute(i, LFAttrBitrate, hint);
		AppendAttribute(i, LFAttrCreationTime, hint);
		AppendAttribute(i, LFAttrFileTime, hint);
		AppendAttribute(i, LFAttrFileSize, hint);
		break;
	}

	return hint;
}

void CFileView::DestroyEdit(BOOL Accept)
{
	if (p_Edit)
	{
		CEdit* victim = p_Edit;
		p_Edit = NULL;

		CString Name;
		victim->GetWindowText(Name);
		victim->DestroyWindow();
		delete victim;

		if ((Accept) && (!Name.IsEmpty()) && (m_EditLabel!=-1))
			GetParent()->SendMessage(WM_RENAMEITEM, (WPARAM)m_EditLabel, (LPARAM)Name.GetBuffer());
	}

	m_EditLabel = -1;
}


BEGIN_MESSAGE_MAP(CFileView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SETCURSOR()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE_VOID(WM_SELECTALL, OnSelectAll)
	ON_MESSAGE_VOID(WM_SELECTNONE, OnSelectNone)
	ON_COMMAND(IDM_SELECTALL, OnSelectAll)
	ON_COMMAND(IDM_SELECTNONE, OnSelectNone)
	ON_COMMAND(IDM_SELECTINVERT, OnSelectInvert)
	ON_UPDATE_COMMAND_UI(ID_APP_NEWFILEDROP, OnUpdateCommands)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_SELECTALL, IDM_SELECTINVERT, OnUpdateCommands)
	ON_EN_KILLFOCUS(2, OnDestroyEdit)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->ItemsDropped, OnItemsDropped)
END_MESSAGE_MAP()

INT CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_TooltipCtrl.Create(this);

	if (theApp.m_ThemeLibLoaded)
		if (theApp.OSVersion>=OS_Vista)
		{
			theApp.zSetWindowTheme(GetSafeHwnd(), L"explorer", NULL);
			hThemeList = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
		}

	if (m_EnableScrolling)
		ResetScrollbars();

	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&theApp.m_DefaultFont);
	m_FontHeight[0] = m_RowHeight = dc->GetTextExtent(_T("Wy"), 2).cy;
	dc->SelectObject(&theApp.m_LargeFont);
	m_FontHeight[1] = dc->GetTextExtent(_T("Wy"), 2).cy;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	return 0;
}

void CFileView::OnDestroy()
{
	if (hThemeList)
		theApp.zCloseThemeData(hThemeList);

	CWnd::OnDestroy();
}

LRESULT CFileView::OnThemeChanged()
{
	if (theApp.m_ThemeLibLoaded)
	{
		if (hThemeList)
			theApp.zCloseThemeData(hThemeList);

		if (theApp.OSVersion>=OS_Vista)
			hThemeList = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	return TRUE;
}

void CFileView::OnSysColorChange()
{
	Invalidate();
}

BOOL CFileView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CFileView::OnSize(UINT nType, INT cx, INT cy)
{
	SetRedraw(FALSE);
	CWnd::OnSize(nType, cx, cy);
	SetRedraw(TRUE);

	AdjustLayout();
}

void CFileView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rect;
	GetClientRect(&rect);

	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_VScrollPos;
		break;
	case SB_BOTTOM:
		nInc = m_VScrollMax-m_VScrollPos;
		break;
	case SB_LINEUP:
		nInc = -m_RowHeight;
		break;
	case SB_LINEDOWN:
		nInc = m_RowHeight;
		break;
	case SB_PAGEUP:
		nInc = min(-1, -(rect.Height()-(INT)m_HeaderHeight));
		break;
	case SB_PAGEDOWN:
		nInc = max(1, rect.Height()-(INT)m_HeaderHeight);
		break;
	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_VERT, &si);

		nInc = si.nTrackPos-m_VScrollPos;
		break;
	}

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindowEx(0, -nInc, NULL, NULL, NULL, NULL, SW_INVALIDATE);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CFileView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_HScrollPos;
		break;
	case SB_BOTTOM:
		nInc = m_HScrollMax-m_HScrollPos;
		break;
	case SB_PAGEUP:
	case SB_LINEUP:
		nInc = -64;
		break;
	case SB_PAGEDOWN:
	case SB_LINEDOWN:
		nInc = 64;
		break;
	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_HORZ, &si);

		nInc = si.nTrackPos-m_HScrollPos;
		break;
	}

	nInc = max(-m_HScrollPos, min(nInc, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_HScrollPos += nInc;
		ScrollWindowEx(-nInc, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_HScrollPos;
		SetScrollInfo(SB_HORZ, &si);
	}

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFileView::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (m_EnableHover || m_EnableTooltip)
	{
		INT Item = ItemAtPosition(point);

		if (!m_Hover)
		{
			m_Hover = TRUE;

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE | TME_HOVER;
			tme.dwHoverTime = LFHOVERTIME;
			tme.hwndTrack = m_hWnd;
			TrackMouseEvent(&tme);
		}
		else
			if ((m_TooltipCtrl.IsWindowVisible()) && (Item!=m_HotItem))
				m_TooltipCtrl.Deactivate();

		if (m_HotItem!=Item)
		{
			if (m_EnableHover)
				InvalidateItem(m_HotItem);
			m_HotItem = Item;
			if (m_EnableHover)
				InvalidateItem(m_HotItem);
		}
	}
}

void CFileView::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	InvalidateItem(m_HotItem);

	m_Hover = FALSE;
	m_HotItem = -1;
}

void CFileView::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if ((m_HotItem!=-1) && (!IsEditing()))
			if (m_HotItem==m_EditLabel)
			{
				m_TooltipCtrl.Deactivate();
				EditLabel(m_EditLabel);
			}
			else
				if (!m_TooltipCtrl.IsWindowVisible() && m_EnableTooltip)
				{
					PrepareFormatData(m_HotItem);

					LFItemDescriptor* i = p_Result->m_Items[m_HotItem];
					INT SysIconIndex = (((i->Type & LFTypeMask)==LFTypeFile) && (i->CoreAttributes.FileFormat[0]!='\0')) ? theApp.m_FileFormats[i->CoreAttributes.FileFormat].SysIconIndex : -1;

					HICON hIcon = (SysIconIndex>=0) ? theApp.m_SystemImageListExtraLarge.ExtractIcon(SysIconIndex) : theApp.m_CoreImageListExtraLarge.ExtractIcon(i->IconID-1);

					INT cx = 48;
					INT cy = 48;
					ImageList_GetIconSize(theApp.m_SystemImageListExtraLarge, &cx, &cy);
					CSize sz(cx, cy);

					ClientToScreen(&point);
					m_TooltipCtrl.Track(point, hIcon, sz, GetLabel(i), GetHint(i));
				}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = LFHOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

BOOL CFileView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(&rect);
	if (!rect.PtInRect(pt))
		return FALSE;

	INT nInc = max(-m_VScrollPos, min(-zDelta*(INT)m_RowHeight/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindowEx(0, -nInc, NULL, NULL, NULL, NULL, SW_INVALIDATE);
		SetScrollPos(SB_VERT, m_VScrollPos, TRUE);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}

void CFileView::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(&rect);
	if (!rect.PtInRect(pt))
		return;

	INT nInc = max(-m_HScrollPos, min(zDelta*64/WHEEL_DELTA, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_HScrollPos += nInc;
		ScrollWindowEx(-nInc, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE);
		SetScrollPos(SB_HORZ, m_HScrollPos, TRUE);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}
}

void CFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((!m_ShowFocusRect) && (theApp.OSVersion==OS_Vista))
	{
		m_ShowFocusRect = TRUE;
		InvalidateItem(m_FocusItem);
	}

	switch(nChar)
	{
	case 'A':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnSelectAll();
		break;
	case 'N':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnSelectNone();
		break;
	case 'I':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnSelectInvert();
		break;
	case VK_SPACE:
		if (m_FocusItem!=-1)
			SelectItem(m_FocusItem, (GetKeyState(VK_CONTROL)>=0) ? TRUE : !GetItemData(m_FocusItem)->Selected);
		break;
	case VK_F2:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			EditLabel(m_FocusItem);
		break;
	case VK_F5:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_RELOAD);
		break;
	case VK_BACK:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, ID_NAV_BACK);
		break;
	case VK_EXECUTE:
	case VK_RETURN:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, IDM_ITEM_OPEN);
		break;
	case VK_DELETE:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, IDM_FILE_DELETE);
		break;
	default:
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CFileView::OnLButtonDown(UINT nFlags, CPoint point)
{
	DestroyEdit();

	INT idx = ItemAtPosition(point);
	if (idx!=-1)
	{
		if (nFlags & MK_CONTROL)
		{
			InvalidateItem(m_FocusItem);
			m_FocusItem = idx;
			SelectItem(idx, !IsSelected(idx));
		}
		else
			if ((m_FocusItem==idx) && (IsSelected(idx)))
			{
				m_EditLabel = idx;
			}
			else
			{
				SetFocusItem(idx, nFlags & MK_SHIFT);
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
	INT idx = ItemAtPosition(point);
	if (idx!=-1)
	{
		if (GetFocus()!=this)
			SetFocus();
	}
	else
	{
		if (!(nFlags & MK_CONTROL))
			OnSelectNone();
	}
}

void CFileView::OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/)
{
	DestroyEdit();

	GetOwner()->SendMessage(WM_COMMAND, IDM_ITEM_OPEN);
}

void CFileView::OnRButtonDown(UINT nFlags, CPoint point)
{
	INT idx = ItemAtPosition(point);
	if (idx!=-1)
	{
		if (!(nFlags & (MK_SHIFT | MK_CONTROL)))
			if (!IsSelected(idx))
			{
				m_FocusItem = idx;

				for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
					SelectItem(a, a==idx, TRUE);

				ChangedItems();
			}
			else
				if (m_FocusItem!=idx)
				{
					InvalidateItem(m_FocusItem);
					m_FocusItem = idx;
					InvalidateItem(m_FocusItem);
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
	INT idx = ItemAtPosition(point);
	if (idx!=-1)
	{
		if (GetFocus()!=this)
			SetFocus();

		if (!GetItemData(idx)->Selected)
		{
			m_FocusItem = idx;

			for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
				SelectItem(a, a==idx, TRUE);

			ChangedItems();
		}
	}
	else
	{
		if (!(nFlags & MK_CONTROL))
			OnSelectNone();
	}

	CWnd::OnRButtonUp(nFlags, point);
}

void CFileView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CFileView::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}

BOOL CFileView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	SetCursor(theApp.LoadStandardCursor(p_Result ? IDC_ARROW : IDC_WAIT));
	return TRUE;
}

void CFileView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	INT idx = -1;
	if ((point.x==-1) && (point.y==-1))
	{
		idx = GetSelectedItem();

		if (idx!=-1)
		{
			FVItemData* d = GetItemData(idx);
			point.x = d->Rect.left-m_HScrollPos;
			point.y = d->Rect.top-m_VScrollPos+m_HeaderHeight;
			ClientToScreen(&point);
		}
	}
	else
	{
		CPoint ptClient(point);
		ScreenToClient(&ptClient);

		idx = ItemAtPosition(ptClient);
	}

	if (idx==-1)
	{
		GetParent()->SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(point.x, point.y));
	}
	else
	{
		CMenu* pMenu = GetItemContextMenu(idx);
		if (pMenu)
		{
			CMenu* pPopup = pMenu->GetSubMenu(0);
			ASSERT_VALID(pPopup);

			HBITMAP bmp = theApp.SetContextMenuIcon(pPopup, IDM_DRIVE_CREATENEWSTORE, IDI_STORE_Bag);

			pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);
			delete pMenu;

			DeleteObject(bmp);
		}
	}
}

void CFileView::OnSelectAll()
{
	if (p_Result)
	{
		for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
			SelectItem(a, TRUE, TRUE);

		ChangedItems();
		RedrawWindow();
	}
}

void CFileView::OnSelectNone()
{
	if (p_Result)
	{
		for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
			SelectItem(a, FALSE, TRUE);

		ChangedItems();
		RedrawWindow();
	}
}

void CFileView::OnSelectInvert()
{
	if (p_Result)
	{
		for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
			SelectItem(a, !IsSelected(a), TRUE);

		ChangedItems();
		RedrawWindow();
	}
}

void CFileView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;

	switch (pCmdUI->m_nID)
	{
	case ID_APP_NEWFILEDROP:
		b = (m_Context<=LFContextStoreHome) && (_waccess(theApp.m_Path+_T("FileDrop.exe"), 0)==0);
		break;
	case IDM_SELECTALL:
	case IDM_SELECTNONE:
	case IDM_SELECTINVERT:
		b = p_Result ? p_Result->m_ItemCount : FALSE;
		break;
	}

	pCmdUI->Enable(b);
}

void CFileView::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}

LRESULT CFileView::OnItemsDropped(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (p_Result)
		if (p_Result->m_Context!=LFContextStores)
			GetOwner()->SendMessage(WM_COMMAND, ID_NAV_RELOAD);

	return NULL;
}

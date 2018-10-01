
// CAbstractFileView.cpp: Implementierung der Klasse CAbstractFileView
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CAbstractFileView
//

CAbstractFileView::CAbstractFileView(UINT Flags, SIZE_T DataSize, const CSize& szItemInflate)
	: CFrontstageItemView(Flags, DataSize, szItemInflate)
{
	ASSERT(Flags & FRONTSTAGE_ENABLEFOCUSITEM);

	p_CookedFiles = NULL;

	m_FlagsMask = Flags;
	m_pWndEdit = NULL;
	m_TypingBuffer[0] = L'\0';
	m_TypingTicks = 0;

	// Item cateogries
	for (UINT a=0; a<LFItemCategoryCount; a++)
		AddItemCategory(LFGetApp()->m_ItemCategories[a].Caption, LFGetApp()->m_ItemCategories[a].Hint);
}

BOOL CAbstractFileView::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (m_pWndEdit)
			switch (pMsg->wParam)
			{
			case VK_EXECUTE:
			case VK_RETURN:
				DestroyEdit(TRUE);
				return TRUE;

			case VK_ESCAPE:
				DestroyEdit(FALSE);
				return TRUE;
			}

		break;

	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		if (m_pWndEdit)
			return TRUE;

		break;
	}

	return CFrontstageItemView::PreTranslateMessage(pMsg);
}

void CAbstractFileView::UpdateSearchResult(LFSearchResult* pCookedFiles)
{
	SetSearchResult(pCookedFiles);
	ValidateAllItems();

	FinishUpdate();
}

void CAbstractFileView::FinishUpdate(BOOL InternalCall)
{
	if (p_CookedFiles)
	{
		BOOL NeedsNewFocusItem = (m_FocusItem>=0) ? !GetItemData(m_FocusItem)->Valid : FALSE;

		for (INT Index=0; Index<m_ItemCount; Index++)
			if (GetItemData(Index)->Valid)
			{
				m_Nothing = FALSE;

				if (NeedsNewFocusItem)
				{
					m_FocusItem = Index;
					NeedsNewFocusItem = FALSE;
				}
			}
			else
			{
				SelectItem(Index, FALSE);
				ItemSelectionChanged(Index);
			}

		if (IsSelectionEnabled())
		{
			m_FocusItemSelected = (m_FocusItem>=0) && IsItemSelected(m_FocusItem);
		}
		else
		{
			if (m_FocusItem>=0)
				SelectItem(m_FocusItem, m_FocusItemSelected);
		}

		AdjustLayout();

		if (!InternalCall)
			EnsureVisible(m_FocusItem);
	}
	else
	{
		Invalidate();
	}

	SetCursor(LFGetApp()->LoadStandardCursor(p_CookedFiles ? IDC_ARROW : IDC_WAIT));
}

void CAbstractFileView::SetSearchResult(LFSearchResult* pCookedFiles)
{
	DestroyEdit();
	HideTooltip();
	ResetDragLocation();

	if (pCookedFiles)
	{
		SetItemCount(pCookedFiles->m_ItemCount, TRUE);

		// Enable shift selection and label edit in certain contexts
		m_Flags &= ~(FRONTSTAGE_ENABLESELECTION | FRONTSTAGE_ENABLESHIFTSELECTION | FRONTSTAGE_ENABLELABELEDIT);

		if (pCookedFiles->m_Context!=LFContextStores)
			m_Flags |= (FRONTSTAGE_ENABLESELECTION | FRONTSTAGE_ENABLESHIFTSELECTION);

		if ((pCookedFiles->m_Context!=LFContextArchive) && (pCookedFiles->m_Context!=LFContextTrash))
			m_Flags |= FRONTSTAGE_ENABLELABELEDIT;

		// Disable flags that weren't initially allowed
		m_Flags &= m_FlagsMask;
	}
	else
	{
		FreeItemData();
	}

	p_CookedFiles = pCookedFiles;
}

void CAbstractFileView::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	const LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[m_HoverItem];

	LFGetApp()->ShowTooltip(this, point, pItemDescriptor->CoreAttributes.FileName,
		LFGetApp()->GetHintForItem(pItemDescriptor), NULL, NULL);
}

INT CAbstractFileView::GetItemCategory(INT Index) const
{
	return (p_CookedFiles && p_CookedFiles->m_HasCategories) ? (*p_CookedFiles)[Index]->CategoryID : -1;
}

LFFont* CAbstractFileView::GetLabelFont() const
{
	return &LFGetApp()->m_DefaultFont;
}

RECT CAbstractFileView::GetLabelRect(INT Index) const
{
	return GetItemRect(Index);
}

void CAbstractFileView::EditLabel(INT Index)
{
	m_EditItem = -1;

	if (IsLabelEditEnabled() && (Index>=0) && (Index<m_ItemCount))
	{
		const LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

		if (LFIsStore(pItemDescriptor) || ((pItemDescriptor->Type & (LFTypeMask | LFTypeMounted))==(LFTypeFile | LFTypeMounted)))
		{
			m_EditItem = Index;
			InvalidateItem(Index);
			EnsureVisible(Index);

			LFFont* pFont = GetLabelFont();
			const INT FontHeight = pFont->GetFontHeight();

			CRect rect(GetLabelRect(Index));
			if (rect.Height()>FontHeight+4)
				rect.bottom = (rect.top+=(rect.Height()-FontHeight-4)/2)+FontHeight+4;

			ASSERT(!m_pWndEdit);
			m_pWndEdit = new CEdit();
			m_pWndEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, this, 2);

			m_pWndEdit->SetWindowText(pItemDescriptor->CoreAttributes.FileName);
			m_pWndEdit->SetFont(pFont);
			m_pWndEdit->SetFocus();
			m_pWndEdit->SetSel(0, -1);
		}
	}
}

void CAbstractFileView::DestroyEdit(BOOL Accept)
{
	if (m_pWndEdit)
	{
		const INT EditItem = m_EditItem;

		// Set m_pWndEdit to NULL to avoid recursive calls when the edit window loses focus
		CEdit* pVictim = m_pWndEdit;
		m_pWndEdit = NULL;

		// Get text
		CString Name;
		pVictim->GetWindowText(Name);

		// Destroy window; this will trigger another DestroyEdit() call!
		pVictim->DestroyWindow();
		delete pVictim;

		if (Accept && (EditItem>=0) && !Name.IsEmpty())
			GetOwner()->SendMessage(WM_RENAMEITEM, (WPARAM)EditItem, (LPARAM)(LPCWSTR)Name);
	}

	m_EditItem = -1;
	m_TypingBuffer[0] = L'\0';
}


BEGIN_MESSAGE_MAP(CAbstractFileView, CFrontstageItemView)
	ON_WM_DESTROY()
	ON_WM_MOUSEHOVER()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()

	ON_NOTIFY(HDN_BEGINDRAG, 1, OnBeginDrag)
	ON_NOTIFY(HDN_BEGINTRACK, 1, OnBeginTrack)

	ON_EN_KILLFOCUS(2, OnDestroyEdit)
END_MESSAGE_MAP()

void CAbstractFileView::OnDestroy()
{
	DestroyEdit();

	CFrontstageItemView::OnDestroy();
}

void CAbstractFileView::OnMouseHover(UINT nFlags, CPoint point)
{
	if (!IsEditing())
		if (m_HoverItem==m_EditItem)
		{
			HideTooltip();
			EditLabel(m_EditItem);

			return;
		}

	CFrontstageItemView::OnMouseHover(nFlags, point);
}

void CAbstractFileView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	const BOOL Plain = (GetKeyState(VK_CONTROL)>=0);

	if (Plain && (nChar>=(m_TypingBuffer[0] ? 32u : 33u)) && (m_EditItem==-1))
	{
		if (m_ItemCount>1)
		{
			// Reset typing buffer?
			const DWORD Ticks = GetTickCount();
			if (Ticks-m_TypingTicks>=1000)
				m_TypingBuffer[0] = L'\0';

			m_TypingTicks = Ticks;

			// Concatenate typing buffer
			WCHAR TypingBuffer[256];
			wcsncpy_s(TypingBuffer, 256, m_TypingBuffer, 254);

			WCHAR Letter[2] = { (WCHAR)nChar, L'\0' };
			wcscat_s(TypingBuffer, 256, Letter);

			INT FocusItem = max(m_FocusItem, 0);

			for (INT a=0; a<m_ItemCount; a++)
			{
				if (_wcsnicmp(TypingBuffer, (*p_CookedFiles)[FocusItem]->CoreAttributes.FileName, wcslen(TypingBuffer))==0)
				{
					wcscpy_s(m_TypingBuffer, 256, TypingBuffer);
					SetFocusItem(FocusItem);

					return;
			}

				if (++FocusItem>=m_ItemCount)
					FocusItem = 0;
			}
		}

		LFGetApp()->PlayDefaultSound();
	}

	CFrontstageItemView::OnChar(nChar, nRepCnt, nFlags);
}

void CAbstractFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	const BOOL Plain = (GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0);

	switch (nChar)
	{
	case VK_F2:
		if (Plain && (m_FocusItem>=0) && IsItemSelected(m_FocusItem))
			EditLabel(m_FocusItem);

		break;

	case VK_SPACE:
		// Do not allow space when typing
		if (m_TypingBuffer[0])
			break;

	default:
		CFrontstageItemView::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

BOOL CAbstractFileView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	SetCursor(LFGetApp()->LoadStandardCursor(p_CookedFiles ? IDC_ARROW : IDC_WAIT));

	return TRUE;
}


// Header notifications

void CAbstractFileView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnDestroyEdit();

	CFrontstageItemView::OnBeginDrag(pNMHDR, pResult);
}

void CAbstractFileView::OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnDestroyEdit();

	CFrontstageItemView::OnBeginTrack(pNMHDR, pResult);
}


// Label edit

void CAbstractFileView::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}

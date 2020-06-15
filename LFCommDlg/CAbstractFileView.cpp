
// CAbstractFileView.cpp: Implementierung der Klasse CAbstractFileView
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CAbstractFileView
//

CAbstractFileView::CAbstractFileView(UINT Flags, SIZE_T szData, const CSize& szItemInflate)
	: CFrontstageItemView(Flags, szData, szItemInflate)
{
	ASSERT(Flags & FRONTSTAGE_ENABLEFOCUSITEM);

	p_CookedFiles = NULL;

	m_FlagsMask = Flags;
	m_TypingBuffer[0] = L'\0';
	m_TypingTicks = 0;

	// Item cateogries
	for (UINT a=0; a<LFItemCategoryCount; a++)
		AddItemCategory(LFGetApp()->m_ItemCategories[a].Caption, LFGetApp()->m_ItemCategories[a].Hint);
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


// Item categories

INT CAbstractFileView::GetItemCategory(INT Index) const
{
	return (p_CookedFiles && p_CookedFiles->m_HasCategories) ? (*p_CookedFiles)[Index]->CategoryID : -1;
}


// Item data

void CAbstractFileView::SetSearchResult(LFSearchResult* pCookedFiles)
{
	DestroyEdit();
	HideTooltip();
	ResetDragLocation();

	if (pCookedFiles)
	{
		SetItemCount(pCookedFiles->m_ItemCount, TRUE);

		// Enable shift selection and label edit in certain contexts
		m_Flags &= ~(FRONTSTAGE_ENABLESELECTION | FRONTSTAGE_ENABLESHIFTSELECTION | FRONTSTAGE_ENABLELABELEDIT | FRONTSTAGE_ENABLEEDITONHOVER);

		if (pCookedFiles->m_Context!=LFContextStores)
			m_Flags |= (FRONTSTAGE_ENABLESELECTION | FRONTSTAGE_ENABLESHIFTSELECTION);

		if ((pCookedFiles->m_Context!=LFContextArchive) && (pCookedFiles->m_Context!=LFContextTrash))
			m_Flags |= FRONTSTAGE_ENABLELABELEDIT | FRONTSTAGE_ENABLEEDITONHOVER;

		// Disable flags that weren't initially allowed
		m_Flags &= m_FlagsMask;
	}
	else
	{
		FreeItemData();
	}

	p_CookedFiles = pCookedFiles;
}


// Item handling

void CAbstractFileView::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	if (IsEditing())
		return;

	const LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[m_HoverItem];

	LFGetApp()->ShowTooltip(this, point, pItemDescriptor->CoreAttributes.FileName,
		LFGetApp()->GetHintForItem(pItemDescriptor), NULL, NULL);
}

COLORREF CAbstractFileView::GetItemTextColor(INT Index, BOOL /*Themed*/) const
{
	return ((*p_CookedFiles)[Index]->CoreAttributes.Flags & LFFlagMissing) ? 0x2020FF : (COLORREF)-1;
}


// Label edit

BOOL CAbstractFileView::AllowItemEditLabel(INT Index) const
{
	ASSERT(Index>=0);
	ASSERT(Index<m_ItemCount);

	const LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

	return LFIsStore(pItemDescriptor) || ((pItemDescriptor->Type & (LFTypeMask | LFTypeMounted))==(LFTypeFile | LFTypeMounted));
}

CEdit* CAbstractFileView::CreateLabelEditControl()
{
	ASSERT(m_EditItem>=0);
	ASSERT(m_EditItem<m_ItemCount);

	const LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[m_EditItem];

	CEdit* pWndEdit = CFrontstageItemView::CreateLabelEditControl();
	pWndEdit->SetWindowText(pItemDescriptor->CoreAttributes.FileName);

	return pWndEdit;
}

void CAbstractFileView::EndLabelEdit(INT Index, CString& Value)
{
	ASSERT(Index>=0);
	ASSERT(Index<m_ItemCount);

	if (!Value.IsEmpty())
		GetOwner()->SendMessage(WM_RENAMEITEM, (WPARAM)Index, (LPARAM)(LPCWSTR)Value);
}

void CAbstractFileView::DestroyEdit(BOOL Accept)
{
	CFrontstageItemView::DestroyEdit(Accept);

	m_TypingBuffer[0] = L'\0';
}


BEGIN_MESSAGE_MAP(CAbstractFileView, CFrontstageItemView)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()

	ON_NOTIFY(HDN_BEGINDRAG, 1, OnBeginDrag)
	ON_NOTIFY(HDN_BEGINTRACK, 1, OnBeginTrack)
END_MESSAGE_MAP()

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
	switch (nChar)
	{
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

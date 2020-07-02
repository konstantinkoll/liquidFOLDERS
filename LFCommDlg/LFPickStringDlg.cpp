
// LFPickStringDlg.cpp: Implementierung der Klasse LFPickStringDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFPickStringDlg.h"


// CUnicodeStringList
//

CUnicodeStringList::CUnicodeStringList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM, sizeof(StringItemData))
{
	// Item categories
	AddItemCategory(CString((LPCSTR)IDS_SUGGESTIONS));

	// Item
	SetItemHeight(0, 1, ITEMCELLPADDINGY);
}


// Layouts

void CUnicodeStringList::AdjustLayout()
{
	AdjustLayoutColumns(1, BACKSTAGEBORDER);
}


// Item categories

INT CUnicodeStringList::GetItemCategory(INT /*Index*/) const
{
	return 0;
}


// Item data

void CUnicodeStringList::AddItem(const LFItemDescriptor* pItemDescriptor)
{
	ASSERT(pItemDescriptor);

	StringItemData Data;

	wcscpy_s(Data.Label, 256, pItemDescriptor->CoreAttributes.FileName);
	wcscpy_s(Data.Description, 256, pItemDescriptor->Description);
	Data.FileCount = pItemDescriptor->AggregateCount;

	CFrontstageItemView::AddItem(&Data);
}

void CUnicodeStringList::SetStrings(LFSearchResult* pSearchResult)
{
	// Add strings
	SetItemCount(pSearchResult->m_ItemCount, FALSE);

	for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
	{
		const LFItemDescriptor* pItemDescriptor = (*pSearchResult)[a];

		if (LFIsFolder(pItemDescriptor) && pItemDescriptor->AggregateCount)
			AddItem(pItemDescriptor);
	}

	// Free search result
	LFFreeSearchResult(pSearchResult);

	// Finish
	LastItem();
	AdjustLayout();
}


// Item handling

void CUnicodeStringList::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	const StringItemData* pData = GetStringItemData(m_HoverItem);

	LFGetApp()->ShowTooltip(this, point, pData->Label, pData->Description);
}

void CUnicodeStringList::SelectString(const CString& UnicodeString)
{
	for (INT a=0; a<m_ItemCount; a++)
		if (wcscmp(GetStringItemData(a)->Label, UnicodeString)==0)
		{
			SetFocusItem(a);
			EnsureVisible(a);

			break;
		}
}


// Item selection

CString CUnicodeStringList::GetSelectedString() const
{
	const INT Index = GetSelectedItem();

	return (Index>=0) ? GetStringItemData(Index)->Label : _T("");
}


// Drawing

void CUnicodeStringList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	const StringItemData* pData = GetStringItemData(Index);

	DrawCountItem(dc, rectItem, Index, Themed, pData->Label, pData->FileCount);
}


// LFPickStringDlg
//

LFPickStringDlg::LFPickStringDlg(ATTRIBUTE Attr, ITEMCONTEXT Context, const CString& UnicodeString, CWnd* pParentWnd)
	: CAttributePickDlg(Attr, Context, IDD_PICKSTRING, pParentWnd)
{
	ASSERT(LFGetApp()->m_Attributes[Attr].AttrProperties.Type==LFTypeUnicodeString);

	m_UnicodeString = UnicodeString;
	m_SelectString = TRUE;
}

void LFPickStringDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
		m_UnicodeString = m_wndStringList.GetSelectedString();
}

void LFPickStringDlg::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	CAttributePickDlg::AdjustLayout(rectLayout, nFlags);

	if (IsWindow(m_wndStringList))
	{
		m_wndStringList.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), m_BottomDivider-rectLayout.top, nFlags);

		if (m_SelectString)
		{
			m_wndStringList.SelectString(m_UnicodeString);
			m_SelectString = FALSE;
		}
	}
}

BOOL LFPickStringDlg::InitDialog()
{
	CAttributePickDlg::InitDialog();

	// String list
	m_wndStringList.Create(this, IDC_STRINGLIST);
	m_wndStringList.SetStrings(RunQuery());

	m_wndStringList.SetFocus();

	return FALSE;
}


BEGIN_MESSAGE_MAP(LFPickStringDlg, CAttributePickDlg)
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(IVN_SELECTIONCHANGED, IDC_STRINGLIST, OnSelectionChanged)
END_MESSAGE_MAP()

void LFPickStringDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CAttributePickDlg::OnGetMinMaxInfo(lpMMI);

	LFDialog::OnGetMinMaxInfo(lpMMI);

	if (IsWindowVisible())
	{
		CRect rect;
		GetWindowRect(rect);

		lpMMI->ptMinTrackSize.x = lpMMI->ptMaxTrackSize.x = rect.Width();
	}

	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
}

void LFPickStringDlg::OnSelectionChanged(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	GetDlgItem(IDOK)->EnableWindow(!m_wndStringList.GetSelectedString().IsEmpty());

	*pResult = 0;
}

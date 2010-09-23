
// CStoreDropdownSelector.cpp: Implementierung der Klasse CStoreDropdownSelector
//

#include "stdafx.h"
#include "CStoreSelector.h"
#include "resource.h"


// CStoreDropdownWindow
//

extern LFMessageIDs* MessageIDs;

CStoreDropdownWindow::CStoreDropdownWindow()
	: CDropdownWindow()
{
	result = NULL;
}


BEGIN_MESSAGE_MAP(CStoreDropdownWindow, CDropdownWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(MessageIDs->StoresChanged, OnUpdateStores)
	ON_REGISTERED_MESSAGE(MessageIDs->StoreAttributesChanged, OnUpdateStores)
	ON_REGISTERED_MESSAGE(MessageIDs->DefaultStoreChanged, OnUpdateStores)
	ON_NOTIFY(LVN_ITEMCHANGED, 1, OnItemChanged)
	ON_BN_CLICKED(IDOK, OnCreateNewStore)
END_MESSAGE_MAP()

int CStoreDropdownWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDropdownWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	LFApplication* pApp = (LFApplication*)AfxGetApp();
	m_wndList.SetImageList(&pApp->m_CoreImageListSmall, LVSIL_SMALL);
	m_wndList.SetImageList(&pApp->m_CoreImageListLarge, LVSIL_NORMAL);

	IMAGEINFO ii;
	pApp->m_CoreImageListLarge.GetImageInfo(0, &ii);
	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&pApp->m_DefaultFont);
	m_wndList.SetIconSpacing(CXDropdownListIconSpacing, ii.rcImage.bottom-ii.rcImage.top+dc->GetTextExtent(_T("Wy"), 2).cy*2+4);
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_wndList.AddStoreColumns();
	m_wndList.AddItemCategories();
	m_wndList.EnableGroupView(TRUE);
	m_wndList.SetView(LV_VIEW_TILE);

	SendMessage(MessageIDs->StoresChanged, LFMSGF_IntStores | LFMSGF_ExtHybStores);

	return 0;
}

void CStoreDropdownWindow::OnDestroy()
{
	LFFreeSearchResult(result);

	CDropdownWindow::OnDestroy();
}

LRESULT CStoreDropdownWindow::OnUpdateStores(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (result)
		LFFreeSearchResult(result);

	LFFilter* filter = LFAllocFilter();
	result = LFQuery(filter);
	LFFreeFilter(filter);

	m_wndList.SetSearchResult(result);

	return NULL;
}

void CStoreDropdownWindow::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
		GetOwner()->SendMessage(WM_SETITEM, NULL, (LPARAM)result->m_Items[pNMListView->iItem]);
}

void CStoreDropdownWindow::OnCreateNewStore()
{
	CWnd* pOwnerWnd = GetOwner();
	pOwnerWnd->PostMessage(WM_CLOSEDROPDOWN);

	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(AfxGetApp()->GetMainWnd(), s);
	if (dlg.DoModal()==IDOK)
	{
		UINT res = LFCreateStore(s, dlg.MakeDefault);
		LFErrorBox(res);

		if (res==LFOk)
		{
			LFItemDescriptor* i = LFAllocItemDescriptor(s);
			pOwnerWnd->SendMessage(WM_SETITEM, NULL, (LPARAM)i);
			LFFreeItemDescriptor(i);
		}
	}

	LFFreeStoreDescriptor(s);
}


// CStoreSelector
//

CStoreSelector::CStoreSelector()
	: CDropdownSelector()
{
	item = NULL;
}

CStoreSelector::~CStoreSelector()
{
	LFFreeItemDescriptor(item);
}

void CStoreSelector::CreateDropdownWindow()
{
	p_DropWindow = new CStoreDropdownWindow();
	p_DropWindow->Create(this, IDD_CREATENEWSTORE);
}

void CStoreSelector::SetEmpty(BOOL Repaint)
{
	LFFreeItemDescriptor(item);
	item = NULL;

	CDropdownSelector::SetEmpty(Repaint);
}

void CStoreSelector::SetItem(LFItemDescriptor* _item, BOOL Repaint, UINT NotifyCode)
{
	if (_item)
	{
		LFFreeItemDescriptor(item);
		item = LFAllocItemDescriptor(_item);

		CDropdownSelector::SetItem(p_App->m_CoreImageListSmall.ExtractIcon(item->IconID-1), item->CoreAttributes.FileName, Repaint, NotifyCode);
	}
	else
	{
		SetEmpty(Repaint);
	}
}

void CStoreSelector::SetItem(LFStoreDescriptor* s, BOOL Repaint, UINT NotifyCode)
{
	if (s)
	{
		LFFreeItemDescriptor(item);
		item = LFAllocItemDescriptor(s);

		CDropdownSelector::SetItem(p_App->m_CoreImageListSmall.ExtractIcon(item->IconID-1), item->CoreAttributes.FileName, Repaint, NotifyCode);
	}
	else
	{
		SetEmpty(Repaint);
	}
}

void CStoreSelector::SetItem(char* Key, BOOL Repaint)
{
	if (Key)
	{
		LFStoreDescriptor* s = LFAllocStoreDescriptor();

		if (LFGetStoreSettings(Key, s)==LFOk)
			SetItem(s, Repaint);

		LFFreeStoreDescriptor(s);
	}
	else
	{
		SetEmpty(Repaint);
	}
}

BOOL CStoreSelector::GetStoreID(char* StoreID)
{
	if (m_IsEmpty)
	{
		*StoreID = '\0';
		return FALSE;
	}

	strcpy_s(StoreID, LFKeySize, item->StoreID);
	return TRUE;
}

void CStoreSelector::Update()
{
	char StoreID[LFKeySize];
	if (GetStoreID(StoreID))
	{
		LFStoreDescriptor* s = LFAllocStoreDescriptor();
		if (LFGetStoreSettings(StoreID, s)==LFOk)
		{
			SetItem(s, TRUE, NM_SELUPDATE);
		}
		else
		{
			SetEmpty();
		}
		LFFreeStoreDescriptor(s);
	}
}

void CStoreSelector::GetTooltipData(HICON& hIcon, CSize& size, CString& caption, CString& hint)
{
	hIcon = p_App->m_CoreImageListLarge.ExtractIcon(item->IconID-1);
	
	int cx = GetSystemMetrics(SM_CXICON);
	int cy = GetSystemMetrics(SM_CYICON);
	ImageList_GetIconSize(p_App->m_CoreImageListLarge, &cx, &cy);
	size.SetSize(cx, cy);

	caption = item->CoreAttributes.FileName;
	hint = item->CoreAttributes.Comment;

	if (item->Description[0]!=L'\0')
	{
		if (!hint.IsEmpty())
			hint.Append(_T("\n"));

		hint.Append(item->Description);
	}

	if (!hint.IsEmpty())
		hint.Append(_T("\n"));

	FILETIME lft;
	wchar_t tmpBuf1[256];
	FileTimeToLocalFileTime(&item->CoreAttributes.CreationTime, &lft);
	LFTimeToString(lft, tmpBuf1, 256);
	wchar_t tmpBuf2[256];
	FileTimeToLocalFileTime(&item->CoreAttributes.FileTime, &lft);
	LFTimeToString(lft, tmpBuf2, 256);

	CString tmpStr;
	tmpStr.Format(_T("%s: %s\n%s: %s"),
		p_App->m_Attributes[LFAttrCreationTime]->Name, tmpBuf1,
		p_App->m_Attributes[LFAttrFileTime]->Name, tmpBuf2);
	hint.Append(tmpStr);
}


BEGIN_MESSAGE_MAP(CStoreSelector, CDropdownSelector)
	ON_MESSAGE(WM_SETITEM, OnSetItem)
END_MESSAGE_MAP()

LRESULT CStoreSelector::OnSetItem(WPARAM /*wParam*/, LPARAM lParam)
{
	SetItem((LFItemDescriptor*)lParam);
	return NULL;
}

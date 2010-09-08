
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

	m_wndList.AddStoreColumns();
	m_wndList.AddItemCategories();
	m_wndList.EnableGroupView(TRUE);

/*	for (UINT a=0; a<4; a++)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_FOLDERCATEGORY1+a));
		m_wndList.AddCategory(a, tmpStr);
	}

	m_wndList.SetImageList(&theApp.m_SystemImageListLarge, LVSIL_NORMAL);*/

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

//	for (UINT a=0; a<5; a++)
//		m_wndList.SetColumnWidth(a, LVSCW_AUTOSIZE_USEHEADER);

	m_wndList.SetRedraw(TRUE);

	return NULL;
}

void CStoreDropdownWindow::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		//LFStoreDescriptor* idl = (LPITEMIDLIST)m_wndList.GetItemData(pNMListView->iItem);
		//GetOwner()->SendMessage(WM_SETITEM, NULL, (LPARAM)pidl);
	}
}

void CStoreDropdownWindow::OnCreateNewStore()
{
	CWnd* pOwnerWnd = GetOwner();
	pOwnerWnd->PostMessage(WM_CLOSEDROPDOWN);

	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(this, IDD_STORENEW, '\0', s);
	if (dlg.DoModal()==IDOK)
	{
		UINT res = LFCreateStore(s, dlg.makeDefault);
		LFErrorBox(res);

		if (res==LFOk)
			pOwnerWnd->SendMessage(WM_SETITEM, NULL, (LPARAM)s);
	}

	LFFreeStoreDescriptor(s);
}


// CDropdownSelector
//

CStoreSelector::CStoreSelector()
	: CDropdownSelector()
{
	store = NULL;
}

CStoreSelector::~CStoreSelector()
{
	LFFreeStoreDescriptor(store);
}

void CStoreSelector::CreateDropdownWindow()
{
	p_DropWindow = new CStoreDropdownWindow();
	p_DropWindow->Create(this, IDD_CREATENEWSTORE);
}

void CStoreSelector::SetEmpty(BOOL Repaint)
{
	LFFreeStoreDescriptor(store);
	store = NULL;

	CDropdownSelector::SetEmpty(Repaint);
}

void CStoreSelector::SetItem(LFStoreDescriptor* _store, BOOL Repaint)
{
	if (_store)
	{
		LFFreeStoreDescriptor(store);
		store = LFAllocStoreDescriptor();
		*store = *_store;

		CDropdownSelector::SetItem(_T(""), NULL, _store->StoreName, Repaint);
/*		else
		{
			SetEmpty();
		}*/
	}
	else
	{
		SetEmpty();
	}
}

void CStoreSelector::GetTooltipData(HICON& hIcon, CSize& size, CString& caption, CString& hint)
{
	size.cx = GetSystemMetrics(SM_CXICON);
	size.cy = GetSystemMetrics(SM_CYICON);

	caption = store->StoreName;
	hint = store->Comment;
//	TooltipDataFromPIDL(pidl, &theApp.m_SystemImageListLarge, hIcon, size, caption, hint);
}


BEGIN_MESSAGE_MAP(CStoreSelector, CDropdownSelector)
	ON_MESSAGE(WM_SETITEM, OnSetItem)
END_MESSAGE_MAP()

LRESULT CStoreSelector::OnSetItem(WPARAM /*wParam*/, LPARAM lParam)
{
	SetItem((LFStoreDescriptor*)lParam);
	return NULL;
}

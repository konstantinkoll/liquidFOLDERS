
// CStoreDropdownSelector.cpp: Implementierung der Klasse CStoreDropdownSelector
//

#include "stdafx.h"
#include "CStoreSelector.h"
#include "resource.h"


// CStoreDropdownWindow
//

CStoreDropdownWindow::CStoreDropdownWindow()
	: CDropdownWindow()
{
}

void CStoreDropdownWindow::PopulateList()
{
	m_wndList.DeleteAllItems();
}


BEGIN_MESSAGE_MAP(CStoreDropdownWindow, CDropdownWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, 1, OnItemChanged)
	ON_BN_CLICKED(IDOK, OnCreateNewStore)
END_MESSAGE_MAP()

int CStoreDropdownWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDropdownWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

/*	for (UINT a=0; a<4; a++)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_FOLDERCATEGORY1+a));
		m_wndList.AddCategory(a, tmpStr);
	}

	m_wndList.SetImageList(&theApp.m_SystemImageListLarge, LVSIL_NORMAL);*/
	m_wndList.EnableGroupView(TRUE);

	PopulateList();
	return 0;
}

void CStoreDropdownWindow::OnDestroy()
{

	CDropdownWindow::OnDestroy();
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
	GetOwner()->PostMessage(WM_CLOSEDROPDOWN);

	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(this, IDD_STORENEW, '\0', s);
	if (dlg.DoModal()==IDOK)
	{
		UINT res = LFCreateStore(s, dlg.makeDefault);
		LFErrorBox(res);

		if (res==LFOk)
			GetOwner()->SendMessage(WM_SETITEM, NULL, (LPARAM)s);
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

/*		SHFILEINFO sfi;
		if (SUCCEEDED(SHGetFileInfo((wchar_t*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON)))
		{
			CString tmpStr;
			ENSURE(tmpStr.LoadString(IDS_FOLDER_CAPTION));
			CDropdownSelector::SetItem(tmpStr, sfi.hIcon, sfi.szDisplayName, Repaint);
		}
		else
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
	caption = store->StoreName;
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

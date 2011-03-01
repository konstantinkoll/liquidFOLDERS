
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
	p_Result = NULL;
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

INT CStoreDropdownWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDropdownWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	LFApplication* pApp = (LFApplication*)AfxGetApp();
	m_wndList.SetImageList(&pApp->m_CoreImageListSmall, LVSIL_SMALL);
	m_wndList.SetImageList(&pApp->m_CoreImageListLarge, LVSIL_NORMAL);

	INT cx = GetSystemMetrics(SM_CXSMICON);
	INT cy = GetSystemMetrics(SM_CYSMICON);
	ImageList_GetIconSize(pApp->m_CoreImageListLarge, &cx, &cy);

	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&pApp->m_DefaultFont);
	m_wndList.SetIconSpacing(CXDropdownListIconSpacing, cy+dc->GetTextExtent(_T("Wy")).cy*2+4);
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
	LFFreeSearchResult(p_Result);

	CDropdownWindow::OnDestroy();
}

LRESULT CStoreDropdownWindow::OnUpdateStores(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (p_Result)
		LFFreeSearchResult(p_Result);

	LFFilter* filter = LFAllocFilter();
	p_Result = LFQuery(filter);
	LFFreeFilter(filter);

	m_wndList.SetSearchResult(p_Result);

	return NULL;
}

void CStoreDropdownWindow::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
		GetOwner()->SendMessage(WM_SETITEM, NULL, (LPARAM)p_Result->m_Items[pNMListView->iItem]);
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
	p_Item = NULL;
}

CStoreSelector::~CStoreSelector()
{
	LFFreeItemDescriptor(p_Item);
}

void CStoreSelector::CreateDropdownWindow()
{
	p_DropWindow = new CStoreDropdownWindow();
	p_DropWindow->Create(this, IDD_CREATENEWSTORE);
}

void CStoreSelector::SetEmpty(BOOL Repaint)
{
	LFFreeItemDescriptor(p_Item);
	p_Item = NULL;

	CDropdownSelector::SetEmpty(Repaint);
}

void CStoreSelector::SetItem(LFItemDescriptor* _item, BOOL Repaint, UINT NotifyCode)
{
	if (_item)
	{
		LFFreeItemDescriptor(p_Item);
		p_Item = LFAllocItemDescriptor(_item);

		CDropdownSelector::SetItem(p_App->m_CoreImageListSmall.ExtractIcon(p_Item->IconID-1), p_Item->CoreAttributes.FileName, Repaint, NotifyCode);
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
		LFFreeItemDescriptor(p_Item);
		p_Item = LFAllocItemDescriptor(s);

		CDropdownSelector::SetItem(p_App->m_CoreImageListSmall.ExtractIcon(p_Item->IconID-1), p_Item->CoreAttributes.FileName, Repaint, NotifyCode);
	}
	else
	{
		SetEmpty(Repaint);
	}
}

void CStoreSelector::SetItem(CHAR* Key, BOOL Repaint)
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

BOOL CStoreSelector::GetStoreID(CHAR* StoreID)
{
	if (m_IsEmpty)
	{
		*StoreID = '\0';
		return FALSE;
	}

	strcpy_s(StoreID, LFKeySize, p_Item->StoreID);
	return TRUE;
}

void CStoreSelector::Update()
{
	CHAR StoreID[LFKeySize];
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

void CStoreSelector::GetTooltipData(HICON& hIcon, CSize& Size, CString& Caption, CString& Hint)
{
	hIcon = p_App->m_CoreImageListExtraLarge.ExtractIcon(p_Item->IconID-1);
	
	INT cx = 48;
	INT cy = 48;
	ImageList_GetIconSize(p_App->m_CoreImageListExtraLarge, &cx, &cy);
	Size.SetSize(cx, cy);

	Caption = p_Item->CoreAttributes.FileName;
	Hint = p_Item->CoreAttributes.Comment;

	if (p_Item->Description[0]!=L'\0')
	{
		if (!Hint.IsEmpty())
			Hint.Append(_T("\n"));

		Hint.Append(p_Item->Description);
	}

	if (!Hint.IsEmpty())
		Hint.Append(_T("\n"));

	FILETIME lft;
	WCHAR tmpBuf1[256];
	FileTimeToLocalFileTime(&p_Item->CoreAttributes.CreationTime, &lft);
	LFTimeToString(lft, tmpBuf1, 256);
	WCHAR tmpBuf2[256];
	FileTimeToLocalFileTime(&p_Item->CoreAttributes.FileTime, &lft);
	LFTimeToString(lft, tmpBuf2, 256);

	CString tmpStr;
	tmpStr.Format(_T("%s: %s\n%s: %s"),
		p_App->m_Attributes[LFAttrCreationTime]->Name, tmpBuf1,
		p_App->m_Attributes[LFAttrFileTime]->Name, tmpBuf2);
	Hint.Append(tmpStr);
}


BEGIN_MESSAGE_MAP(CStoreSelector, CDropdownSelector)
	ON_MESSAGE(WM_SETITEM, OnSetItem)
END_MESSAGE_MAP()

LRESULT CStoreSelector::OnSetItem(WPARAM /*wParam*/, LPARAM lParam)
{
	SetItem((LFItemDescriptor*)lParam);
	return NULL;
}

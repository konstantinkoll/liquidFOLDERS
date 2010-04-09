#include "StdAfx.h"
#include "LFChooseDefaultStoreDlg.h"
#include "LFStoreNewDlg.h"
#include "Kitchen.h"
#include "Resource.h"

// LFChooseDefaultStoreDlg

extern AFX_EXTENSION_MODULE LFCommDlgDLL;
extern LFMessageIDs* MessageIDs;

LFChooseDefaultStoreDlg::LFChooseDefaultStoreDlg(CWnd* pParentWnd)
	: CDialog(IDD_CHOOSEDEFAULTSTORE, pParentWnd)
{
	m_icStore = NULL;
	m_icDefaultStore = NULL;
	m_Icons = NULL;
	result = NULL;
	StoreID[0] = '\0';
}

LFChooseDefaultStoreDlg::~LFChooseDefaultStoreDlg()
{
	if (m_icStore)
		DestroyIcon(m_icStore);
	if (m_icDefaultStore)
		DestroyIcon(m_icDefaultStore);
	if (m_Icons)
		delete m_Icons;
	if (result)
		LFFreeSearchResult(result);
}


BEGIN_MESSAGE_MAP(LFChooseDefaultStoreDlg, CDialog)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_STORELIST, OnGetDispInfo)
	ON_NOTIFY(NM_DBLCLK, IDC_STORELIST, OnDoubleClick)
	ON_BN_CLICKED(IDC_NEWSTORE, OnNewStore)
	ON_REGISTERED_MESSAGE(MessageIDs->StoresChanged, UpdateStores)
	ON_REGISTERED_MESSAGE(MessageIDs->StoreAttributesChanged, UpdateStores)
	ON_REGISTERED_MESSAGE(MessageIDs->DefaultStoreChanged, UpdateStores)
END_MESSAGE_MAP()

BOOL LFChooseDefaultStoreDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Icons laden
	HINSTANCE hModIcons = LoadLibrary(_T("LFCore.DLL"));
	if (hModIcons)
	{
		m_icStore = (HICON)LoadImage(hModIcons, MAKEINTRESOURCE(IDI_STORE_Empty), IMAGE_ICON, 16, 16, LR_LOADTRANSPARENT);
		m_icDefaultStore = (HICON)LoadImage(hModIcons, MAKEINTRESOURCE(IDI_STORE_Default), IMAGE_ICON, 16, 16, LR_LOADTRANSPARENT);
		FreeLibrary(hModIcons);

		m_Icons = new CImageList();
		m_Icons->Create(16, 16, ILC_COLOR32, 2, 1);
		m_Icons->Add(m_icStore);
		m_Icons->Add(m_icDefaultStore);
	}

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_icDefaultStore, TRUE);		// Großes Symbol verwenden
	SetIcon(m_icDefaultStore, FALSE);		// Kleines Symbol verwenden

	// Store-Liste füllen
	const UINT dwExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	m_List.SetExtendedStyle(m_List.GetExtendedStyle() | dwExStyle);
	m_List.SetImageList(m_Icons, LVSIL_SMALL);
	m_List.EnableTheming();

	AddColumn(&m_List, LFAttrFileName, 0);
	AddColumn(&m_List, LFAttrComment, 1);
	AddColumn(&m_List, LFAttrCreationTime, 2);
	AddColumn(&m_List, LFAttrStoreID, 3);
	AddColumn(&m_List, LFAttrHint, 4);

	SendMessage(MessageIDs->StoresChanged, LFMSGF_IntStores);
	LFErrorBox(result->m_LastError);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFChooseDefaultStoreDlg::AddColumn(CListCtrl* l, UINT attr, UINT no)
{
	LFAttributeDescriptor* i = LFGetAttributeInfo(attr);

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = i->Name;
	lvc.cx = i->RecommendedWidth;
	lvc.fmt = ((attr==LFAttrStoreID) || (attr==LFAttrFileID)) ? LVCFMT_RIGHT : LVCFMT_LEFT;
	lvc.iSubItem = no;
	l->InsertColumn(no, &lvc);

	LFFreeAttributeDescriptor(i);
}

LRESULT LFChooseDefaultStoreDlg::UpdateStores(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam & LFMSGF_IntStores)
	{
		char StoreID[LFKeySize] = "";
		int idx = m_List.GetNextItem(-1, LVIS_SELECTED);
		if ((idx!=-1) && (result))
			strcpy_s(StoreID, LFKeySize, result->m_Files[idx]->CoreAttributes.StoreID);

		m_List.SetRedraw(FALSE);

		LFFilter* filter = LFAllocFilter();
		filter->Legacy = TRUE;

		if (result)
		{
			m_List.SetItemCount(0);
			LFFreeSearchResult(result);
			result = NULL;
		}
		result = LFQuery(filter);
		LFFreeFilter(filter);

		RemoveNoninternalStores(result);
		SortSearchResult(result);

		((CButton*)GetDlgItem(IDOK))->EnableWindow(result->m_Count);
		m_List.SetItemCount(result->m_Count);
		for (UINT a=0; a<5; a++)
			m_List.SetColumnWidth(a, LVSCW_AUTOSIZE_USEHEADER);

		idx = -1;
		for (UINT a=0; a<result->m_Count; a++)
			if (((idx==-1) && (result->m_Files[a]->Type & LFTypeDefaultStore)) || (!strcmp(StoreID, result->m_Files[a]->CoreAttributes.StoreID)))
				idx = a;

		m_List.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_List.SetRedraw(TRUE);
	}

	return NULL;
}

void LFChooseDefaultStoreDlg::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	int idx = pItem->iItem;

	if (pItem->mask & LVIF_TEXT)
	{
		const UINT attrs[5] = { LFAttrFileName, LFAttrComment, LFAttrCreationTime, LFAttrStoreID, LFAttrHint };
		pItem->pszText = (LPWSTR)result->m_Files[idx]->AttributeStrings[attrs[pItem->iSubItem]];
	}

	if (pItem->mask & LVIF_IMAGE)
		pItem->iImage = (result->m_Files[idx]->Type & LFTypeDefaultStore);
}

void LFChooseDefaultStoreDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	if (result)
		if (result->m_Count)
			PostMessage(WM_COMMAND, (WPARAM)IDOK);
}

void LFChooseDefaultStoreDlg::OnNewStore()
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(this, IDD_STORENEW, '\0', s);
	if (dlg.DoModal()==IDOK)
		LFErrorBox(LFCreateStore(s, dlg.makeDefault));

	LFFreeStoreDescriptor(s);
}

void LFChooseDefaultStoreDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STORELIST, m_List);

	if ((pDX->m_bSaveAndValidate) && (result))
	{
		int idx = m_List.GetNextItem(-1, LVIS_SELECTED);
		if (idx!=-1)
		{
			strcpy_s(StoreID, LFKeySize, result->m_Files[idx]->CoreAttributes.StoreID);
			LFErrorBox(LFMakeDefaultStore(StoreID), GetSafeHwnd());
		}
		else
		{
			strcpy_s(StoreID, LFKeySize, "");
		}
	}
}

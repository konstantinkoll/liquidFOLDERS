
// CFileView.cpp: Implementierung der Klasse CFileView
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CFileView
//

#define GetItemData(Index)        ((FVItemData*)(m_pItemData+Index*m_DataSize))
#define FIRSTSENDTO               0xFF00
#define HORIZONTALSCROLLWIDTH     64

CIcons CFileView::m_LargeColorDots;
CIcons CFileView::m_DefaultColorDots;

CFileView::CFileView(SIZE_T DataSize, UINT Flags)
	: CFrontstageWnd()
{
	ASSERT(DataSize>=sizeof(FVItemData));

	p_Filter = NULL;
	p_RawFiles = p_CookedFiles = NULL;
	m_pWndEdit = NULL;
	m_pItemData = NULL;
	m_ItemDataAllocated = m_HeaderHeight = m_HScrollMax = m_VScrollMax = m_HScrollPos = m_VScrollPos = 0;
	m_FocusItem = m_HotItem = m_SelectionAnchor = m_EditLabel = m_Context = -1;
	m_Context = LFContextAllFiles;
	m_DataSize = DataSize;
	m_Flags = Flags;
	m_Nothing = TRUE;
	m_Hover = m_BeginDragDrop = m_ShowFocusRect = m_AllowMultiSelect = FALSE;
	m_TypingBuffer[0] = L'\0';
	m_TypingTicks = 0;

	ZeroMemory(&m_Bitmaps, sizeof(m_Bitmaps));
}

CFileView::~CFileView()
{
	DestroyEdit();

	for (UINT a=0; a<2; a++)
		DeleteObject(m_Bitmaps[a].hBitmap);

	if (m_pItemData)
		free(m_pItemData);
}

BOOL CFileView::Create(CWnd* pParentWnd, UINT nID, const CRect& rect, LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData, UINT nClassStyle)
{
	CString className = AfxRegisterWndClass(nClassStyle, theApp.LoadStandardCursor(IDC_ARROW));

	if (!CFrontstageWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, rect, pParentWnd, nID))
		return FALSE;

	UpdateViewSettings(pCookedFiles ? pCookedFiles->m_Context : LFContextAllFiles, TRUE);
	UpdateSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData, TRUE);

	return TRUE;
}

BOOL CFileView::PreTranslateMessage(MSG* pMsg)
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
		theApp.HideTooltip();
		break;
	}

	return CFrontstageWnd::PreTranslateMessage(pMsg);
}

void CFileView::UpdateViewSettings(INT Context, BOOL UpdateSearchResultPending)
{
	DestroyEdit();
	theApp.HideTooltip();

	// Set new context
	if (Context>=0)
		m_Context = Context;

	p_ContextViewSettings = &theApp.m_ContextViewSettings[m_Context];
	p_GlobalViewSettings = &theApp.m_GlobalViewSettings;

	// Allow view subclass to see both old and new settings
	m_BeginDragDrop = FALSE;
	SetViewSettings(UpdateSearchResultPending);

	// Copy settings
	m_ContextViewSettings = *p_ContextViewSettings;
	m_GlobalViewSettings = *p_GlobalViewSettings;
}

void CFileView::UpdateSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData, BOOL InternalCall)
{
	DestroyEdit();
	theApp.HideTooltip();

	LPBYTE pVictim = m_pItemData;

	m_Nothing = TRUE;

	if (pCookedFiles)
	{
		const SIZE_T Size = (SIZE_T)(m_ItemDataAllocated=pCookedFiles->m_ItemCount)*m_DataSize;
		ZeroMemory(m_pItemData=(LPBYTE)malloc(Size), Size);

		m_AllowMultiSelect = (pCookedFiles->m_Context!=LFContextStores);

		p_ContextViewSettings = &theApp.m_ContextViewSettings[m_Context=pCookedFiles->m_Context];
		m_ContextViewSettings.SortBy = p_ContextViewSettings->SortBy;

		m_FocusItem = pPersistentData ? min(pPersistentData->FocusItem, (INT)pCookedFiles->m_ItemCount-1) : pCookedFiles->m_ItemCount ? 0 : -1;
		m_HScrollPos = pPersistentData ? pPersistentData->HScrollPos : 0;
		m_VScrollPos = pPersistentData ? pPersistentData->VScrollPos : 0;
		m_HotItem = -1;
		m_HideFileExt = LFHideFileExt();
	}
	else
	{
		m_pItemData = NULL;
		m_ItemDataAllocated = 0;

		m_FocusItem = m_HotItem = -1;
		m_HScrollPos = m_VScrollPos = 0;

		m_ShowFocusRect = FALSE;
	}

	m_BeginDragDrop = FALSE;
	SetSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData);

	free(pVictim);

	if (p_CookedFiles)
	{
		BOOL NeedsNewFocusItem = (m_FocusItem>=0) ? !GetItemData(m_FocusItem)->Valid : TRUE;

		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
			if (GetItemData(a)->Valid)
			{
				m_Nothing = FALSE;

				if (NeedsNewFocusItem)
				{
					m_FocusItem = a;
					NeedsNewFocusItem = FALSE;
				}
			}
			else
			{
				SelectItem(a, FALSE, TRUE);
			}

		if (pPersistentData && (pPersistentData->FocusItem!=-1) && pPersistentData->FocusItemSelected)
			SelectItem(m_FocusItem, TRUE, TRUE);

		AdjustLayout();

		if (!InternalCall)
			EnsureVisible(m_FocusItem);
	}
	else
	{
		Invalidate();
	}

	SetCursor(theApp.LoadStandardCursor(pCookedFiles ? IDC_ARROW : IDC_WAIT));
}

void CFileView::SetViewSettings(BOOL /*UpdateSearchResultPending*/)
{
}

void CFileView::SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* /*Data*/)
{
	p_Filter = pFilter;
	p_RawFiles = pRawFiles;
	p_CookedFiles = pCookedFiles;
}

void CFileView::ValidateAllItems()
{
	if (p_CookedFiles)
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
			GetItemData(a)->Valid = TRUE;
}

void CFileView::AdjustLayout()
{
	AdjustScrollbars();
	Invalidate();
}

INT CFileView::GetFocusItem() const
{
	if (p_CookedFiles && (m_FocusItem>=0))
		if (GetItemData(m_FocusItem)->Valid)
			return m_FocusItem;

	return -1;
}

INT CFileView::GetSelectedItem() const
{
	const INT FocusItem = GetFocusItem();

	if (FocusItem>=0)
		if (IsItemSelected(FocusItem))
			return FocusItem;

	return -1;
}

BOOL CFileView::HasItemsSelected() const
{
	if (p_CookedFiles)
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
			if (IsItemSelected(a))
				return TRUE;

	return FALSE;
}

void CFileView::SelectItem(INT Index, BOOL Select, BOOL InternalCall)
{
	if (p_CookedFiles)
	{
		ASSERT(Index>=0);
		ASSERT(Index<(INT)p_CookedFiles->m_ItemCount);

		if (GetItemData(Index)->Valid)
		{
			LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];
			SelectItem(pItemDescriptor, Select);

			if (((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder) && (pItemDescriptor->AggregateFirst!=-1))
				for (INT a=pItemDescriptor->AggregateFirst; a<=pItemDescriptor->AggregateLast; a++)
					SelectItem((*p_RawFiles)[a], Select);

			if (!InternalCall)
				ChangedItem(Index);
		}
	}
}

void CFileView::EnsureVisible(INT Index)
{
	if ((m_Flags & FF_ENABLESCROLLING)==0)
		return;

	if (!p_CookedFiles)
		return;

	if ((Index<0) || (Index>=(INT)p_CookedFiles->m_ItemCount))
		return;

	CRect rect;
	GetClientRect(rect);

	RECT rectItem = GetItemRect(Index);

	// Vertical
	INT nInc = 0;

	if (rectItem.bottom>rect.Height())
		nInc = rectItem.bottom-rect.Height();

	if (rectItem.top<nInc+(INT)m_HeaderHeight)
		nInc = rectItem.top-(INT)m_HeaderHeight;

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);
	}

	// Horizontal
	if (m_ContextViewSettings.View!=LFViewList)
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
			ScrollWindow(-nInc, 0);
			SetScrollPos(SB_HORZ, m_HScrollPos);
		}
	}
}

void CFileView::SetFocusItem(INT FocusItem, BOOL ShiftSelect, BOOL Deselect)
{
	if (!m_AllowMultiSelect)
		ShiftSelect = FALSE;

	if (ShiftSelect && (m_Flags & FF_ENABLESHIFTSELECTION))
	{
		if (m_SelectionAnchor==-1)
			m_SelectionAnchor = m_FocusItem;

		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			SelectItem(a, ((a>=FocusItem) && (a<=m_SelectionAnchor)) || ((a>=m_SelectionAnchor) && (a<=FocusItem)), TRUE);
	}
	else
	{
		m_SelectionAnchor = -1;

		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			SelectItem(a, Deselect ? (a==FocusItem) : (a==FocusItem) | IsItemSelected(a), TRUE);
	}

	m_FocusItem = FocusItem;
	m_EditLabel = -1;

	ChangedItems();
	EnsureVisible(m_FocusItem);
}

RECT CFileView::GetItemRect(INT Index) const
{
	ASSERT(p_CookedFiles);
	ASSERT(Index>=0);
	ASSERT(Index<(INT)p_CookedFiles->m_ItemCount);

	RECT rect = GetItemData(Index)->Rect;
	OffsetRect(&rect, -m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);

	return rect;
}

LFFont* CFileView::GetLabelFont() const
{
	return &theApp.m_DefaultFont;
}

RECT CFileView::GetLabelRect(INT Index) const
{
	return GetItemRect(Index);
}

INT CFileView::ItemAtPosition(CPoint point) const
{
	if (p_CookedFiles)
	{
		point.Offset(m_HScrollPos, m_VScrollPos-m_HeaderHeight);

		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
		{
			FVItemData* pData = GetItemData(a);

			if (pData->Valid && PtInRect(&pData->Rect, point))
				return a;
		}
	}

	return -1;
}

void CFileView::InvalidateItem(INT Index)
{
	if (p_CookedFiles)
		if ((Index>=0) && (Index<(INT)p_CookedFiles->m_ItemCount))
		{
			const FVItemData* pData = GetItemData(Index);
			RECT rect = GetItemRect(Index);

			InflateRect(&rect, pData->RectInflate, pData->RectInflate);
			InvalidateRect(&rect);
		}
}

CMenu* CFileView::GetViewContextMenu()
{
	return NULL;
}

void CFileView::AppendSendToItem(CMenu* pMenu, UINT nID, LPCWSTR lpszNewItem, HICON hIcon, INT cx, INT cy)
{
	const UINT Index = nID % 0xFF;

	pMenu->AppendMenu(MF_STRING, nID, lpszNewItem);

	if (hIcon)
	{
		m_SendToItems[Index].hIcon = hIcon;
		m_SendToItems[Index].cx = cx;
		m_SendToItems[Index].cy = cy;

		MENUITEMINFO mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_BITMAP;
		mii.hbmpItem = HBMMENU_CALLBACK;

		SetMenuItemInfo(*pMenu, pMenu->GetMenuItemCount()-1, TRUE, &mii);
	}
}

CMenu* CFileView::GetSendToMenu()
{
	CMenu* pMenu = new CMenu();
	pMenu->CreatePopupMenu();

	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE;
	mi.dwStyle = MNS_CHECKORBMP;
	SetMenuInfo(*pMenu, &mi);

	const INT cx = GetSystemMetrics(SM_CXSMICON);
	const INT cy = GetSystemMetrics(SM_CYSMICON);

	BOOL Added = FALSE;

	// Stores
	UINT nID = FIRSTSENDTO;
	if (LFGetDefaultStore()==LFOk)
	{
		AppendSendToItem(pMenu, nID, CString((LPCSTR)IDS_DEFAULTSTORE), (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_APPLICATION), IMAGE_ICON, cx, cy, LR_SHARED), cx, cy);
		Added = TRUE;

		const UINT Index = (nID++) & 0xFF;

		m_SendToItems[Index].IsStore = TRUE;
		strcpy_s(m_SendToItems[Index].StoreID, LFKeySize, "");
	}

	if (LFGetStoreCount())
	{
		AppendSendToItem(pMenu, nID, CString((LPCSTR)IDS_CONTEXTMENU_CHOOSESTORE), NULL, cx, cy);
		Added = TRUE;

		const UINT Index = (nID++) & 0xFF;

		m_SendToItems[Index].IsStore = TRUE;
		strcpy_s(m_SendToItems[Index].StoreID, LFKeySize, "CHOOSE");
	}

	// SendTo shortcuts
	nID = FIRSTSENDTO+0x40;
	if (Added)
	{
		pMenu->AppendMenu(MF_SEPARATOR);
		Added = FALSE;
	}

	WCHAR Path[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL, Path, CSIDL_SENDTO, FALSE))
	{
		WCHAR Mask[MAX_PATH];
		wcscpy_s(Mask, MAX_PATH, Path);
		wcscat_s(Mask, MAX_PATH, L"\\*.*");

		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(Mask, &ffd);

		if (hFind!=INVALID_HANDLE_VALUE)
			do
			{
				if ((ffd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_DIRECTORY))==0)
				{
					WCHAR Name[MAX_PATH];
					wcscpy_s(Name, MAX_PATH, ffd.cFileName);

					WCHAR* pLastExt = wcsrchr(Name, L'.');
					if (pLastExt)
						if (*pLastExt!=L'\0')
						{
							CString Ext(pLastExt);
							Ext.MakeUpper();
							if ((Ext==_T(".DESKLINK")) || (Ext==_T(".LFSENDTO")))
								continue;

							*pLastExt = L'\0';
						}

					WCHAR Dst[MAX_PATH];
					wcscpy_s(Dst, MAX_PATH, Path);
					wcscat_s(Dst, MAX_PATH, L"\\");
					wcscat_s(Dst, MAX_PATH, ffd.cFileName);

					SHFILEINFO sfi;
					if (SHGetFileInfo(Dst, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON))
					{
						AppendSendToItem(pMenu, nID, Name, theApp.m_SystemImageListSmall.ExtractIcon(sfi.iIcon), cx, cy);
						Added = TRUE;

						const UINT Index = (nID++) & 0xFF;

						m_SendToItems[Index].IsStore = FALSE;
						wcscpy_s(m_SendToItems[Index].Path, MAX_PATH, Dst);
					}
				}
			}
			while (FindNextFile(hFind, &ffd));

		FindClose(hFind);
	}

	// Volumes
	DWORD VolumesOnSystem = LFGetLogicalVolumes(LFGLV_EXTERNAL | LFGLV_NETWORK | LFGLV_FLOPPIES);
	DWORD VolumesWOFloppies = LFGetLogicalVolumes(LFGLV_EXTERNAL | LFGLV_NETWORK);
	BOOL HideVolumesWithNoMedia = LFHideVolumesWithNoMedia();

	for (CHAR cVolume='A'; cVolume<='Z'; cVolume++, VolumesOnSystem>>=1, VolumesWOFloppies>>=1)
	{
		if ((VolumesOnSystem & 1)==0)
			continue;

		BOOL CheckEmpty = HideVolumesWithNoMedia && ((VolumesWOFloppies & 1)!=0);

		WCHAR szDriveRoot[] = L" :\\";
		szDriveRoot[0] = cVolume;
		SHFILEINFO sfi;
		if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON | (CheckEmpty ? SHGFI_ATTRIBUTES : 0)))
		{
			if ((!sfi.dwAttributes) && CheckEmpty)
				continue;

			if (Added)
			{
				pMenu->AppendMenu(MF_SEPARATOR);
				Added = FALSE;
			}

			AppendSendToItem(pMenu, nID, sfi.szDisplayName, sfi.hIcon, cx, cy);

			const UINT Index = (nID++) & 0xFF;

			m_SendToItems[Index].IsStore = FALSE;
			wcscpy_s(m_SendToItems[Index].Path, MAX_PATH, szDriveRoot);
		}
	}

	return pMenu;
}

CMenu* CFileView::GetItemContextMenu(INT Index)
{
	UINT InsertPos = 0;

	CMenu* pMenu = new CMenu();

	LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

	switch (pItemDescriptor->Type & LFTypeMask)
	{
	case LFTypeStore:
		pMenu->LoadMenu(IDM_STORE);
		pMenu->GetSubMenu(0)->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

		break;

	case LFTypeFolder:
		if ((pItemDescriptor->AggregateFirst!=-1) && (pItemDescriptor->AggregateLast!=-1))
			pMenu->LoadMenu(IDM_FOLDER);

		break;

	case LFTypeFile:
		if ((m_Context==LFContextArchive) || (m_Context==LFContextTrash))
		{
			pMenu->LoadMenu(IDM_FILE_AWAY);
			InsertPos = 2;
		}
		else
		{
			pMenu->LoadMenu(IDM_FILE);
		}

		break;
	}

	if (!IsMenu(*pMenu))
	{
		pMenu->CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)CreateMenu(), _T("POPUP"));
	}

	CMenu* pPopup = pMenu->GetSubMenu(0);
	ASSERT_VALID(pPopup);

	if (m_Context!=LFContextTrash)
		if (((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) || (((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder) && (pItemDescriptor->AggregateFirst!=-1) && (pItemDescriptor->AggregateLast!=-1)))
		{
			if (m_Context!=LFContextArchive)
				pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, m_Context==LFContextTasks ? IDM_FILE_TASKDONE : IDM_FILE_MAKETASK, CString((LPCSTR)(m_Context==LFContextTasks ? IDS_CONTEXTMENU_TASKDONE : IDS_CONTEXTMENU_MAKETASK)));

			pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, m_Context==LFContextClipboard ? IDM_FILE_REMOVEFROMCLIPBOARD : IDM_FILE_REMEMBER, CString((LPCSTR)(m_Context==LFContextClipboard ? IDS_CONTEXTMENU_REMOVEFROMCLIPBOARD : IDS_CONTEXTMENU_REMEMBER)));

			// SendTo
			CMenu* pSendPopup = GetSendToMenu();

			pPopup->InsertMenu(InsertPos, MF_POPUP | MF_BYPOSITION, (UINT_PTR)pSendPopup->m_hMenu, CString((LPCSTR)IDS_CONTEXTMENU_SENDTO));
			pPopup->InsertMenu(InsertPos, MF_SEPARATOR | MF_BYPOSITION);

			pSendPopup->Detach();
			delete pSendPopup;
		}

	switch (pItemDescriptor->Type & LFTypeMask)
	{
	case LFTypeStore:
		pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_ITEM_OPENFILEDROP, CString((LPCSTR)IDS_CONTEXTMENU_OPENFILEDROP));
		pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_ITEM_OPENNEWWINDOW, CString((LPCSTR)IDS_CONTEXTMENU_OPENNEWWINDOW));

		break;

	case LFTypeFile:
		if (pItemDescriptor->CoreAttributes.ContextID==LFContextFilters)
		{
			pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_FILE_EDIT, CString((LPCSTR)IDS_CONTEXTMENU_EDIT));
		}
		else
		{
			if (pItemDescriptor->CoreAttributes.URL[0]!='\0')
				pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_FILE_OPENBROWSER, CString((LPCSTR)IDS_CONTEXTMENU_OPENBROWSER));

			pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_FILE_OPENWITH, CString((LPCSTR)IDS_CONTEXTMENU_OPENWITH));
		}

		break;
	}

	pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_ITEM_OPEN, CString((LPCSTR)IDS_CONTEXTMENU_OPEN));
	pPopup->SetDefaultItem(InsertPos, TRUE);

	return pMenu;
}

void CFileView::GetPersistentData(FVPersistentData& Data, BOOL ForReload) const
{
	ZeroMemory(&Data, sizeof(Data));

	Data.FocusItem = m_FocusItem;
	Data.FocusItemSelected = !ForReload && p_CookedFiles && (m_FocusItem!=-1) && (m_FocusItem<(INT)p_CookedFiles->m_ItemCount) ? IsItemSelected(m_FocusItem) : FALSE;
	Data.HScrollPos = m_HScrollPos;
	Data.VScrollPos = m_VScrollPos;
}

void CFileView::EditLabel(INT Index)
{
	m_EditLabel = -1;

	if ((m_Flags & FF_ENABLELABELEDIT) && p_CookedFiles && (Index>=0) && (Index<(INT)p_CookedFiles->m_ItemCount) && (m_Context!=LFContextArchive) && (m_Context!=LFContextTrash))
	{
		LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

		if (((pItemDescriptor->Type & LFTypeMask)==LFTypeStore) ||
			((pItemDescriptor->Type & (LFTypeMask | LFTypeMounted))==(LFTypeFile | LFTypeMounted)))
		{
			m_EditLabel = Index;
			InvalidateItem(Index);
			EnsureVisible(Index);

			LFFont* pFont = GetLabelFont();
			const INT FontHeight = pFont->GetFontHeight();

			CRect rect(GetLabelRect(Index));
			if (rect.Height()>FontHeight+4)
			{
				rect.top += (rect.Height()-FontHeight-4)/2;
				rect.bottom = rect.top+FontHeight+4;
			}

			m_pWndEdit = new CEdit();
			m_pWndEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, this, 2);

			// DO NOT USE GetLabel(), as it is a beautified version of the file name for display purposes only!
			m_pWndEdit->SetWindowText(pItemDescriptor->CoreAttributes.FileName);

			m_pWndEdit->SetFont(pFont);
			m_pWndEdit->SetFocus();
			m_pWndEdit->SetSel(0, -1);
		}
	}
}

void CFileView::ResetScrollbars()
{
	if (m_Flags & FF_ENABLESCROLLING)
	{
		ScrollWindow(m_HScrollPos, m_VScrollPos);

		SetScrollPos(SB_VERT, m_VScrollPos=0);
		SetScrollPos(SB_HORZ, m_HScrollPos=0);
	}
}

void CFileView::AdjustScrollbars()
{
	if ((m_Flags & FF_ENABLESCROLLING)==0)
		return;

	// Dimensions
	CRect rect;
	GetWindowRect(rect);

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

	// Set vertical bars
	m_VScrollMax = max(0, m_ScrollHeight-rect.Height()+(INT)m_HeaderHeight);
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height()-m_HeaderHeight;
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	// Set horizontal bars
	m_HScrollMax = max(0, m_ScrollWidth-rect.Width());
	m_HScrollPos = min(m_HScrollPos, m_HScrollMax);

	si.nPage = rect.Width();
	si.nMax = m_ScrollWidth-1;
	si.nPos = m_HScrollPos;
	SetScrollInfo(SB_HORZ, &si);
}

CString CFileView::GetLabel(LFItemDescriptor* pItemDescriptor) const
{
	CString strLabel = pItemDescriptor->CoreAttributes.FileName;

	// Remove annotation
	LFVariantData Data1;
	LFGetAttributeVariantDataEx(pItemDescriptor, LFAttrApplication, Data1);

	if (!LFIsNullVariantData(Data1))
	{
		const INT Length = strLabel.GetLength();

		if (strLabel.GetAt(Length-1)==L')')
		{
			UINT Pos = strLabel.ReverseFind(L'(');

			if (Pos!=-1)
			{
				LFVariantData Data2;
				LFInitVariantData(Data2, LFAttrApplication);
				LFVariantDataFromString(Data2, strLabel.Mid(Pos+1, Length-Pos-2));

				if (LFCompareVariantData(Data1, Data2)==0)
					strLabel = strLabel.Left(Pos).TrimRight();
			}
		}
	}

	// Extension
	if ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile)
		if ((!m_HideFileExt || (pItemDescriptor->CoreAttributes.FileName[0]==L'\0')) && (pItemDescriptor->CoreAttributes.FileFormat[0]!='\0') && (strcmp(pItemDescriptor->CoreAttributes.FileFormat, "filter")!=0))
		{
			strLabel += _T(".");
			strLabel += pItemDescriptor->CoreAttributes.FileFormat;
		}

	return strLabel;
}

BOOL CFileView::BeginDragDrop()
{
	m_BeginDragDrop = FALSE;

	if (HasItemsSelected())
		GetParent()->SendMessage(WM_BEGINDRAGDROP);

	return TRUE;
}

void CFileView::DestroyEdit(BOOL Accept)
{
	if (m_pWndEdit)
	{
		INT Item = m_EditLabel;

		CString Name;
		m_pWndEdit->GetWindowText(Name);

		// Destroying the edit control will release its focus, in turn causing DestroyEdit()
		// to be called. To prevent an infinite recursion, we set m_pWndEdit to NULL first.
		CEdit* pVictim = m_pWndEdit;
		m_pWndEdit = NULL;

		pVictim->DestroyWindow();
		delete pVictim;

		if (Accept && (!Name.IsEmpty()) && (Item!=-1))
			GetParent()->SendMessage(WM_RENAMEITEM, (WPARAM)Item, (LPARAM)(LPCWSTR)Name);
	}

	m_EditLabel = -1;
	m_TypingBuffer[0] = L'\0';
}

void CFileView::ScrollWindow(INT dx, INT dy, LPCRECT /*lpRect*/, LPCRECT /*lpClipRect*/)
{
	ASSERT(m_Flags & FF_ENABLESCROLLING);

	CRect rect;
	GetClientRect(rect);

	rect.top = m_HeaderHeight;

	if (IsCtrlThemed() && (dy!=0))
	{
		rect.bottom -= BACKSTAGERADIUS;

		ScrollWindowEx(dx, dy, rect, rect, NULL, NULL, SW_INVALIDATE);
		RedrawWindow(CRect(rect.left, rect.bottom, rect.right, rect.bottom+BACKSTAGERADIUS), NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else
	{
		ScrollWindowEx(dx, dy, rect, rect, NULL, NULL, SW_INVALIDATE);
	}
}

void CFileView::DrawItemBackground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached)
{
	const BOOL Selected = IsItemSelected(Index);

	if (Cached && Themed && Selected)
	{
		CDC MemDC;
		MemDC.CreateCompatibleDC(&dc);

		HBITMAP hOldBitmap;

		const INT Width = rectItem->right-rectItem->left;
		const INT Height = rectItem->bottom-rectItem->top;

		if ((m_Bitmaps[BM_SELECTED].Width!=Width) || (m_Bitmaps[BM_SELECTED].Height!=Height))
		{
			DeleteObject(m_Bitmaps[BM_SELECTED].hBitmap);

			m_Bitmaps[BM_SELECTED].Width = Width;
			m_Bitmaps[BM_SELECTED].Height = Height;
			m_Bitmaps[BM_SELECTED].hBitmap = CreateCompatibleBitmap(dc, Width, Height);

			hOldBitmap = (HBITMAP)MemDC.SelectObject(m_Bitmaps[BM_SELECTED].hBitmap);

			MemDC.FillSolidRect(0, 0, Width, Height, 0xFFFFFF);
			DrawListItemBackground(MemDC, CRect(0, 0, Width, Height), Themed, GetFocus()==this, m_HotItem==Index, m_FocusItem==Index, Selected);
		}
		else
		{
			hOldBitmap = (HBITMAP)MemDC.SelectObject(m_Bitmaps[BM_SELECTED].hBitmap);
		}

		dc.BitBlt(rectItem->left, rectItem->top, Width, Height, &MemDC, 0, 0, SRCCOPY);

		MemDC.SelectObject(hOldBitmap);

		dc.SetTextColor(((*p_CookedFiles)[Index]->CoreAttributes.Flags & LFFlagMissing) ? 0x0000FF : 0xFFFFFF);
	}
	else
	{
		DrawListItemBackground(dc, rectItem, Themed, GetFocus()==this,
			m_HotItem==Index, m_FocusItem==Index, Selected,
			((*p_CookedFiles)[Index]->CoreAttributes.Flags & LFFlagMissing) ? 0x0000FF : (COLORREF)-1,
			m_ShowFocusRect);
	}
}

void CFileView::DrawItemForeground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached)
{
	const BOOL Selected = IsItemSelected(Index);

	if (((m_HotItem!=Index) && !Selected) || !Themed)
		return;

	if (Cached)
	{
		CDC MemDC;
		MemDC.CreateCompatibleDC(&dc);

		HBITMAP hOldBitmap;

		const INT Width = rectItem->right-rectItem->left;
		const INT Height = rectItem->bottom-rectItem->top;

		if ((m_Bitmaps[BM_REFLECTION].Width!=Width) || (m_Bitmaps[BM_REFLECTION].Height!=Height))
		{
			DeleteObject(m_Bitmaps[BM_REFLECTION].hBitmap);

			m_Bitmaps[BM_REFLECTION].Width = Width;
			m_Bitmaps[BM_REFLECTION].Height = Height;
			m_Bitmaps[BM_REFLECTION].hBitmap = CreateTransparentBitmap(Width, Height);

			hOldBitmap = (HBITMAP)MemDC.SelectObject(m_Bitmaps[BM_REFLECTION].hBitmap);

			DrawListItemForeground(MemDC, CRect(0, 0, Width, Height), Themed, GetFocus()==this, m_HotItem==Index, m_FocusItem==Index, Selected);
		}
		else
		{
			hOldBitmap = (HBITMAP)MemDC.SelectObject(m_Bitmaps[BM_REFLECTION].hBitmap);
		}

		AlphaBlend(dc, rectItem->left, rectItem->top, Width, Height, MemDC, 0, 0, Width, Height, BF);

		MemDC.SelectObject(hOldBitmap);
	}
	else
	{
		DrawListItemForeground(dc, rectItem, Themed, GetFocus()==this, m_HotItem==Index, m_FocusItem==Index, Selected);
	}
}

void CFileView::DrawJumboIcon(CDC& dc, Graphics& g, CPoint pt, LFItemDescriptor* pItemDescriptor, INT ThumbnailYOffset) const
{
	theApp.m_IconFactory.DrawJumboIcon(dc, g, pt, pItemDescriptor, theApp.m_Attributes[m_ContextViewSettings.SortBy].AttrProperties.ShowRepresentativeThumbnail ? p_RawFiles : NULL, TRUE, ThumbnailYOffset);
}

BOOL CFileView::DrawNothing(CDC& dc, LPCRECT lpRectClient, BOOL Themed) const
{
	if (m_Nothing)
	{
		CRect rectText(lpRectClient);
		rectText.top += m_HeaderHeight+BACKSTAGEBORDER;

		dc.SetTextColor(Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DSHADOW));
		dc.DrawText(CString((LPCSTR)IDS_NOTHINGTODISPLAY), rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

		return TRUE;
	}

	ASSERT(p_CookedFiles);

	return FALSE;
}

UINT CFileView::GetColorDotCount(const LFItemDescriptor* pItemDescriptor)
{
	ASSERT(pItemDescriptor);

	switch (pItemDescriptor->Type & LFTypeMask)
	{
#ifdef _DEBUG
	case LFTypeStore:
		ASSERT(pItemDescriptor->CoreAttributes.Color==0);
		ASSERT(pItemDescriptor->AggregateColorSet<2);
		break;
#endif

	case LFTypeFile:
		return pItemDescriptor->CoreAttributes.Color ? 1 : 0;

	case LFTypeFolder:
		UINT Count = 0;

		for (UINT a=1; a<LFItemColorCount; a++)
			if (pItemDescriptor->AggregateColorSet & (1 << a))
				Count++;

		return Count;
	}

	return 0;
}

INT CFileView::GetColorDotWidth(const LFItemDescriptor* pItemDescriptor, const CIcons& Icons) const
{
	const UINT Count = GetColorDotCount(pItemDescriptor);
	const INT Size = Icons.GetIconSize();

	return Count*(4*Size/3)-((Count>1) ? (Count-1)*(5*Size/6) : 0);
}

void CFileView::DrawColorDots(CDC& dc, CRect& rect, const LFItemDescriptor* pItemDescriptor, INT FontHeight, CIcons& Icons) const
{
	ASSERT(pItemDescriptor);

	BOOL First = TRUE;

	switch (pItemDescriptor->Type & LFTypeMask)
	{
	case LFTypeFolder:
		for (BYTE a=1; a<LFItemColorCount; a++)
			if (pItemDescriptor->AggregateColorSet & (1 << a))
				DrawColorDot(dc, rect, a, First, Icons, FontHeight);

		break;

	case LFTypeFile:
		ASSERT(pItemDescriptor->AggregateColorSet==(1 << pItemDescriptor->CoreAttributes.Color));

		if (pItemDescriptor->CoreAttributes.Color)
			DrawColorDot(dc, rect, pItemDescriptor->CoreAttributes.Color, First, Icons, FontHeight);

		break;
	}
}

void CFileView::UnselectAllAfterTransaction()
{
	if (p_CookedFiles)
		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			SelectItem(a, FALSE, TRUE);
}


BEGIN_MESSAGE_MAP(CFileView, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHWHEEL()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SETCURSOR()

	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_CONTEXTMENU()

	ON_MESSAGE_VOID(WM_SELECTALL, OnSelectAll)
	ON_MESSAGE_VOID(WM_SELECTNONE, OnSelectNone)
	ON_COMMAND(IDM_SELECTALL, OnSelectAll)
	ON_COMMAND(IDM_SELECTNONE, OnSelectNone)
	ON_COMMAND(IDM_SELECTINVERT, OnSelectInvert)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_SELECTALL, IDM_SELECTINVERT, OnUpdateCommands)

	ON_EN_KILLFOCUS(2, OnDestroyEdit)
END_MESSAGE_MAP()

INT CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Font height
	m_LargeFontHeight = theApp.m_LargeFont.GetFontHeight();
	m_DefaultFontHeight = m_RowHeight = theApp.m_DefaultFont.GetFontHeight();
	m_SmallFontHeight = theApp.m_SmallFont.GetFontHeight();

	// Color dots
	theApp.LoadColorDots(m_LargeColorDots, m_LargeFontHeight);
	theApp.LoadColorDots(m_DefaultColorDots, m_DefaultFontHeight);

	if (m_Flags & FF_ENABLESCROLLING)
		ResetScrollbars();

	return 0;
}

void CFileView::OnDestroy()
{
	DeleteObject(m_Bitmaps[BM_SELECTED].hBitmap);
	DeleteObject(m_Bitmaps[BM_REFLECTION].hBitmap);

	CWnd::OnDestroy();
}

BOOL CFileView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CFileView::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CFileView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rect;
	GetClientRect(rect);

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
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);

		if (m_pWndEdit)
		{
			CRect rect;
			m_pWndEdit->GetWindowRect(rect);
			ScreenToClient(rect);

			rect.OffsetRect(0, -nInc);
			m_pWndEdit->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	CFrontstageWnd::OnVScroll(nSBCode, nPos, pScrollBar);
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
		nInc = -HORIZONTALSCROLLWIDTH;
		break;

	case SB_PAGEDOWN:
	case SB_LINEDOWN:
		nInc = HORIZONTALSCROLLWIDTH;
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
		ScrollWindow(-nInc, 0);
		SetScrollPos(SB_HORZ, m_HScrollPos);

		UpdateWindow();
	}

	CFrontstageWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFileView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_BeginDragDrop && (nFlags & MK_LBUTTON))
		if ((abs(point.x-m_DragPos.x)>=GetSystemMetrics(SM_CXDRAG)) || (abs(point.y-m_DragPos.y)>=GetSystemMetrics(SM_CYDRAG)))
			if (BeginDragDrop())
				return;

	if ((m_Flags & (FF_ENABLEHOVER | FF_ENABLETOOLTIPS))==(FF_ENABLEHOVER | FF_ENABLETOOLTIPS))
	{
		const INT Index = ItemAtPosition(point);

		if (!m_Hover)
		{
			m_Hover = TRUE;

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE | TME_HOVER;
			tme.dwHoverTime = HOVERTIME;
			tme.hwndTrack = m_hWnd;
			TrackMouseEvent(&tme);
		}
		else
			if ((theApp.IsTooltipVisible()) && (Index!=m_HotItem))
				theApp.HideTooltip();

		if (m_HotItem!=Index)
		{
			if (m_Flags & FF_ENABLEHOVER)
				InvalidateItem(m_HotItem);

			m_HotItem = Index;

			if (m_Flags & FF_ENABLEHOVER)
				InvalidateItem(m_HotItem);
		}
	}
}

void CFileView::OnMouseLeave()
{
	InvalidateItem(m_HotItem);
	theApp.HideTooltip();

	m_Hover = FALSE;
	m_HotItem = -1;
}

void CFileView::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if ((m_HotItem!=-1) && !IsEditing())
			if (m_HotItem==m_EditLabel)
			{
				theApp.HideTooltip();
				EditLabel(m_EditLabel);
			}
			else
				if (!theApp.IsTooltipVisible() && (m_Flags & FF_ENABLETOOLTIPS))
				{
					LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[m_HotItem];

					HBITMAP hBitmap = (m_Flags & FF_ENABLETOOLTIPICONS) ? theApp.m_IconFactory.GetJumboIconBitmap(pItemDescriptor, theApp.m_Attributes[m_ContextViewSettings.SortBy].AttrProperties.ShowRepresentativeThumbnail ? p_RawFiles : NULL) : NULL;

					theApp.ShowTooltip(this, point, GetLabel(pItemDescriptor),
						theApp.GetHintForItem(pItemDescriptor, theApp.m_IconFactory.GetTypeName(pItemDescriptor->CoreAttributes.FileFormat)),
						NULL, hBitmap);
				}
	}
	else
	{
		theApp.HideTooltip();
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = HOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

BOOL CFileView::OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(rect);

	if (!rect.PtInRect(pt))
		return FALSE;

	INT nScrollLines;
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
	if (nScrollLines<1)
		nScrollLines = 1;

	INT nInc = max(-m_VScrollPos, min(-zDelta*(INT)m_RowHeight*nScrollLines/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}

void CFileView::OnMouseHWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(rect);

	if (!rect.PtInRect(pt))
		return;

	INT nInc = max(-m_HScrollPos, min(zDelta*64/WHEEL_DELTA, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_HScrollPos += nInc;
		ScrollWindow(-nInc, 0);
		SetScrollPos(SB_HORZ, m_HScrollPos);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}
}

void CFileView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((GetKeyState(VK_CONTROL)>=0) && (nChar>=(m_TypingBuffer[0] ? 32u : 33u)) && (m_EditLabel==-1))
	{
		if (p_CookedFiles)
			if (p_CookedFiles->m_ItemCount>1)
			{
				// Reset typing buffer?
				DWORD Ticks = GetTickCount();
				if (Ticks-m_TypingTicks>=1000)
					m_TypingBuffer[0] = L'\0';

				m_TypingTicks = Ticks;

				// Concatenate typing buffer
				WCHAR TypingBuffer[256];
				wcsncpy_s(TypingBuffer, 256, m_TypingBuffer, 254);

				WCHAR Letter[2] = { (WCHAR)nChar, L'\0' };
				wcscat_s(TypingBuffer, 256, Letter);

				INT FocusItem = max(m_FocusItem, 0);

				for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
				{
					if (_wcsnicmp(TypingBuffer, (*p_CookedFiles)[FocusItem]->CoreAttributes.FileName, wcslen(TypingBuffer))==0)
					{
						wcscpy_s(m_TypingBuffer, 256, TypingBuffer);
						SetFocusItem(FocusItem, FALSE);

						return;
					}

					if (++FocusItem>=(INT)p_CookedFiles->m_ItemCount)
						FocusItem = 0;
				}
			}

		theApp.PlayDefaultSound();
	}

	CFrontstageWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case 'A':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnSelectAll();

		break;

	case 'C':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, IDM_FILE_COPY);

		break;

	case 'I':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnSelectInvert();

		break;

	case 'N':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnSelectNone();

		break;

	case VK_SPACE:
		if ((m_FocusItem!=-1) && !m_TypingBuffer[0])
			SelectItem(m_FocusItem, (GetKeyState(VK_CONTROL)>=0) ? TRUE : !IsItemSelected(m_FocusItem));

		break;

	case VK_F2:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0) && (m_FocusItem!=-1))
			if (IsItemSelected(m_FocusItem))
				EditLabel(m_FocusItem);

		break;

	case VK_F5:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, ID_NAV_RELOAD);

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
		CFrontstageWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CFileView::OnLButtonDown(UINT nFlags, CPoint point)
{
	DestroyEdit();

	const INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		if (!m_BeginDragDrop)
		{
			m_BeginDragDrop = TRUE;
			m_DragPos = point;
		}

		if ((nFlags & MK_CONTROL) && m_AllowMultiSelect)
		{
			InvalidateItem(m_FocusItem);
			m_FocusItem = Index;
			SelectItem(Index, !IsItemSelected(Index));
		}
		else
			if ((m_FocusItem==Index) && IsItemSelected(Index))
			{
				m_EditLabel = Index;
			}
			else
			{
				SetFocusItem(Index, nFlags & MK_SHIFT, !IsItemSelected(Index));
			}
	}

	if (GetFocus()!=this)
		SetFocus();
}

void CFileView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!(nFlags & MK_CONTROL) || !m_AllowMultiSelect)
		if (ItemAtPosition(point)==-1)
			OnSelectNone();

	m_BeginDragDrop = FALSE;
}

void CFileView::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	DestroyEdit();

	const INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		SetFocusItem(Index, FALSE);
		GetOwner()->SendMessage(WM_COMMAND, IDM_ITEM_OPEN);
	}
}

void CFileView::OnRButtonDown(UINT nFlags, CPoint point)
{
	const INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		if (!(nFlags & (MK_SHIFT | MK_CONTROL)) || !m_AllowMultiSelect)
			if (!IsItemSelected(Index))
			{
				m_FocusItem = Index;

				for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
					SelectItem(a, a==Index, TRUE);

				ChangedItems();
			}
			else
				if (m_FocusItem!=Index)
				{
					InvalidateItem(m_FocusItem);
					InvalidateItem(m_FocusItem=Index);
				}
	}
	else
	{
		if (GetFocus()!=this)
			SetFocus();
	}

	m_BeginDragDrop = FALSE;
}

void CFileView::OnRButtonUp(UINT nFlags, CPoint point)
{
	const INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		if (GetFocus()!=this)
			SetFocus();

		if (!IsItemSelected(Index))
		{
			m_FocusItem = Index;

			for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
				SelectItem(a, a==Index, TRUE);

			ChangedItems();
		}
	}
	else
	{
		if (!(nFlags & MK_CONTROL) || !m_AllowMultiSelect)
			OnSelectNone();
	}

	GetParent()->UpdateWindow();

	CFrontstageWnd::OnRButtonUp(nFlags, point);
}

void CFileView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CFileView::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}

BOOL CFileView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	SetCursor(theApp.LoadStandardCursor(p_CookedFiles ? IDC_ARROW : IDC_WAIT));

	return TRUE;
}


// Context menu

void CFileView::OnMeasureItem(INT nIDCtl, LPMEASUREITEMSTRUCT lpmis)
{
	if ((lpmis==NULL) || (lpmis->CtlType!=ODT_MENU))
	{
		CFrontstageWnd::OnMeasureItem(nIDCtl, lpmis);

		return;
	}

	if ((lpmis->itemID>=FIRSTSENDTO) && (lpmis->itemID<=FIRSTSENDTO+0xFF))
	{
		lpmis->itemWidth = GetSystemMetrics(SM_CXSMICON)*5/4;
		lpmis->itemHeight = GetSystemMetrics(SM_CYSMICON);
	}
}

void CFileView::OnDrawItem(INT nIDCtl, LPDRAWITEMSTRUCT lpdis)
{
	if ((lpdis==NULL) || (lpdis->CtlType!=ODT_MENU))
	{
		CFrontstageWnd::OnDrawItem(nIDCtl, lpdis);

		return;
	}

	if ((lpdis->itemID>=FIRSTSENDTO) && (lpdis->itemID<=FIRSTSENDTO+0xFF))
	{
		const UINT Index = lpdis->itemID % 0xFF;

		DrawIconEx(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, m_SendToItems[Index].hIcon, m_SendToItems[Index].cx, m_SendToItems[Index].cy, 0, NULL, DI_NORMAL);
	}
}

void CFileView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	INT Index = -1;
	if ((point.x<0) || (point.y<0))
	{
		Index = GetSelectedItem();

		if (Index!=-1)
		{
			FVItemData* pData = GetItemData(Index);

			point.x = pData->Rect.left-m_HScrollPos;
			point.y = pData->Rect.bottom-m_VScrollPos+(INT)m_HeaderHeight+1;
			ClientToScreen(&point);
		}
	}
	else
	{
		CPoint pt(point);
		ScreenToClient(&pt);

		Index = ItemAtPosition(pt);
	}

	if (Index==-1)
	{
		GetParent()->SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(point.x, point.y));
	}
	else
	{
		ZeroMemory(&m_SendToItems, sizeof(m_SendToItems));

		UINT idCmd = 0;

		CMenu* pMenu = GetItemContextMenu(Index);
		if (pMenu)
		{
			CMenu* pPopup = pMenu->GetSubMenu(0);
			ASSERT_VALID(pPopup);

			idCmd = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, GetOwner(), NULL);

			delete pMenu;
		}

		for (UINT a=0; a<256; a++)
			DestroyIcon(m_SendToItems[a].hIcon);

		if (idCmd)
			if (idCmd<FIRSTSENDTO)
			{
				GetOwner()->SendMessage(WM_COMMAND, (WPARAM)idCmd);
			}
			else
			{
				GetParent()->SendMessage(WM_SENDTO, (WPARAM)&m_SendToItems[idCmd % 0xFF]);
			}
	}
}


// Menu commands

void CFileView::OnSelectAll()
{
	if (p_CookedFiles && m_AllowMultiSelect)
	{
		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			SelectItem(a, TRUE, TRUE);

		ChangedItems();
		RedrawWindow();
	}
}

void CFileView::OnSelectNone()
{
	if (p_CookedFiles)
	{
		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			SelectItem(a, FALSE, TRUE);

		ChangedItems();
		RedrawWindow();
	}
}

void CFileView::OnSelectInvert()
{
	if (p_CookedFiles && m_AllowMultiSelect)
	{
		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			SelectItem(a, !IsItemSelected(a), TRUE);

		ChangedItems();
		RedrawWindow();
	}
}

void CFileView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_SELECTALL:
	case IDM_SELECTINVERT:
		bEnable &= m_AllowMultiSelect;

	case IDM_SELECTNONE:
		bEnable &= p_CookedFiles ? (p_CookedFiles->m_ItemCount!=NULL) : FALSE;
		break;
	}

	pCmdUI->Enable(bEnable);
}


// Edit

void CFileView::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}

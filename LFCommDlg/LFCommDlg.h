
#pragma once
#include "CBackstageBar.h"
#include "CBackstageEdit.h"
#include "CBackstageShadow.h"
#include "CBackstageSidebar.h"
#include "CBackstageWidgets.h"
#include "CBackstageWnd.h"
#include "CCategory.h"
#include "CDesktopDimmer.h"
#include "CExplorerList.h"
#include "CExplorerNotification.h"
#include "CExplorerTree.h"
#include "CFrontstagePane.h"
#include "CFrontstageWnd.h"
#include "CGenreList.h"
#include "CHeaderArea.h"
#include "CHeaderButton.h"
#include "CHoverButton.h"
#include "CIconCtrl.h"
#include "CIcons.h"
#include "CInspectorGrid.h"
#include "CItemPanel.h"
#include "CPropertyEdit.h"
#include "CTaskbar.h"
#include "CTaskButton.h"
#include "CTooltipHeader.h"
#include "GLFont.h"
#include "GLRenderer.h"
#include "LFAboutDlg.h"
#include "LFAddStoreDlg.h"
#include "LFApplication.h"
#include "LFAttributeListDlg.h"
#include "LFBrowseForFolderDlg.h"
#include "LFChooseStoreDlg.h"
#include "LFDropboxDlg.h"
#include "LFDropSource.h"
#include "LFDropTarget.h"
#include "LFEditFilterDlg.h"
#include "LFEditGenreDlg.h"
#include "LFFont.h"
#include "LFGotoYearDlg.h"
#include "LFICloudDlg.h"
#include "LFItemTemplateDlg.h"
#include "LFLicenseDlg.h"
#include "LFMessageBoxDlg.h"
#include "LFOneDriveDlg.h"
#include "LFProgressDlg.h"
#include "LFSelectLocationGPSDlg.h"
#include "LFSelectLocationIATADlg.h"
#include "LFStoreDataObject.h"
#include "LFStoreMaintenanceDlg.h"
#include "LFStorePropertiesDlg.h"
#include "LFTabbedDialog.h"
#include "LFTransactionDataObject.h"
#include "LFUpdateDlg.h"
#include "Workers.h"

#define LFGetApp() ((LFApplication*)AfxGetApp())

#define MB_ICONREADY      0x00000050L
#define MB_ICONSHIELD     0x00000060L

#define LFCATEGORYPADDING     2

#define REQUEST_TEXTCOLOR                1
#define REQUEST_TOOLTIP_DATA             2
#define REQUEST_DRAWBUTTONFOREGROUND     3

#define COLORREF2RGB(clr)             (0xFF000000 | ((clr & 0xFF)<<16) | (clr & 0xFF00) | (clr>>16))
#define COLORREF2ARGB(clr, alpha)     ((alpha<<24) | ((clr & 0xFF)<<16) | (clr & 0xFF00) | (clr>>16))

struct NM_TEXTCOLOR
{
	NMHDR hdr;
	INT Item;
	COLORREF Color;
};

struct NM_TOOLTIPDATA
{
	NMHDR hdr;
	INT Item;
	WCHAR Caption[256];
	WCHAR Hint[4096];
	HICON hIcon;
	HBITMAP hBitmap;
};

struct NM_DRAWBUTTONFOREGROUND
{
	NMHDR hdr;
	LPDRAWITEMSTRUCT lpDrawItemStruct;
	CDC* pDC;
};

struct PROGRESSDATA
{
	ULONGLONG ullCompleted;
	ULONGLONG ullTotal;
	TBPFLAG tbpFlags;
};

extern BLENDFUNCTION BF;

BOOL DuplicateGlobalMemory(const HGLOBAL hSrc, HGLOBAL& hDst);

INT GetAttributeIconIndex(UINT Attr);
void TooltipDataFromPIDL(LPITEMIDLIST pidl, CImageList* pIcons, HICON& hIcon, CString& Caption, CString& Hint);
void CreateRoundRectangle(LPCRECT lpRect, INT Radius, GraphicsPath& Path);
void CreateRoundTop(LPCRECT lpRect, INT Radius, GraphicsPath& Path);
void CreateReflectionRectangle(LPCRECT lpRect, INT Radius, GraphicsPath& Path);
BOOL IsCtrlThemed();
HBITMAP CreateTransparentBitmap(LONG Width, LONG Height);
HBITMAP CreateTruecolorBitmap(LONG Width, LONG Height);
CBitmap* CreateTruecolorBitmapObject(LONG Width, LONG Height);
void DrawLocationIndicator(Graphics& g, INT x, INT y, INT sz=16);
void DrawControlBorder(CWnd* pWnd);
void DrawCategory(CDC& dc, CRect rect, LPCWSTR Caption, LPCWSTR Hint, BOOL Themed);
void DrawListItemBackground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected, COLORREF TextColor=(COLORREF)-1, BOOL ShowFocusRect=TRUE);
void DrawListItemForeground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected);
void DrawSubitemBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Selected, BOOL Hover, BOOL ClipHorizontal=FALSE);
void DrawBackstageBorder(Graphics& g, CRect rect);
void DrawBackstageSelection(CDC& dc, Graphics& g, const CRect& rect, BOOL Selected, BOOL Enabled, BOOL Themed);
void DrawBackstageButtonBackground(CDC& dc, Graphics& g, CRect rect, BOOL Hover, BOOL Pressed, BOOL Enabled, BOOL Themed, BOOL Red=FALSE);
void DrawLightButtonBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover);
void DrawWhiteButtonBorder(Graphics& g, LPCRECT lpRect, BOOL IncludeBottom=TRUE);
void DrawWhiteButtonBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover, BOOL Disabled=FALSE, BOOL DrawBorder=FALSE);
void DrawWhiteButtonForeground(CDC& dc, LPDRAWITEMSTRUCT lpDrawItemStruct, BOOL ShowKeyboardCues=FALSE);


// liquidFOLDERS

void SetCompareComboBox(CComboBox* pComboBox, UINT Attr, INT Request=-1);

void AppendAttribute(CString& Str, UINT Attr, LPCWSTR Value);
void AppendAttribute(CString& Str, LFItemDescriptor* pItemDescriptor, UINT Attr);
CString GetHintForItem(LFItemDescriptor* pItemDescriptor, LPCWSTR pFormatName=NULL);
CString GetHintForStore(LFStoreDescriptor* pStoreDescriptor);


// IATA database

HBITMAP LFIATACreateAirportMap(LFAirport* pAirport, UINT Width, UINT Height);


// Update

void GetFileVersion(HMODULE hModule, CString& Version, CString* Copyright=NULL);
void LFCheckForUpdate(BOOL Force=FALSE, CWnd* pParentWnd=NULL);


// MessageBox

INT LFMessageBox(CWnd* pParentWnd, const CString& Text, const CString& Caption, UINT Type);
void LFErrorBox(CWnd* pParentWnd, UINT Result);
BOOL LFNagScreen(CWnd* pParentWnd=NULL);


// Progress

inline LRESULT LFSetTaskbarProgress(CWnd* pWnd, ULONGLONG ullCompleted, ULONGLONG ullTotal, TBPFLAG tbpFlags=TBPF_NORMAL)
{
	ASSERT(pWnd);

	PROGRESSDATA pd = { ullCompleted, ullTotal, tbpFlags };

	return pWnd->GetTopLevelParent()->SendMessage(LFGetApp()->m_SetProgressMsg, (WPARAM)pWnd->GetSafeHwnd(), (LPARAM)&pd);
}

inline LRESULT LFHideTaskbarProgress(CWnd* pWnd)
{
	ASSERT(pWnd);

	return LFSetTaskbarProgress(pWnd, 0, 0, TBPF_NOPROGRESS);
}

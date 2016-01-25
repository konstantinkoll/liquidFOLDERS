
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
#include "CHeaderArea.h"
#include "CHeaderButton.h"
#include "CHoverButton.h"
#include "CIconCtrl.h"
#include "CIcons.h"
#include "CInspectorGrid.h"
#include "CPropertyEdit.h"
#include "CStorePanel.h"
#include "CTaskbar.h"
#include "CTaskButton.h"
#include "CTooltipHeader.h"
#include "CWhiteButton.h"
#include "GLFont.h"
#include "GLRenderer.h"
#include "LFAboutDlg.h"
#include "LFAddStoreDlg.h"
#include "LFApplication.h"
#include "LFAttributeListDlg.h"
#include "LFBrowseForFolderDlg.h"
#include "LFChooseStoreDlg.h"
#include "LFDropSource.h"
#include "LFDropTarget.h"
#include "LFEditFilterDlg.h"
#include "LFFont.h"
#include "LFGotoYearDlg.h"
#include "LFItemTemplateDlg.h"
#include "LFLicenseDlg.h"
#include "LFMessageBoxDlg.h"
#include "LFProgressDlg.h"
#include "LFSelectLocationGPSDlg.h"
#include "LFSelectLocationIATADlg.h"
#include "LFStoreDataObject.h"
#include "LFStoreMaintenanceDlg.h"
#include "LFStorePropertiesDlg.h"
#include "LFTransactionDataObject.h"
#include "LFUpdateDlg.h"
#include "Workers.h"

#define LFGetApp() ((LFApplication*)AfxGetApp())

#define MB_ICONREADY      0x00000050L
#define MB_ICONSHIELD     0x00000060L

#define LFCategoryPadding     2

extern BLENDFUNCTION BF;

BOOL DuplicateGlobalMemory(const HGLOBAL hSrc, HGLOBAL& hDst);

INT GetAttributeIconIndex(UINT Attr);
void TooltipDataFromPIDL(LPITEMIDLIST pidl, CImageList* pIcons, HICON& hIcon, CString& Caption, CString& Hint);
void CreateRoundRectangle(LPCRECT lpRect, INT Radius, GraphicsPath& Path);
void CreateRoundTop(LPCRECT lpRect, INT Radius, GraphicsPath& Path);
void CreateReflectionRectangle(LPCRECT lpRect, INT Radius, GraphicsPath& Path);
BOOL IsCtrlThemed();
HBITMAP CreateTransparentBitmap(LONG Width, LONG Height);
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
void DrawWhiteButtonForeground(CDC& dc, LPDRAWITEMSTRUCT lpDrawItemStruct, BOOL Selected, BOOL ShowKeyboardCues=FALSE);

void SetCompareComboBox(CComboBox* pComboBox, UINT Attr, INT Request=-1);

void AppendTooltipString(UINT Attr, CString& Str, WCHAR* tmpStr);
void AppendTooltipAttribute(LFItemDescriptor* pItemDescriptor, UINT Attr, CString& Str);
void GetHintForStore(LFItemDescriptor* pItemDescriptor, CString& Str);

HBITMAP LFIATACreateAirportMap(LFAirport* pAirport, UINT Width, UINT Height);

void GetFileVersion(HMODULE hModule, CString& Version, CString* Copyright=NULL);
void LFCheckForUpdate(BOOL Force=FALSE, CWnd* pParentWnd=NULL);

INT LFMessageBox(CWnd* pParentWnd, const CString& Text, const CString& Caption, UINT Type);
void LFErrorBox(CWnd* pParentWnd, UINT Result);
BOOL LFNagScreen(CWnd* pParentWnd=NULL);

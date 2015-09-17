
#pragma once
#include "CCategory.h"
#include "CDesktopDimmer.h"
#include "CExplorerList.h"
#include "CExplorerNotification.h"
#include "CExplorerTree.h"
#include "CGdiPlusBitmap.h"
#include "CGlassEdit.h"
#include "CGlassPane.h"
#include "CGlassWindow.h"
#include "CHeaderArea.h"
#include "CHeaderButton.h"
#include "CHoverButton.h"
#include "CIconCtrl.h"
#include "CIcons.h"
#include "CImageListTransparent.h"
#include "CInspectorGrid.h"
#include "CPropertyEdit.h"
#include "CSidebar.h"
#include "CStorePanel.h"
#include "CTagList.h"
#include "CTaskbar.h"
#include "CTaskButton.h"
#include "CTooltipHeader.h"
#include "LFAboutDlg.h"
#include "LFAddStoreDlg.h"
#include "LFApplication.h"
#include "LFAttributeListDlg.h"
#include "LFBrowseForFolderDlg.h"
#include "LFChooseStoreDlg.h"
#include "LFDropSource.h"
#include "LFDropTarget.h"
#include "LFEditFilterDlg.h"
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
void CreateRoundRectangle(LPRECT pRect, INT Radius, GraphicsPath& Path);
void CreateReflectionRectangle(LPRECT pRect, INT Radius, GraphicsPath& Path);
BOOL IsCtrlThemed();
HBITMAP CreateTransparentBitmap(LONG Width, LONG Height);
void DrawControlBorder(CWnd* pWnd);
void DrawCategory(CDC& dc, CRect rect, WCHAR* Caption, WCHAR* Hint, BOOL Themed);
void DrawListItemBackground(CDC& dc, LPRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected, COLORREF TextColor=(COLORREF)-1, BOOL ShowFocusRect=TRUE);
void DrawListItemForeground(CDC& dc, LPRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected);
void DrawSubitemBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Selected, BOOL Hover, BOOL ClipHorizontal=FALSE);
void DrawLightButtonBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover);
void DrawWhiteButtonBorder(Graphics& g, CRect rect, BOOL IncludeBottom=TRUE);
void DrawWhiteButtonBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover, BOOL Disabled=FALSE, BOOL DrawBorder=FALSE);

void SetCompareComboBox(CComboBox* pComboBox, UINT Attr, INT Request=-1);

void AppendTooltipString(UINT Attr, CString& Str, WCHAR* tmpStr);
void AppendTooltipAttribute(LFItemDescriptor* i, UINT Attr, CString& Str);
void GetHintForStore(LFItemDescriptor* i, CString& Str);

HBITMAP LFIATACreateAirportMap(LFAirport* pAirport, UINT Width, UINT Height);

void GetFileVersion(HMODULE hModule, CString* Version, CString* Copyright=NULL);
void LFCheckForUpdate(BOOL Force=FALSE, CWnd* pParentWnd=NULL);

INT LFMessageBox(CWnd* pParentWnd, CString Text, CString Caption, UINT Type);
void LFErrorBox(CWnd* pParentWnd, UINT Result);

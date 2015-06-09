#pragma once
#include "CCategory.h"
#include "CConditionList.h"
#include "CExplorerList.h"
#include "CExplorerNotification.h"
#include "CExplorerTree.h"
#include "CGdiPlusBitmap.h"
#include "CGlassEdit.h"
#include "CGlassPane.h"
#include "CGlassWindow.h"
#include "CGroupBox.h"
#include "CHeaderArea.h"
#include "CHeaderButton.h"
#include "CIconCtrl.h"
#include "CImageListTransparent.h"
#include "CInspectorGrid.h"
#include "CMapPreviewCtrl.h"
#include "CPropertyEdit.h"
#include "CSidebar.h"
#include "CStoreButton.h"
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
#include "LFProgressDlg.h"
#include "LFSelectLocationGPSDlg.h"
#include "LFSelectLocationIATADlg.h"
#include "LFStoreDataObject.h"
#include "LFStoreMaintenanceDlg.h"
#include "LFStorePropertiesDlg.h"
#include "LFTooltip.h"
#include "LFTransactionDataObject.h"
#include "LFUpdateDlg.h"
#include "MenuIcons.h"
#include "Workers.h"

#define LFGetApp() ((LFApplication*)AfxGetApp())

#define LFCategoryPadding     2

BOOL DuplicateGlobalMemory(const HGLOBAL hSrc, HGLOBAL& hDst);
INT GetAttributeIconIndex(UINT Attr);
void CreateRoundRectangle(CRect rect, INT rad, GraphicsPath& path);
void TooltipDataFromPIDL(LPITEMIDLIST pidl, CImageList* icons, HICON& hIcon, CSize& size, CString& caption, CString& hint);
BOOL IsCtrlThemed();
void DrawControlBorder(CWnd* pWnd);
void DrawCategory(CDC& dc, CRect rect, WCHAR* Caption, WCHAR* Hint, BOOL Themed);
void SetCompareComboBox(CComboBox* pComboBox, UINT attr, INT request=-1);

void GetFileVersion(HMODULE hModule, CString* Version, CString* Copyright=NULL);
void LFCheckForUpdate(BOOL Force=FALSE, CWnd* pParentWnd=NULL);

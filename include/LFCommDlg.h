#pragma once
#include "..\LFCommDlg\CAttributeProperties.h"
#include "..\LFCommDlg\CBottomArea.h"
#include "..\LFCommDlg\CCaptionBar.h"
#include "..\LFCommDlg\CDropdownSelector.h"
#include "..\LFCommDlg\CExplorerHeader.h"
#include "..\LFCommDlg\CExplorerList.h"
#include "..\LFCommDlg\CExplorerTree.h"
#include "..\LFCommDlg\CGdiPlusBitmap.h"
#include "..\LFCommDlg\CGlasWindow.h"
#include "..\LFCommDlg\CImageListTransparent.h"
#include "..\LFCommDlg\CInspectorGrid.h"
#include "..\LFCommDlg\CMapPreviewCtrl.h"
#include "..\LFCommDlg\CPaneList.h"
#include "..\LFCommDlg\CPaneText.h"
#include "..\LFCommDlg\CTagList.h"
#include "..\LFCommDlg\CTaskbar.h"
#include "..\LFCommDlg\CTaskButton.h"
#include "..\LFCommDlg\CTooltipHeader.h"
#include "..\LFCommDlg\LFAboutDlg.h"
#include "..\LFCommDlg\LFApplication.h"
#include "..\LFCommDlg\LFBrowseForFolderDlg.h"
#include "..\LFCommDlg\LFChooseDefaultStoreDlg.h"
#include "..\LFCommDlg\LFDropTarget.h"
#include "..\LFCommDlg\LFEditTagsDlg.h"
#include "..\LFCommDlg\LFItemTemplateDlg.h"
#include "..\LFCommDlg\LFLicenseDlg.h"
#include "..\LFCommDlg\LFSelectLocationGPSDlg.h"
#include "..\LFCommDlg\LFSelectLocationIATADlg.h"
#include "..\LFCommDlg\LFStoreDeleteDlg.h"
#include "..\LFCommDlg\LFStoreMaintenanceDlg.h"
#include "..\LFCommDlg\LFStoreNewDlg.h"
#include "..\LFCommDlg\LFStorePropertiesDlg.h"
#include "..\LFCommDlg\LFTooltip.h"
#include "..\LFCommDlg\LFWelcomeDlg.h"

#ifdef LFCommDlg_EXPORTS
#define LFCommDlg_API __declspec(dllexport)
#else
#define LFCommDlg_API __declspec(dllimport)
#endif

LFCommDlg_API void CreateRoundRectangle(CRect rect, int rad, GraphicsPath& path);
LFCommDlg_API void TooltipDataFromPIDL(LPITEMIDLIST pidl, CImageList* icons, HICON& hIcon, CSize& size, CString& caption, CString& hint);
LFCommDlg_API BOOL IsCtrlThemed();

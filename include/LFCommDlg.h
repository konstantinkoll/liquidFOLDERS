#pragma once
#include "..\LFCommDlg\CAttributeProperties.h"
#include "..\LFCommDlg\CBottomArea.h"
#include "..\LFCommDlg\CDropdownSelector.h"
#include "..\LFCommDlg\CExplorerHeader.h"
#include "..\LFCommDlg\CExplorerList.h"
#include "..\LFCommDlg\CExplorerNotification.h"
#include "..\LFCommDlg\CExplorerTree.h"
#include "..\LFCommDlg\CGdiPlusBitmap.h"
#include "..\LFCommDlg\CGlasPane.h"
#include "..\LFCommDlg\CGlasWindow.h"
#include "..\LFCommDlg\CHeaderButton.h"
#include "..\LFCommDlg\CIconCtrl.h"
#include "..\LFCommDlg\CImageListTransparent.h"
#include "..\LFCommDlg\CInspectorGrid.h"
#include "..\LFCommDlg\CMapPreviewCtrl.h"
#include "..\LFCommDlg\CStoreSelector.h"
#include "..\LFCommDlg\CTagList.h"
#include "..\LFCommDlg\CTaskbar.h"
#include "..\LFCommDlg\CTaskButton.h"
#include "..\LFCommDlg\CTooltipHeader.h"
#include "..\LFCommDlg\LFAboutDlg.h"
#include "..\LFCommDlg\LFApplication.h"
#include "..\LFCommDlg\LFAttributeListDlg.h"
#include "..\LFCommDlg\LFBrowseForFolderDlg.h"
#include "..\LFCommDlg\LFChooseStoreDlg.h"
#include "..\LFCommDlg\LFDropTarget.h"
#include "..\LFCommDlg\LFEditTagsDlg.h"
#include "..\LFCommDlg\LFItemTemplateDlg.h"
#include "..\LFCommDlg\LFLicenseDlg.h"
#include "..\LFCommDlg\LFSelectLocationGPSDlg.h"
#include "..\LFCommDlg\LFSelectLocationIATADlg.h"
#include "..\LFCommDlg\LFStoreDeleteDlg.h"
#include "..\LFCommDlg\LFStoreMaintenanceDlg.h"
#include "..\LFCommDlg\LFStoreNewDlg.h"
#include "..\LFCommDlg\LFStoreNewVolumeDlg.h"
#include "..\LFCommDlg\LFStorePropertiesDlg.h"
#include "..\LFCommDlg\LFTooltip.h"
#include "..\LFCommDlg\LFWelcomeDlg.h"

#ifdef LFCommDlg_EXPORTS
#define LFCommDlg_API __declspec(dllexport)
#else
#define LFCommDlg_API __declspec(dllimport)
#endif

LFCommDlg_API void CreateRoundRectangle(CRect rect, INT rad, GraphicsPath& path);
LFCommDlg_API void TooltipDataFromPIDL(LPITEMIDLIST pidl, CImageList* icons, HICON& hIcon, CSize& size, CString& caption, CString& hint);
LFCommDlg_API BOOL IsCtrlThemed();
LFCommDlg_API void DrawControlBorder(CWnd* pWnd);

LFCommDlg_API void LFImportFolder(CHAR* StoreID, CWnd* pParentWnd=NULL);
LFCommDlg_API void LFBackupStores(CWnd* pParentWnd=NULL);
LFCommDlg_API void LFAbout(CString AppName, CString Build, UINT IconResID, CWnd* pParent=NULL);

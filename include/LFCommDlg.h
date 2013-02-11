#pragma once
#include "..\LFCommDlg\CBottomArea.h"
#include "..\LFCommDlg\CConditionList.h"
#include "..\LFCommDlg\CDropdownSelector.h"
#include "..\LFCommDlg\CExplorerHeader.h"
#include "..\LFCommDlg\CExplorerList.h"
#include "..\LFCommDlg\CExplorerNotification.h"
#include "..\LFCommDlg\CExplorerTree.h"
#include "..\LFCommDlg\CFrameCtrl.h"
#include "..\LFCommDlg\CGdiPlusBitmap.h"
#include "..\LFCommDlg\CGlassEdit.h"
#include "..\LFCommDlg\CGlassPane.h"
#include "..\LFCommDlg\CGlassWindow.h"
#include "..\LFCommDlg\CGroupBox.h"
#include "..\LFCommDlg\CHeaderButton.h"
#include "..\LFCommDlg\CIconCtrl.h"
#include "..\LFCommDlg\CImageListTransparent.h"
#include "..\LFCommDlg\CInspectorGrid.h"
#include "..\LFCommDlg\CLabel.h"
#include "..\LFCommDlg\CMapPreviewCtrl.h"
#include "..\LFCommDlg\CPropertyEdit.h"
#include "..\LFCommDlg\CStorePanel.h"
#include "..\LFCommDlg\CStoreSelector.h"
#include "..\LFCommDlg\CTagList.h"
#include "..\LFCommDlg\CTaskbar.h"
#include "..\LFCommDlg\CTaskButton.h"
#include "..\LFCommDlg\CTooltipHeader.h"
#include "..\LFCommDlg\CTooltipList.h"
#include "..\LFCommDlg\LFAboutDlg.h"
#include "..\LFCommDlg\LFApplication.h"
#include "..\LFCommDlg\LFAttributeListDlg.h"
#include "..\LFCommDlg\LFBrowseForFolderDlg.h"
#include "..\LFCommDlg\LFChooseStoreDlg.h"
#include "..\LFCommDlg\LFDropSource.h"
#include "..\LFCommDlg\LFDropTarget.h"
#include "..\LFCommDlg\LFEditTagsDlg.h"
#include "..\LFCommDlg\LFEditTimeDlg.h"
#include "..\LFCommDlg\LFLicenseDlg.h"
#include "..\LFCommDlg\LFProgressDlg.h"
#include "..\LFCommDlg\LFSelectLocationGPSDlg.h"
#include "..\LFCommDlg\LFSelectLocationIATADlg.h"
#include "..\LFCommDlg\LFStoreDataObject.h"
#include "..\LFCommDlg\LFStoreDeleteDlg.h"
#include "..\LFCommDlg\LFStoreMaintenanceDlg.h"
#include "..\LFCommDlg\LFStoreNewLocalDlg.h"
#include "..\LFCommDlg\LFStorePropertiesDlg.h"
#include "..\LFCommDlg\LFTooltip.h"
#include "..\LFCommDlg\LFTransactionDataObject.h"
#include "..\LFCommDlg\LFWelcomeDlg.h"

#define LF_WORKERTHREAD_START(pParam) LF_WORKERTHREAD_START_EX(pParam, 0);
#define LF_WORKERTHREAD_START_EX(pParam, MajorCount) CoInitialize(NULL); WorkerParameters* wp = (WorkerParameters*)pParam; LFProgress p; LFInitProgress(&p, wp->Hdr.hWnd);
#define LF_WORKERTHREAD_FINISH() LF_WORKERTHREAD_FINISH_EX(LFOk);
#define LF_WORKERTHREAD_FINISH_EX(Result) CoUninitialize(); PostMessage(wp->Hdr.hWnd, WM_COMMAND, (WPARAM)IDOK, NULL); return Result;
#define LFGetApp() ((LFApplication*)AfxGetApp())

#ifdef LFCommDlg_EXPORTS
#define LFCommDlg_API __declspec(dllexport)
#else
#define LFCommDlg_API __declspec(dllimport)
#endif

LFCommDlg_API BOOL DuplicateGlobalMemory(const HGLOBAL hSrc, HGLOBAL& hDst);
LFCommDlg_API INT GetAttributeIconIndex(UINT Attr);
LFCommDlg_API void CreateRoundRectangle(CRect rect, INT rad, GraphicsPath& path);
LFCommDlg_API void TooltipDataFromPIDL(LPITEMIDLIST pidl, CImageList* icons, HICON& hIcon, CSize& size, CString& caption, CString& hint);
LFCommDlg_API BOOL IsCtrlThemed();
LFCommDlg_API void DrawControlBorder(CWnd* pWnd);
LFCommDlg_API void SetCompareComboBox(CComboBox* pComboBox, UINT attr, INT request=-1);

LFCommDlg_API void LFDoWithProgress(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd=NULL);
LFCommDlg_API void LFImportFolder(CHAR* StoreID, CWnd* pParentWnd=NULL);
LFCommDlg_API void LFRunMaintenance(CWnd* pParentWnd=NULL, HWND hWndSource=NULL);

LFCommDlg_API void LFCreateNewStore(CWnd* pParentWnd=NULL, CHAR Volume='\0');
LFCommDlg_API void LFAbout(CString AppName, CString Build, UINT IconResID, CWnd* pParentWnd=NULL);

LFCommDlg_API void GetFileVersion(HMODULE hModule, CString* Version, CString* Copyright=NULL);
LFCommDlg_API CString GetLatestVersion(CString& CurrentVersion);
LFCommDlg_API void LFCheckForUpdate(BOOL Force=FALSE, CWnd* pParentWnd=NULL);

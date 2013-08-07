
// LFCommDlg.cpp : Definiert die Initialisierungsroutinen für die DLL.
//

#include "stdafx.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include <winhttp.h>


extern AFX_EXTENSION_MODULE LFCommDlgDLL;


LFCommDlg_API BOOL DuplicateGlobalMemory(const HGLOBAL hSrc, HGLOBAL& hDst)
{
	if (!hSrc)
	{
		hDst = NULL;
		return FALSE;
	}

	SIZE_T sz = GlobalSize(hSrc);
	hDst = GlobalAlloc(GMEM_MOVEABLE, sz);
	if (!hDst)
		return FALSE;

	void* pSrc = GlobalLock(hSrc);
	void* pDst = GlobalLock(hDst);
	memcpy(pDst, pSrc, sz);
	GlobalUnlock(hSrc);
	GlobalUnlock(hDst);

	return TRUE;
}

LFCommDlg_API INT GetAttributeIconIndex(UINT Attr)
{
	static const UINT IconPosition[] = { LFAttrFileName, LFAttrStoreID, LFAttrFileID, LFAttrTitle, LFAttrCreationTime,
		LFAttrAddTime, LFAttrFileTime, LFAttrRecordingTime, LFAttrDeleteTime, LFAttrDueTime, LFAttrDoneTime, LFAttrArchiveTime,
		LFAttrLocationName, LFAttrLocationIATA, LFAttrLocationGPS, LFAttrRating, LFAttrRoll, LFAttrArtist, LFAttrComments,
		LFAttrDuration, LFAttrLanguage, LFAttrDimension, LFAttrWidth, LFAttrHeight, LFAttrAspectRatio, LFAttrTags,
		LFAttrAlbum, LFAttrPriority, LFAttrURL, LFAttrISBN, LFAttrRecordingEquipment, LFAttrChannels, LFAttrCustomer };

	for (UINT a=0; a<sizeof(IconPosition)/sizeof(UINT); a++)
		if (IconPosition[a]==Attr)
			return a;

	return -1;
}

LFCommDlg_API void CreateRoundRectangle(CRect rect, INT rad, GraphicsPath& path)
{
	path.Reset();

	INT l = rect.left;
	INT t = rect.top;
	INT w = rect.Width();
	INT h = rect.Height();
	INT d = rad<<1;

	path.AddArc(l, t, d, d, 180, 90);
	path.AddLine(l+rad, t, l+w-rad, t);
	path.AddArc(l+w-d, t, d, d, 270, 90);
	path.AddLine(l+w, t+rad, l+w, t+h-rad);
	path.AddArc(l+w-d, t+h-d, d, d, 0, 90);
	path.AddLine(l+w-rad, t+h, l+rad, t+h);
	path.AddArc(l, t+h-d, d, d, 90, 90);
	path.AddLine(l, t+h-rad, l, t+rad);
	path.CloseFigure();
}

LFCommDlg_API void TooltipDataFromPIDL(LPITEMIDLIST pidl, CImageList* icons, HICON& hIcon, CSize& size, CString& caption, CString& hint)
{
	LFApplication* pApp = LFGetApp();

	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((WCHAR*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_SYSICONINDEX | SHGFI_LARGEICON)))
	{
		hIcon = icons->ExtractIcon(sfi.iIcon);
		caption = sfi.szDisplayName;
		hint = sfi.szTypeName;

		IShellFolder* pParentFolder = NULL;
		LPCITEMIDLIST Child = NULL;
		if (SUCCEEDED(SHBindToParent(pidl, IID_IShellFolder, (void**)&pParentFolder, &Child)))
		{
			WIN32_FIND_DATA ffd;
			if (SUCCEEDED(SHGetDataFromIDList(pParentFolder, Child, SHGDFIL_FINDDATA, &ffd, sizeof(WIN32_FIND_DATA))))
			{
				FILETIME lft;
				WCHAR tmpBuf1[256];
				FileTimeToLocalFileTime(&ffd.ftCreationTime, &lft);
				LFTimeToString(lft, tmpBuf1, 256);
				WCHAR tmpBuf2[256];
				FileTimeToLocalFileTime(&ffd.ftLastWriteTime, &lft);
				LFTimeToString(lft, tmpBuf2, 256);

				CString tmpStr;
				tmpStr.Format(_T("\n%s: %s\n%s: %s"),
					pApp->m_Attributes[LFAttrCreationTime]->Name, tmpBuf1,
					pApp->m_Attributes[LFAttrFileTime]->Name, tmpBuf2);
				hint.Append(tmpStr);
			}
			pParentFolder->Release();
		}

		INT cx = 48;
		INT cy = 48;
		ImageList_GetIconSize(*icons, &cx, &cy);
		size.cx = cx;
		size.cy = cy;
	}
}

LFCommDlg_API BOOL IsCtrlThemed()
{
	LFApplication* pApp = LFGetApp();
	if (pApp)
		if (pApp->m_ThemeLibLoaded)
			return pApp->zIsAppThemed();

	return FALSE;
}

LFCommDlg_API void DrawControlBorder(CWnd* pWnd)
{
	CRect rect;
	pWnd->GetWindowRect(rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;

	CWindowDC dc(pWnd);

	LFApplication* pApp = LFGetApp();
	if (pApp->m_ThemeLibLoaded)
		if (pApp->zIsAppThemed())
		{
			HTHEME hTheme = pApp->zOpenThemeData(pWnd->GetSafeHwnd(), VSCLASS_LISTBOX);
			if (hTheme)
			{
				CRect rectClient(rect);
				rectClient.DeflateRect(2, 2);
				dc.ExcludeClipRect(rectClient);

				pApp->zDrawThemeBackground(hTheme, dc, LBCP_BORDER_NOSCROLL, pWnd->IsWindowEnabled() ? GetFocus()==pWnd->GetSafeHwnd() ? LBPSN_FOCUSED : LBPSN_NORMAL : LBPSN_DISABLED, rect, rect);
				pApp->zCloseThemeData(hTheme);

				return;
			}
		}

	dc.Draw3dRect(rect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));
	rect.DeflateRect(1, 1);
	dc.Draw3dRect(rect, 0x000000, GetSysColor(COLOR_3DFACE));
}

void AddCompare(CComboBox* pComboBox, UINT ResID, UINT CompareID)
{
	CString tmpStr;
	ENSURE(tmpStr.LoadString(ResID));

	pComboBox->SetItemData(pComboBox->AddString(tmpStr), CompareID);
}

LFCommDlg_API void SetCompareComboBox(CComboBox* pComboBox, UINT attr, INT request)
{
	pComboBox->SetRedraw(FALSE);
	pComboBox->ResetContent();

	switch (LFGetApp()->m_Attributes[attr]->Type)
	{
	case LFTypeUnicodeString:
	case LFTypeAnsiString:
		AddCompare(pComboBox, IDS_COMPARE_CONTAINS, LFFilterCompareContains);
		AddCompare(pComboBox, IDS_COMPARE_ISEQUAL, LFFilterCompareIsEqual);
		AddCompare(pComboBox, IDS_COMPARE_ISNOTEQUAL, LFFilterCompareIsNotEqual);
		AddCompare(pComboBox, IDS_COMPARE_BEGINSWITH, LFFilterCompareBeginsWith);
		AddCompare(pComboBox, IDS_COMPARE_ENDSWITH, LFFilterCompareEndsWith);
		break;
	case LFTypeUnicodeArray:
		AddCompare(pComboBox, IDS_COMPARE_CONTAINS, LFFilterCompareContains);
		break;
	case LFTypeFourCC:
	case LFTypeFraction:
	case LFTypeFlags:
	case LFTypeGeoCoordinates:
		AddCompare(pComboBox, IDS_COMPARE_ISEQUAL, LFFilterCompareIsEqual);
		AddCompare(pComboBox, IDS_COMPARE_ISNOTEQUAL, LFFilterCompareIsNotEqual);
		break;
	case LFTypeRating:
	case LFTypeUINT:
	case LFTypeINT64:
	case LFTypeTime:
	case LFTypeDouble:
	case LFTypeDuration:
	case LFTypeBitrate:
	case LFTypeMegapixel:
		AddCompare(pComboBox, IDS_COMPARE_ISEQUAL, LFFilterCompareIsEqual);
		AddCompare(pComboBox, IDS_COMPARE_ISNOTEQUAL, LFFilterCompareIsNotEqual);
		AddCompare(pComboBox, IDS_COMPARE_ISABOVEEQUAL, LFFilterCompareIsAboveOrEqual);
		AddCompare(pComboBox, IDS_COMPARE_ISBELOWEQUAL, LFFilterCompareIsBelowOrEqual);
		break;
	default:
		ASSERT(FALSE);
	}

	if (request!=-1)
	{
		BOOL first = TRUE;

		for (INT a=0; a<pComboBox->GetCount(); a++)
		{
			INT data = (INT)pComboBox->GetItemData(a);
			if ((data==request) || ((first==TRUE) && ((data==LFFilterCompareIsEqual) || (data==LFFilterCompareContains))))
			{
				pComboBox->SetCurSel(a);
				first = FALSE;
			}
		}
	}

	pComboBox->SetRedraw(TRUE);
	pComboBox->Invalidate();
}


struct WorkerParameters
{
	LFWorkerParameters Hdr;
	CHAR StoreID[LFKeySize];
	BOOL DeleteSource;
	HWND hWndSource;
	union
	{
		LFFileImportList* FileImportList;
		LFMaintenanceList* MaintenanceList;
	};
};

DWORD WINAPI WorkerImport(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	LFTransactionImport(wp->StoreID, wp->FileImportList, NULL, true, wp->DeleteSource==TRUE, &p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerMaintenance(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	wp->MaintenanceList = LFStoreMaintenance(wp->hWndSource, &p);

	LF_WORKERTHREAD_FINISH();
}

LFCommDlg_API void LFDoWithProgress(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd)
{
	LFProgressDlg dlg(pThreadProc, pParameters, pParentWnd);
	dlg.DoModal();
}

LFCommDlg_API void LFImportFolder(CHAR* StoreID, CWnd* pParentWnd)
{
	// Allowed?
	if (LFGetApp()->ShowNagScreen(NAG_EXPIRED | NAG_FORCE, pParentWnd, TRUE))
		return;

	CString caption;
	ENSURE(caption.LoadString(IDS_IMPORTFOLDER_CAPTION));
	CString hint;
	ENSURE(hint.LoadString(IDS_IMPORTFOLDER_HINT));

	LFBrowseForFolderDlg dlg(TRUE, TRUE, _T(""), pParentWnd, caption, hint);
	if (dlg.DoModal()==IDOK)
	{
		WorkerParameters wp;
		ZeroMemory(&wp, sizeof(wp));
		wp.FileImportList = LFAllocFileImportList();
		LFAddImportPath(wp.FileImportList, dlg.m_FolderPath);
		strcpy_s(wp.StoreID, LFKeySize, StoreID);
		wp.DeleteSource = dlg.m_DeleteSource;

		LFDoWithProgress(WorkerImport, (LFWorkerParameters*)&wp, pParentWnd);

		LFErrorBox(wp.FileImportList->m_LastError, pParentWnd ? pParentWnd->GetSafeHwnd() : NULL);

		LFFreeFileImportList(wp.FileImportList);
	}
}

LFCommDlg_API void LFRunMaintenance(CWnd* pParentWnd, HWND hWndSource)
{
	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	wp.hWndSource = hWndSource;

	LFDoWithProgress(WorkerMaintenance, &wp.Hdr, pParentWnd);
	LFErrorBox(wp.MaintenanceList->m_LastError, pParentWnd->GetSafeHwnd());

	LFStoreMaintenanceDlg dlg(wp.MaintenanceList, pParentWnd);
	dlg.DoModal();

	LFFreeMaintenanceList(wp.MaintenanceList);
}


LFCommDlg_API void LFCreateNewStore(CWnd* pParentWnd, CHAR Volume)
{
	LFStoreNewLocalDlg dlg(pParentWnd, Volume);
	dlg.DoModal();
}

LFCommDlg_API void LFAbout(CString AppName, CString Build, UINT IconResID, CWnd* pParentWnd)
{
	LFAboutDlg dlg(AppName, Build, IconResID, pParentWnd);
	dlg.DoModal();
}


LFCommDlg_API void GetFileVersion(HMODULE hModule, CString* Version, CString* Copyright)
{
	if (Version)
		Version->Empty();
	if (Copyright)
		Copyright->Empty();

	CString modFilename;
	if (GetModuleFileName(hModule, modFilename.GetBuffer(MAX_PATH), MAX_PATH)>0)
	{
		modFilename.ReleaseBuffer(MAX_PATH);
		DWORD dwHandle = 0;
		DWORD dwSize = GetFileVersionInfoSize(modFilename, &dwHandle);
		if (dwSize>0)
		{
			LPBYTE lpInfo = new BYTE[dwSize];
			ZeroMemory(lpInfo, dwSize);

			if (GetFileVersionInfo(modFilename, 0, dwSize, lpInfo))
			{
				UINT valLen = 0;
				LPVOID valPtr = NULL;
				LPCWSTR valData = NULL;

				if (Version)
					if (VerQueryValue(lpInfo, _T("\\"), &valPtr, &valLen))
					{
						VS_FIXEDFILEINFO* pFinfo = (VS_FIXEDFILEINFO*)valPtr;
						Version->Format(_T("%u.%u.%u"), 
							(UINT)((pFinfo->dwProductVersionMS >> 16) & 0xFF),
							(UINT)((pFinfo->dwProductVersionMS) & 0xFF),
							(UINT)((pFinfo->dwProductVersionLS >> 16) & 0xFF));
					}
				if (Copyright)
					*Copyright = VerQueryValue(lpInfo, _T("StringFileInfo\\000004E4\\LegalCopyright"), (void**)&valData, &valLen) ? valData : _T("© liquidFOLDERS");
			}
			delete[] lpInfo;
		}
	}
}

LFCommDlg_API CString GetLatestVersion(CString& CurrentVersion)
{
	CString VersionIni;

	// Obtain current version from DLL version resource
	GetFileVersion(LFCommDlgDLL.hModule, &CurrentVersion);

	// Variant
#ifdef _M_X64
#define ISET _T(" (x64")
#else
#define ISET _T(" (x32")
#endif

	CurrentVersion += ISET;

	// Licensed?
	if (LFIsLicensed())
	{
		CurrentVersion += _T("; licensed");
	}
	else
		if (LFIsSharewareExpired())
		{
			CurrentVersion += _T("; expired");
		}

	CurrentVersion += _T(")");

	// Get available version
	HINTERNET hSession = WinHttpOpen(_T("liquidFOLDERS/")+CurrentVersion, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession)
	{
		HINTERNET hConnect = WinHttpConnect(hSession, L"update.liquidfolders.net", INTERNET_DEFAULT_HTTP_PORT, 0);
		if (hConnect)
		{
			HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/version.ini", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
			if (hRequest)
			{
				if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0))
					if (WinHttpReceiveResponse(hRequest, NULL))
					{
						DWORD dwSize;

						do
						{
							dwSize = 0;
							if (WinHttpQueryDataAvailable(hRequest, &dwSize))
							{
								CHAR* pBuffer = new CHAR[dwSize+1];
								DWORD dwDownloaded;
								if (WinHttpReadData(hRequest, pBuffer, dwSize, &dwDownloaded))
								{
									pBuffer[dwDownloaded] = '\0';
									CString tmpStr(pBuffer);
									VersionIni += tmpStr;
									delete[] pBuffer;
								}
							}
						}
						while (dwSize>0);
					}

				WinHttpCloseHandle(hRequest);
			}

			WinHttpCloseHandle(hConnect);
		}

		WinHttpCloseHandle(hSession);
	}

	return VersionIni;
}

CString GetIniValue(CString Ini, CString Name)
{
	while (!Ini.IsEmpty())
	{
		INT pos = Ini.Find(L"\n");
		if (pos==-1)
			pos = Ini.GetLength()+1;

		CString Line = Ini.Mid(0, pos-1);
		Ini.Delete(0, pos+1);

		pos = Line.Find(L"=");
		if (pos!=-1)
			if (Line.Mid(0, pos)==Name)
				return Line.Mid(pos+1, Line.GetLength()-pos);
	}

	return _T("");
}

struct Version
{
	UINT Major, Minor, Build;
};

__forceinline INT ParseVersion(CString ver, Version* v)
{
	ZeroMemory(v, sizeof(Version));
	return swscanf_s(ver, L"%u.%u.%u", &v->Major, &v->Minor, &v->Build);
}

LFCommDlg_API void LFCheckForUpdate(BOOL Force, CWnd* pParentWnd)
{
	BOOL UpdateFound = FALSE;
	BOOL Check = Force;

	// Check due?
	if (!Check)
		Check = LFGetApp()->IsUpdateCheckDue();

	// Perform check
	CString VersionIni;
	CString LatestVersion;
	if (Check)
	{
		CWaitCursor csr;

		CString CurrentVersion;
		VersionIni = GetLatestVersion(CurrentVersion);

		if (!VersionIni.IsEmpty())
		{
			LatestVersion = GetIniValue(VersionIni, _T("Version"));
			if (!LatestVersion.IsEmpty())
			{
				Version CV;
				Version LV;
				ParseVersion(CurrentVersion, &CV);
				ParseVersion(LatestVersion, &LV);

				UpdateFound = (LV.Major>CV.Major) ||
								((LV.Major==CV.Major) && (LV.Minor>CV.Minor)) ||
								((LV.Major==CV.Major) && (LV.Minor==CV.Minor) && (LV.Build>CV.Build));
			}
		}
	}

	// Result
	CString Caption;
	ENSURE(Caption.LoadString(IDS_UPDATE));

	if (UpdateFound)
	{
		CString Mask;
		ENSURE(Mask.LoadString(IDS_UPDATE_AVAILABLE));

		CString Text;
		Text.Format(Mask, LatestVersion);
		if (MessageBox(pParentWnd->GetSafeHwnd(), Text, Caption, MB_ICONQUESTION | MB_YESNO)==IDYES)
		{
			CString url;
			ENSURE(url.LoadString(IDS_UPDATEURL));

			ShellExecute(pParentWnd->GetSafeHwnd(), _T("open"), url, NULL, NULL, SW_SHOW);
		}
	}
	else
		if (Force)
		{
			CString Text;
			ENSURE(Text.LoadString(IDS_UPDATE_NOTAVAILABLE));
			MessageBox(pParentWnd->GetSafeHwnd(), Text, Caption, MB_ICONINFORMATION | MB_OK);
		}
}

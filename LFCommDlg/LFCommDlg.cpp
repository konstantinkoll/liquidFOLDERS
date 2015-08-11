
// LFCommDlg.cpp : Definiert die Initialisierungsroutinen f�r die DLL.
//

#include "stdafx.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include <winhttp.h>


BOOL DuplicateGlobalMemory(const HGLOBAL hSrc, HGLOBAL& hDst)
{
	if (!hSrc)
	{
		hDst = NULL;
		return FALSE;
	}

	SIZE_T Size = GlobalSize(hSrc);

	hDst = GlobalAlloc(GMEM_MOVEABLE, Size);
	if (!hDst)
		return FALSE;

	void* pSrc = GlobalLock(hSrc);
	void* pDst = GlobalLock(hDst);

	memcpy(pDst, pSrc, Size);

	GlobalUnlock(hSrc);
	GlobalUnlock(hDst);

	return TRUE;
}


INT GetAttributeIconIndex(UINT Attr)
{
	static const UINT IconPosition[] = { LFAttrFileName, LFAttrStoreID, LFAttrFileID, LFAttrTitle, LFAttrCreationTime,
		LFAttrAddTime, LFAttrFileTime, LFAttrRecordingTime, LFAttrDeleteTime, LFAttrDueTime, LFAttrDoneTime, LFAttrArchiveTime,
		LFAttrLocationIATA, LFAttrLocationGPS, LFAttrRating, LFAttrRoll, LFAttrArtist, LFAttrComments, LFAttrDuration,
		LFAttrLanguage, LFAttrDimension, LFAttrWidth, LFAttrHeight, LFAttrAspectRatio, LFAttrHashtags, LFAttrAlbum,
		LFAttrPriority, LFAttrURL, LFAttrISBN, LFAttrEquipment, LFAttrChannels, LFAttrCustomer, LFAttrLikeCount };

	for (UINT a=0; a<sizeof(IconPosition)/sizeof(UINT); a++)
		if (IconPosition[a]==Attr)
			return a;

	return -1;
}

void CreateRoundRectangle(CRect rect, INT Radius, GraphicsPath& Path)
{
	Path.Reset();

	INT l = rect.left;
	INT t = rect.top;
	INT w = rect.Width();
	INT h = rect.Height();
	INT d = Radius<<1;

	Path.AddArc(l, t, d, d, 180, 90);
	Path.AddLine(l+Radius, t, l+w-Radius, t);
	Path.AddArc(l+w-d, t, d, d, 270, 90);
	Path.AddLine(l+w, t+Radius, l+w, t+h-Radius);
	Path.AddArc(l+w-d, t+h-d, d, d, 0, 90);
	Path.AddLine(l+w-Radius, t+h, l+Radius, t+h);
	Path.AddArc(l, t+h-d, d, d, 90, 90);
	Path.AddLine(l, t+h-Radius, l, t+Radius);
	Path.CloseFigure();
}

void TooltipDataFromPIDL(LPITEMIDLIST pidl, CImageList* pIcons, HICON& hIcon, CString& Caption, CString& Hint)
{
	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((WCHAR*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_SYSICONINDEX | SHGFI_LARGEICON)))
	{
		hIcon = pIcons->ExtractIcon(sfi.iIcon);
		Caption = sfi.szDisplayName;
		Hint = sfi.szTypeName;

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
					LFGetApp()->m_Attributes[LFAttrCreationTime].Name, tmpBuf1,
					LFGetApp()->m_Attributes[LFAttrFileTime].Name, tmpBuf2);
				Hint.Append(tmpStr);
			}
			pParentFolder->Release();
		}
	}
}

BOOL IsCtrlThemed()
{
	return LFGetApp()->m_ThemeLibLoaded ? LFGetApp()->zIsAppThemed() : FALSE;
}

HBITMAP CreateTransparentBitmap(LONG Width, LONG Height)
{
	BITMAPINFO DIB;
	ZeroMemory(&DIB, sizeof(DIB));

	DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DIB.bmiHeader.biWidth = Width;
	DIB.bmiHeader.biHeight = -Height;
	DIB.bmiHeader.biPlanes = 1;
	DIB.bmiHeader.biBitCount = 32;
	DIB.bmiHeader.biCompression = BI_RGB;

	HDC hDC = GetDC(NULL);
	HBITMAP hBitmap = CreateDIBSection(hDC, &DIB, DIB_RGB_COLORS, NULL, NULL, 0);
	ReleaseDC(NULL, hDC);

	return hBitmap;
}

void DrawControlBorder(CWnd* pWnd)
{
	CRect rect;
	pWnd->GetWindowRect(rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;

	CWindowDC dc(pWnd);

	if (LFGetApp()->m_ThemeLibLoaded)
		if (LFGetApp()->zIsAppThemed())
		{
			HTHEME hTheme = LFGetApp()->zOpenThemeData(pWnd->GetSafeHwnd(), VSCLASS_LISTBOX);
			if (hTheme)
			{
				CRect rectClient(rect);
				rectClient.DeflateRect(2, 2);
				dc.ExcludeClipRect(rectClient);

				LFGetApp()->zDrawThemeBackground(hTheme, dc, LBCP_BORDER_NOSCROLL, pWnd->IsWindowEnabled() ? GetFocus()==pWnd->GetSafeHwnd() ? LBPSN_FOCUSED : LBPSN_NORMAL : LBPSN_DISABLED, rect, rect);
				LFGetApp()->zCloseThemeData(hTheme);

				return;
			}
		}

	dc.Draw3dRect(rect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));
	rect.DeflateRect(1, 1);
	dc.Draw3dRect(rect, 0x000000, GetSysColor(COLOR_3DFACE));
}

void DrawCategory(CDC& dc, CRect rect, WCHAR* Caption, WCHAR* Hint, BOOL Themed)
{
	ASSERT(Caption);

	dc.SetBkMode(TRANSPARENT);

	rect.DeflateRect(LFCategoryPadding, LFCategoryPadding);

	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_LargeFont);
	dc.SetTextColor(Themed ? 0xCC3300 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(Caption, rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

	CRect rectLine(rect);
	dc.DrawText(Caption, rectLine, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT | DT_NOPREFIX);
	rectLine.right += 2*LFCategoryPadding;

	if (rectLine.right<=rect.right)
		if (Themed)
		{
			Graphics g(dc);
	
			LinearGradientBrush brush(Point(rectLine.right, rectLine.top), Point(rect.right, rectLine.top), Color(0xFF, 0xE2, 0xE2, 0xE2), Color(0x00, 0xE2, 0xE2, 0xE2));
			g.FillRectangle(&brush, rectLine.right, rectLine.top+(rectLine.Height()+1)/2, rect.right-rectLine.right, 1);
		}
		else
		{
			CPen pen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));

			CPen* pOldPen = dc.SelectObject(&pen);
			dc.MoveTo(rectLine.right, rect.top+(rectLine.Height()+1)/2);
			dc.LineTo(rect.right, rect.top+(rectLine.Height()+1)/2);
			dc.SelectObject(pOldPen);
		}

	if (Hint)
		if (Hint[0]!=L'\0')
		{
			if (Themed)
				dc.SetTextColor(((dc.GetTextColor()>>1) & 0x7F7F7F) | 0x808080);

			rect.top += rectLine.Height();
			dc.SelectObject(&LFGetApp()->m_DefaultFont);
			dc.DrawText(Hint, rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
		}

	dc.SelectObject(pOldFont);
}

void DrawListItemBackground(CDC& dc, LPRECT rectItem, HTHEME hThemeList, BOOL Themed, BOOL WinFocused, BOOL Hot, BOOL Focused, BOOL Selected, COLORREF TextColor, BOOL ShowFocusRect)
{
	dc.SetBkMode(TRANSPARENT);

	if (hThemeList)
	{
		dc.SetTextColor(TextColor==(COLORREF)-1 ? 0x000000 : TextColor);

		if (Hot | Selected)
		{
			const INT StateIDs[4] = { LISS_NORMAL, LISS_HOT, !WinFocused ? LISS_SELECTEDNOTFOCUS : LISS_SELECTED, LISS_HOTSELECTED };
			UINT State = 0;
			if (Hot)
				State |= 1;
			if (Selected)
				State |= 2;
			LFGetApp()->zDrawThemeBackground(hThemeList, dc, LVP_LISTITEM, StateIDs[State], rectItem, rectItem);
		}

		if (WinFocused && Focused)
			switch (LFGetApp()->OSVersion)
			{
			case OS_Vista:
				if (ShowFocusRect)
				{
					CRect rect(rectItem);
					rect.DeflateRect(1, 1);

					dc.SetBkColor(0xFFFFFF);
					dc.DrawFocusRect(rect);
				}

				break;

			case OS_Seven:
				if (!Selected)
				{
					CRect rect(rectItem);
					rect.bottom--;
					rect.right--;

					Graphics g(dc);
					g.SetSmoothingMode(SmoothingModeAntiAlias);

					GraphicsPath path;
					CreateRoundRectangle(rect, 2, path);

					Pen pen(Color(0xFF, 0x7D, 0xA2, 0xCE));
					g.DrawPath(&pen, &path);
				}

				break;

			case OS_Eight:
				if (!Selected)
					dc.Draw3dRect(rectItem, 0xCEA27D, 0xCEA27D);

				break;
			}
	}
	else
	{
		if (Selected)
		{
			dc.FillSolidRect(rectItem, GetSysColor(WinFocused ? COLOR_HIGHLIGHT : COLOR_3DFACE));
			dc.SetTextColor(GetSysColor(WinFocused ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
			dc.SetBkColor(0x000000);
		}
		else
		{
			dc.SetTextColor(Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
			dc.SetBkColor(Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

			if (!Focused)
				dc.SetBkMode(OPAQUE);
		}

		if (WinFocused && Focused)
			dc.DrawFocusRect(rectItem);

		if (TextColor!=(COLORREF)-1 && !Selected)
			dc.SetTextColor(TextColor);
	}
}

void DrawLightButtonBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover)
{
	if (Themed)
	{
		if (Focused || Selected || Hover)
		{
			Graphics g(dc);
			g.SetSmoothingMode(SmoothingModeNone);

			CRect rectBounds(rect);
			rectBounds.right--;
			rectBounds.bottom--;

			// Inner Border
			rectBounds.DeflateRect(1, 1);

			if (Selected)
			{
				SolidBrush brush(Color(0x20, 0x50, 0x57, 0x62));
				g.FillRectangle(&brush, rectBounds.left, rectBounds.top, rectBounds.Width()+1, rectBounds.Height()+1);
			}
			else
				if (Hover)
				{
					SolidBrush brush1(Color(0x40, 0xFF, 0xFF, 0xFF));
					g.FillRectangle(&brush1, rectBounds.left, rectBounds.top+1, rectBounds.Width(), rectBounds.Height()/2+1);

					SolidBrush brush2(Color(0x28, 0xA0, 0xAF, 0xC3));
					g.FillRectangle(&brush2, rectBounds.left, rectBounds.top+rectBounds.Height()/2+2, rectBounds.Width(), rectBounds.Height()/2-1);
				}

			g.SetSmoothingMode(SmoothingModeAntiAlias);
			GraphicsPath path;

			if (!Selected)
			{
				CreateRoundRectangle(rectBounds, 1, path);

				Pen pen(Color(0x80, 0xFF, 0xFF, 0xFF));
				g.DrawPath(&pen, &path);
			}

			// Outer Border
			rectBounds.InflateRect(1, 1);
			CreateRoundRectangle(rectBounds, 2, path);

			Pen pen(Color(0x70, 0x50, 0x57, 0x62));
			g.DrawPath(&pen, &path);
		}
	}
	else
	{
		if (Selected || Hover)
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
			dc.DrawEdge(rect, Selected ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT | BF_SOFT);
		}

		if (Focused)
		{
			CRect rectFocus(rect);
			rectFocus.DeflateRect(2, 2);
			dc.SetTextColor(0x000000);
			dc.DrawFocusRect(rectFocus);
		}
	}
}


void AddCompare(CComboBox* pComboBox, UINT ResID, UINT CompareID)
{
	CString tmpStr((LPCSTR)ResID);

	pComboBox->SetItemData(pComboBox->AddString(tmpStr), CompareID);
}

void SetCompareComboBox(CComboBox* pComboBox, UINT Attr, INT request)
{
	pComboBox->SetRedraw(FALSE);
	pComboBox->ResetContent();

	switch (LFGetApp()->m_Attributes[Attr].Type)
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
	case LFTypeSize:
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


void AppendTooltipString(UINT Attr, CString& Str, WCHAR* tmpStr)
{
	if (tmpStr)
		if (tmpStr[0]!=L'\0')
		{
			if (!Str.IsEmpty())
				Str += _T("\n");
			if ((Attr!=LFAttrComments) && (Attr!=LFAttrFileFormat) && (Attr!=LFAttrDescription))
			{
				Str += LFGetApp()->m_Attributes[Attr].Name;
				Str += _T(": ");
			}

			Str += tmpStr;
		}
}

void AppendTooltipAttribute(LFItemDescriptor* i, UINT Attr, CString& Str)
{
	WCHAR tmpStr[256];
	LFAttributeToString(i, Attr, tmpStr, 256);
	AppendTooltipString(Attr, Str, tmpStr);
}

void GetHintForStore(LFItemDescriptor* i, CString& Str)
{
	WCHAR tmpStr[256];

	AppendTooltipAttribute(i, LFAttrComments, Str);

	LFCombineFileCountSize(i->AggregateCount, i->CoreAttributes.FileSize, tmpStr, 256);
	AppendTooltipString(LFAttrDescription, Str, tmpStr);

	AppendTooltipAttribute(i, LFAttrCreationTime, Str);
	AppendTooltipAttribute(i, LFAttrFileTime, Str);

	AppendTooltipAttribute(i, LFAttrDescription, Str);

	if (((i->Type & LFTypeSourceMask)>LFTypeSourceInternal) && (!(i->Type & LFStoreNotMounted)))
		AppendTooltipString(LFAttrComments, Str, LFGetApp()->m_SourceNames[i->Type & LFTypeSourceMask][1]);
}


void GetFileVersion(HMODULE hModule, CString* Version, CString* Copyright)
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
					*Copyright = VerQueryValue(lpInfo, _T("StringFileInfo\\000004E4\\LegalCopyright"), (void**)&valData, &valLen) ? valData : _T("� liquidFOLDERS");
			}

			delete[] lpInfo;
		}
	}
}

CString GetLatestVersion(CString CurrentVersion)
{
	CString VersionIni;

	// License
	if (LFIsLicensed())
	{
		CurrentVersion += _T(" (licensed)");
	}
	else
		if (LFIsSharewareExpired())
		{
			CurrentVersion += _T(" (expired");
		}

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
								}

								delete[] pBuffer;
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

CString GetIniValue(CString Ini, const CString Name)
{
	while (!Ini.IsEmpty())
	{
		INT Pos = Ini.Find(L"\n");
		if (Pos==-1)
			Pos = Ini.GetLength()+1;

		CString Line = Ini.Mid(0, Pos-1);
		Ini.Delete(0, Pos+1);

		Pos = Line.Find(L"=");
		if (Pos!=-1)
			if (Line.Mid(0, Pos)==Name)
				return Line.Mid(Pos+1, Line.GetLength()-Pos);
	}

	return _T("");
}

void ParseVersion(const CString tmpStr, LFVersion* pVersion)
{
	ASSERT(pVersion);

	swscanf_s(tmpStr, L"%u.%u.%u", &pVersion->Major, &pVersion->Minor, &pVersion->Build);
}

BOOL IsVersionLater(LFVersion& LatestVersion, LFVersion& CurrentVersion)
{
	return ((LatestVersion.Major>CurrentVersion.Major) ||
		((LatestVersion.Major==CurrentVersion.Major) && (LatestVersion.Minor>CurrentVersion.Minor)) ||
		((LatestVersion.Major==CurrentVersion.Major) && (LatestVersion.Minor==CurrentVersion.Minor) && (LatestVersion.Build>CurrentVersion.Build)));
}

BOOL IsLaterFeature(const CString VersionIni, const CString Name, LFVersion& CurrentVersion)
{
	LFVersion FeatureVersion = { 0 };

	ParseVersion(GetIniValue(VersionIni, Name), &FeatureVersion);

	return IsVersionLater(FeatureVersion, CurrentVersion);
}

DWORD GetFeatures(const CString VersionIni, LFVersion& CurrentVersion)
{
	DWORD Features = 0;

	if (IsLaterFeature(VersionIni, _T("SecurityPatch"), CurrentVersion))
		Features |= UPDATE_SECUTIRYPATCH;

	if (IsLaterFeature(VersionIni, _T("ImportantBugfix"), CurrentVersion))
		Features |= UPDATE_IMPORTANTBUGFIX;

	if (IsLaterFeature(VersionIni, _T("NetworkAPI"), CurrentVersion))
		Features |= UPDATE_NETWORKAPI;

	if (IsLaterFeature(VersionIni, _T("NewFeature"), CurrentVersion))
		Features |= UPDATE_NEWFEATURE;

	if (IsLaterFeature(VersionIni, _T("NewVisualization"), CurrentVersion))
		Features |= UPDATE_NEWVISUALIZATION;

	if (IsLaterFeature(VersionIni, _T("UI"), CurrentVersion))
		Features |= UPDATE_UI;

	if (IsLaterFeature(VersionIni, _T("SmallBugfix"), CurrentVersion))
		Features |= UPDATE_SMALLBUGFIX;

	if (IsLaterFeature(VersionIni, _T("IATA"), CurrentVersion))
		Features |= UPDATE_IATA;

	if (IsLaterFeature(VersionIni, _T("Performance"), CurrentVersion))
		Features |= UPDATE_PERFORMANCE;

	return Features;
}

void LFCheckForUpdate(BOOL Force, CWnd* pParentWnd)
{
	// Obtain current version from instance version resource
	CString CurrentVersion;
	GetFileVersion(AfxGetResourceHandle(), &CurrentVersion);

	LFVersion CV = { 0 };
	ParseVersion(CurrentVersion, &CV);

	// Check due?
	BOOL Check = Force;
	if (!Check)
		Check = LFGetApp()->IsUpdateCheckDue();

	// Perform check
	CString LatestVersion = LFGetApp()->GetGlobalString(_T("LatestUpdateVersion"));
	CString LatestMSN = LFGetApp()->GetGlobalString(_T("LatestUpdateMSN"));
	DWORD LatestFeatures = LFGetApp()->GetGlobalInt(_T("LatestUpdateFeatures"));

	if (Check)
	{
		CWaitCursor wait;
		CString VersionIni = GetLatestVersion(CurrentVersion);

		if (!VersionIni.IsEmpty())
		{
			LatestVersion = GetIniValue(VersionIni, _T("Version"));
			LatestMSN = GetIniValue(VersionIni, _T("MSN"));
			LatestFeatures = GetFeatures(VersionIni, CV);

			LFGetApp()->WriteGlobalString(_T("LatestUpdateVersion"), LatestVersion);
			LFGetApp()->WriteGlobalString(_T("LatestUpdateMSN"), LatestMSN);
			LFGetApp()->WriteGlobalInt(_T("LatestUpdateFeatures"), LatestFeatures);
		}
	}

	// Update available?
	BOOL UpdateAvailable = FALSE;
	if (!LatestVersion.IsEmpty())
	{
		LFVersion LV = { 0 };
		ParseVersion(LatestVersion, &LV);

		CString IgnoreMSN = LFGetApp()->GetGlobalString(_T("IgnoreUpdateMSN"));

		UpdateAvailable = ((IgnoreMSN!=LatestMSN) || (Force)) && IsVersionLater(LV, CV);
	}

	// Result
	if (UpdateAvailable)
	{
		if (pParentWnd)
		{
			if (LFGetApp()->m_pUpdateNotification)
				LFGetApp()->m_pUpdateNotification->DestroyWindow();

			LFUpdateDlg dlg(LatestVersion, LatestMSN, LatestFeatures, pParentWnd);
			dlg.DoModal();
		}
		else
			if (LFGetApp()->m_pUpdateNotification)
			{
				LFGetApp()->m_pUpdateNotification->SendMessage(WM_COMMAND, IDM_UPDATE_RESTORE);
			}
			else
			{
				LFGetApp()->m_pUpdateNotification = new LFUpdateDlg(LatestVersion, LatestMSN, LatestFeatures);
				LFGetApp()->m_pUpdateNotification->Create(IDD_UPDATE, CWnd::GetDesktopWindow());
				LFGetApp()->m_pUpdateNotification->ShowWindow(SW_SHOW);
			}
	}
	else
		if (Force)
		{
			CString Caption((LPCSTR)IDS_UPDATE);
			CString Text((LPCSTR)IDS_UPDATENOTAVAILABLE);

			MessageBox(pParentWnd->GetSafeHwnd(), Text, Caption, MB_ICONINFORMATION | MB_OK);
		}
}

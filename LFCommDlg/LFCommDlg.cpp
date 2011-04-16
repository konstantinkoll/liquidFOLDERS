
// LFCommDlg.cpp : Definiert die Initialisierungsroutinen für die DLL.
//

#include "stdafx.h"
#include "LFCore.h"
#include "LFCommDlg.h"


LFCommDlg_API INT GetAttributeIconIndex(UINT Attr)
{
	static const UINT IconPosition[] = { LFAttrFileName, LFAttrStoreID, LFAttrFileID, LFAttrTitle, LFAttrCreationTime,
		LFAttrAddTime, LFAttrFileTime, LFAttrRecordingTime, LFAttrDeleteTime, LFAttrDueTime, LFAttrDoneTime, LFAttrArchiveTime,
		LFAttrLocationName, LFAttrLocationIATA, LFAttrLocationGPS, LFAttrRating, LFAttrRoll, LFAttrArtist, LFAttrComment,
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
	LFApplication* pApp = (LFApplication*)AfxGetApp();

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
	LFApplication* pApp = (LFApplication*)AfxGetApp();
	if (pApp)
		if (pApp->m_ThemeLibLoaded)
			return pApp->zIsThemeActive();

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

	LFApplication* pApp = (LFApplication*)AfxGetApp();
	if (pApp)
		if (pApp->m_ThemeLibLoaded)
			if (pApp->zIsThemeActive())
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

LFCommDlg_API void LFImportFolder(CHAR* StoreID, CWnd* pParentWnd, BOOL AllowChooseStore)
{
	CString caption;
	ENSURE(caption.LoadString(IDS_IMPORTFOLDER_CAPTION));
	CString hint;
	ENSURE(hint.LoadString(IDS_IMPORTFOLDER_HINT));

	LFBrowseForFolderDlg dlg(TRUE, TRUE, _T(""), pParentWnd, caption, hint);
	if (dlg.DoModal()==IDOK)
	{
		LFFileImportList* il = LFAllocFileImportList();
		LFAddImportPath(il, dlg.m_FolderPath);

		// Template füllen
		LFItemDescriptor* it = LFAllocItemDescriptor();
		LFItemTemplateDlg tdlg(pParentWnd, it, StoreID, AllowChooseStore);
		if (tdlg.DoModal()!=IDCANCEL)
			LFErrorBox(LFImportFiles(StoreID, il, it, true, dlg.m_DeleteSource==TRUE), pParentWnd ? pParentWnd->GetSafeHwnd() : NULL);

		LFFreeItemDescriptor(it);
		LFFreeFileImportList(il);
	}
}

CString MakeHex(BYTE* x, UINT bCount)
{
	CString tmpStr;
	for (UINT a=0; a<bCount; a++)
	{
		CString digit;
		digit.Format(_T("%.2x"), x[a]);
		tmpStr += digit;
		if (a<bCount-1)
			tmpStr += _T(",");
	}
	return tmpStr;
}

void CEscape(CString &s)
{
	for (INT a = s.GetLength()-1; a>=0; a--)
		if ((s[a]==L'\\') || (s[a]==L'\"'))
			s.Insert(a, L'\\');
}

LFCommDlg_API void LFBackupStores(CWnd* pParentWnd)
{
	CHAR* Keys;
	UINT StoreCount;
	UINT Result = LFGetStores(&Keys, &StoreCount);
	if (Result!=LFOk)
	{
		LFErrorBox(Result, pParentWnd->GetSafeHwnd());
		return;
	}

	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_REGFILEFILTER));
	tmpStr += _T(" (*.reg)|*.reg||");

	CFileDialog dlg(FALSE, _T(".reg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, tmpStr, pParentWnd);

	if (dlg.DoModal()==IDOK)
	{

		CStdioFile f;
		if (!f.Open(dlg.GetFileName(), CFile::modeCreate | CFile::modeWrite))
		{
			LFErrorBox(LFDriveNotReady, pParentWnd->GetSafeHwnd());
		}
		else
		{
			try
			{
				f.WriteString(_T("Windows Registry Editor Version 5.00\n"));

				CHAR* Ptr = Keys;
				for (UINT a=0; a<StoreCount; a++)
				{
					LFStoreDescriptor s;
					if (LFGetStoreSettings(Ptr, &s)==LFOk)
						if (s.StoreMode<=LFItemCategoryHybridStores)
						{
							// Header
							tmpStr = _T("\n[HKEY_CURRENT_USER\\Software\\liquidFOLDERS\\Stores\\");
							tmpStr += s.StoreID;
							f.WriteString(tmpStr+_T("]\n"));

							// Name
							tmpStr = s.StoreName;
							CEscape(tmpStr);
							f.WriteString(_T("\"Name\"=\"")+tmpStr+_T("\"\n"));

							// Mode
							tmpStr.Format(_T("\"Mode\"=dword:%.8x\n"), s.StoreMode);
							f.WriteString(tmpStr);

							// AutoLocation
							tmpStr.Format(_T("\"AutoLocation\"=dword:%.8x\n"), s.AutoLocation);
							f.WriteString(tmpStr);

							if (!s.AutoLocation)
							{
								// Path
								tmpStr = s.DatPath;
								CEscape(tmpStr);
								f.WriteString(_T("\"Path\"=\"")+tmpStr+_T("\"\n"));
							}

							// GUID
							f.WriteString(_T("\"GUID\"=hex:")+MakeHex((BYTE*)&s.guid, sizeof(s.guid))+_T("\n"));

							// IndexVersion
							tmpStr.Format(_T("\"IndexVersion\"=dword:%.8x\n"), s.IndexVersion);
							f.WriteString(tmpStr);

							// CreationTime
							f.WriteString(_T("\"CreationTime\"=hex:")+MakeHex((BYTE*)&s.CreationTime, sizeof(s.CreationTime))+_T("\n"));

							// FileTime
							f.WriteString(_T("\"FileTime\"=hex:")+MakeHex((BYTE*)&s.FileTime, sizeof(s.FileTime))+_T("\n"));

							// MaintenanceTime
							f.WriteString(_T("\"MaintenanceTime\"=hex:")+MakeHex((BYTE*)&s.MaintenanceTime, sizeof(s.MaintenanceTime))+_T("\n"));
						}

					Ptr += LFKeySize;
				}
			}
			catch(CFileException ex)
			{
				LFErrorBox(LFDriveNotReady, pParentWnd->GetSafeHwnd());
			}

			f.Close();
		}
	}

	free(Keys);
}

void LFAbout(CString AppName, CString Build, UINT IconResID, CWnd* pParentWnd)
{
	LFAboutDlg dlg(AppName, Build, IconResID, pParentWnd);
	dlg.DoModal();
}

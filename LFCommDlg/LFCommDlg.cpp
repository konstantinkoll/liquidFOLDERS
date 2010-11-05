
// LFCommDlg.cpp : Definiert die Initialisierungsroutinen für die DLL.
//

#include "stdafx.h"
#include "LFCore.h"
#include "LFCommDlg.h"


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

		IMAGEINFO ii;
		icons->GetImageInfo(0, &ii);
		size.cx = ii.rcImage.right-ii.rcImage.left;
		size.cy = ii.rcImage.bottom-ii.rcImage.top;
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

LFCommDlg_API void LFImportFolder(CHAR* StoreID, CWnd* pParentWnd)
{
	CString caption;
	ENSURE(caption.LoadString(IDS_IMPORTFOLDER_CAPTION));
	CString hint;
	ENSURE(hint.LoadString(IDS_IMPORTFOLDER_HINT));

	LFBrowseForFolderDlg dlg(TRUE, _T(""), pParentWnd, caption, hint);
	if (dlg.DoModal()==IDOK)
	{
		LFFileImportList* il = LFAllocFileImportList();
		LFAddImportPath(il, dlg.m_FolderPath);

		// Template füllen
		LFItemDescriptor* it = LFAllocItemDescriptor();
		LFItemTemplateDlg tdlg(pParentWnd, it, StoreID);
		if (tdlg.DoModal()!=IDCANCEL)
			LFErrorBox(LFImportFiles(StoreID, il, it), pParentWnd->GetSafeHwnd());

		LFFreeItemDescriptor(it);
		LFFreeFileImportList(il);
	}
}

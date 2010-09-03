// LFCommDlg.cpp : Definiert die Initialisierungsroutinen für die DLL.
//

#include "stdafx.h"
#include "LFCore.h"
#include "LFCommDlg.h"


LFCommDlg_API void CreateRoundRectangle(CRect rect, int rad, GraphicsPath& path)
{
	path.Reset();

	int l = rect.left;
	int t = rect.top;
	int w = rect.Width();
	int h = rect.Height();
	int d = rad<<1;

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
	if (SUCCEEDED(SHGetFileInfo((wchar_t*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_LARGEICON)))
	{
		hIcon = icons->ExtractIcon(sfi.iIcon);
		caption = sfi.szDisplayName;
		hint = sfi.szTypeName;

		IShellFolder* Parent = NULL;
		LPCITEMIDLIST Child = NULL;
		if (SUCCEEDED(SHBindToParent(pidl, IID_IShellFolder, (void**)&Parent, &Child)))
		{
			WIN32_FIND_DATA ffd;
			if (SUCCEEDED(SHGetDataFromIDList(Parent, Child, SHGDFIL_FINDDATA, &ffd, sizeof(WIN32_FIND_DATA))))
			{
				FILETIME lft;

				wchar_t tmpBuf1[256];
				FileTimeToLocalFileTime(&ffd.ftCreationTime, &lft);
				LFTimeToString(lft, tmpBuf1, 256);
				wchar_t tmpBuf2[256];
				FileTimeToLocalFileTime(&ffd.ftLastWriteTime, &lft);
				LFTimeToString(lft, tmpBuf2, 256);
				
				CString tmpStr;
				tmpStr.Format(_T("\n%s: %s\n%s: %s"),
					pApp->m_Attributes[LFAttrCreationTime]->Name, tmpBuf1,
					pApp->m_Attributes[LFAttrFileTime]->Name, tmpBuf2);
				hint.Append(tmpStr);
			}
			Parent->Release();
		}

		IMAGEINFO ii;
		icons->GetImageInfo(0, &ii);
		size.cx = ii.rcImage.right-ii.rcImage.left;
		size.cy = ii.rcImage.bottom-ii.rcImage.top;
	}
}

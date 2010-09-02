
// Migrate.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "Migrate.h"
#include "MigrateWnd.h"
#include "resource.h"


// CMigrateApp

BEGIN_MESSAGE_MAP(CMigrateApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CMigrateApp-Erstellung

CMigrateApp::CMigrateApp()
	: LFApplication(HasGUI_Standard)
{
	SHGetMalloc(&p_Malloc);
}

// Das einzige CMigrateApp-Objekt

CMigrateApp theApp;


// CMigrateApp-Initialisierung

BOOL CMigrateApp::InitInstance()
{
	LFApplication::InitInstance();

	// System image list
	SHFILEINFO shfi;
	m_SystemImageListSmall.Attach((HIMAGELIST)SHGetFileInfo(_T(""), NULL, &shfi, sizeof(shfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON));
	m_SystemImageListLarge.Attach((HIMAGELIST)SHGetFileInfo(_T(""), NULL, &shfi, sizeof(shfi), SHGFI_SYSICONINDEX | SHGFI_LARGEICON));

	// Registry auslesen
	SetRegistryBase(_T("Settings"));
	m_DeleteImported = GetInt(_T("DeleteImported"), FALSE);
	m_Simulate = GetInt(_T("Simulate"), FALSE);

	m_pMainWnd = new CMigrateWnd();
	((CMigrateWnd*)m_pMainWnd)->Create();
	m_pMainWnd->ShowWindow(SW_SHOW);

	return TRUE;
}

int CMigrateApp::ExitInstance()
{
	WriteInt(_T("DeleteImported"), m_DeleteImported);
	WriteInt(_T("Simulate"), m_Simulate);

	return LFApplication::ExitInstance();
}

void CMigrateApp::OnAppAbout()
{
	LFAboutDlgParameters p;
	ENSURE(p.appname.LoadString(IDR_APPLICATION));
	p.build = __TIMESTAMP__;
	p.icon = new CGdiPlusBitmapResource();
	p.icon->Load(IDB_ABOUTICON, _T("PNG"), AfxGetResourceHandle());
	p.TextureSize = -1;
	p.RibbonColor = ID_VIEW_APPLOOK_OFF_2007_NONE;
	p.HideEmptyDrives = -1;
	p.HideEmptyDomains = -1;

	LFAboutDlg dlg(&p, m_pMainWnd);
	dlg.DoModal();

	delete p.icon;
}

LPITEMIDLIST CMigrateApp::GetNextItem(LPITEMIDLIST pidl)
{
	if (!pidl)
		return NULL;

	return (LPITEMIDLIST)(LPBYTE)(((LPBYTE)pidl)+pidl->mkid.cb);
}

UINT CMigrateApp::GetByteSize(LPITEMIDLIST pidl)
{
	if (!pidl)
		return 0;

	UINT Size = 0;
	while (pidl->mkid.cb)
	{
		Size += pidl->mkid.cb;
		pidl = GetNextItem(pidl);
	}

	return Size+sizeof(ITEMIDLIST);
}

LPITEMIDLIST CMigrateApp::Clone(LPITEMIDLIST pidl)
{
	UINT cb = GetByteSize(pidl);

	LPITEMIDLIST pidlcopy = (LPITEMIDLIST)p_Malloc->Alloc(cb);
	if (pidlcopy)
		CopyMemory(pidlcopy, pidl, cb);

	return pidlcopy;
}

LPITEMIDLIST CMigrateApp::Concat(LPITEMIDLIST left, LPITEMIDLIST right)
{
	UINT cb1 = theApp.GetByteSize(left)-sizeof(ITEMIDLIST);
	UINT cb2 = theApp.GetByteSize(right);

	LPITEMIDLIST pidlconcat = (LPITEMIDLIST)p_Malloc->Alloc(cb1+cb2);
	if (pidlconcat)
	{
		ZeroMemory(pidlconcat, cb1+cb2);
		CopyMemory(pidlconcat, left, cb1);
		CopyMemory(((LPBYTE)pidlconcat)+cb1, right, cb2);
	}

	return pidlconcat;
}

void CMigrateApp::TooltipDataFromPIDL(LPITEMIDLIST pidl, HICON& hIcon, CSize& size, CString& caption, CString& hint)
{
	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((wchar_t*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_LARGEICON)))
	{
		hIcon = theApp.m_SystemImageListLarge.ExtractIcon(sfi.iIcon);
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
					theApp.m_Attributes[LFAttrCreationTime]->Name, tmpBuf1,
					theApp.m_Attributes[LFAttrFileTime]->Name, tmpBuf2);
				hint.Append(tmpStr);
			}
			Parent->Release();
		}

		IMAGEINFO ii;
		theApp.m_SystemImageListLarge.GetImageInfo(0, &ii);
		size.cx = ii.rcImage.right-ii.rcImage.left;
		size.cy = ii.rcImage.bottom-ii.rcImage.top;
	}
}

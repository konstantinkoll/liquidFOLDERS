
HBITMAP SetMenuItemIcon(HMENU hMenu, UINT item, HICON hIcon, int cx, int cy)
{
	if (((LFApplication*)AfxGetApp())->OSVersion<OS_Vista)
		return NULL;

	HDC hDC = CreateCompatibleDC(NULL);

	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = cx;
	dib.bmiHeader.biHeight = -cy;
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP bmp = CreateDIBSection(hDC, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP pOldBitmap = (HBITMAP)SelectObject(hDC, bmp);

	DrawIconEx(hDC, 0, 0, hIcon, cx, cy, 0, NULL, DI_NORMAL);

	SelectObject(hDC, pOldBitmap);
	DeleteDC(hDC);

	MENUITEMINFO mii;
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_BITMAP;
	mii.hbmpItem = bmp;
	SetMenuItemInfo(hMenu, item, TRUE, &mii);

	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE;
	mi.dwStyle = MNS_CHECKORBMP;
	SetMenuInfo(hMenu, &mi);

	return bmp;
}

HBITMAP SetMenuItemIcon(HMENU hMenu, UINT item, WORD ResID)
{
	if (((LFApplication*)AfxGetApp())->OSVersion<OS_Vista)
		return NULL;

	int cx = GetSystemMetrics(SM_CXSMICON);
	int cy = GetSystemMetrics(SM_CYSMICON);
	HICON hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(ResID), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);

	HBITMAP res = SetMenuItemIcon(hMenu, item, hIcon, cx, cy);
	DestroyIcon(hIcon);

	return res;
}

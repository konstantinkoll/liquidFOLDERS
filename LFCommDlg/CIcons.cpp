
// CIcons.cpp: Implementierung der Klasse CIcons
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CIcons

CIcons::CIcons()
{
	hBitmap = hBitmapShadow = NULL;
}

CIcons::~CIcons()
{
	DeleteObject(hBitmap);
	DeleteObject(hBitmapShadow);
}

void CIcons::Load(UINT nID, CSize Size)
{
	ASSERT(!hBitmap);

	hBitmap = (HBITMAP)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	m_Size = Size;

	PreMultiplyAlpha(hBitmap);
	CreateShadow(hBitmap);
}

void CIcons::Load(UINT nID, INT Size)
{
	Load(nID, CSize(Size, Size));
}

void CIcons::Draw(CDC& dc, INT x, INT y, UINT nImage, BOOL Shadow) const
{
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	HBITMAP hOldBitmap;

	if (Shadow)
	{
		hOldBitmap = (HBITMAP)dcMem.SelectObject(hBitmapShadow);
		dc.AlphaBlend(x, y-1, m_Size.cx, m_Size.cy, &dcMem, nImage*m_Size.cx, 0, m_Size.cx, m_Size.cy, BF);
		dcMem.SelectObject(hBitmap);
	}
	else
	{
		hOldBitmap = (HBITMAP)dcMem.SelectObject(hBitmap);
	}

	dc.AlphaBlend(x, y, m_Size.cx, m_Size.cy, &dcMem, nImage*m_Size.cx, 0, m_Size.cx, m_Size.cy, BF);
	dcMem.SelectObject(hOldBitmap);
}

HICON CIcons::ExtractIcon(UINT nImage) const
{
	HICON hIcon = NULL;

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	HBITMAP hBitmap = CreateTransparentBitmap(m_Size.cx, m_Size.cy+1);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	Draw(dc, 0, 1, nImage, TRUE);

	dc.SelectObject(hOldBitmap);

	if (hBitmap)
	{
		ICONINFO ii;
		ZeroMemory(&ii, sizeof(ii));
		ii.fIcon = TRUE;
		ii.hbmColor = hBitmap;
		ii.hbmMask = hBitmap;

		hIcon = CreateIconIndirect(&ii);
	}

	DeleteObject(hBitmap);

	return hIcon;
}

void CIcons::PreMultiplyAlpha(HBITMAP hBitmap)
{
	BITMAP Bitmap;
	if (GetObject(hBitmap, sizeof(Bitmap), &Bitmap))
	{
		const LONG Length = Bitmap.bmWidth*Bitmap.bmHeight;
		RGBQUAD* pBits = (RGBQUAD*)Bitmap.bmBits;

		for (LONG Count=0; Count<Length; Count++)
		{
			pBits->rgbRed = (BYTE)(pBits->rgbRed*pBits->rgbReserved/255);
			pBits->rgbGreen = (BYTE)(pBits->rgbGreen*pBits->rgbReserved/255);
			pBits->rgbBlue = (BYTE)(pBits->rgbBlue*pBits->rgbReserved/255);
			pBits++;
		}
	}
}

void CIcons::CreateShadow(HBITMAP hBitmap)
{
	BITMAP Bitmap;
	if (GetObject(hBitmap, sizeof(Bitmap), &Bitmap))
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		hBitmapShadow = CreateTransparentBitmap(Bitmap.bmWidth, Bitmap.bmHeight);
		HBITMAP hOldBitmap1 = (HBITMAP)dc.SelectObject(hBitmapShadow);

		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);

		HBITMAP hOldBitmap2 = (HBITMAP)dcMem.SelectObject(hBitmap);
		dc.AlphaBlend(0, 0, Bitmap.bmWidth, Bitmap.bmHeight, &dcMem, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, BF);
		dcMem.SelectObject(hOldBitmap2);

		dc.SelectObject(hOldBitmap1);
		
		if (GetObject(hBitmapShadow, sizeof(Bitmap), &Bitmap))
		{
			const LONG Length = Bitmap.bmWidth*Bitmap.bmHeight;
			COLORREF* Ptr = (COLORREF*)Bitmap.bmBits;

			for (LONG Count=0; Count<Length; Count++)
			{
				*Ptr &= 0xFF000000;
				Ptr++;
			}
		}
	}
}

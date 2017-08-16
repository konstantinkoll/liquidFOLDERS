
// CIcons.cpp: Implementierung der Klasse CIcons
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include <math.h>


// CIcons

#define HOTGAMMADARK      1.3
#define HOTGAMMALIGHT     0.75

const INT CIcons::m_IconSizes[] = { 16, 24, 32, 48, 64 };
BOOL CIcons::m_GammaTableCreated = FALSE;
BYTE CIcons::m_GammaTableDarkBackground[256];
BYTE CIcons::m_GammaTableLightBackground[256];

CIcons::CIcons()
{
	hBitmapNormal = hBitmapShadow = hBitmapHot = hBitmapDisabled = NULL;
	m_MaxIcons = m_IconCount = 0;
	m_UseDarkBackgroundGamma = FALSE;
}

CIcons::~CIcons()
{
	DeleteObject(hBitmapNormal);
	DeleteObject(hBitmapShadow);
	DeleteObject(hBitmapHot);
	DeleteObject(hBitmapDisabled);
}

void CIcons::Load(UINT nID, CSize Size)
{
	ASSERT(Size.cx>0);
	ASSERT(Size.cy>0);

	if (!hBitmapNormal)
	{
		Bitmap* pIcons = LFGetApp()->GetResourceImage(nID);
		if (pIcons)
		{
			hBitmapNormal = CreateTransparentBitmap(pIcons->GetWidth(), pIcons->GetHeight());

			CDC dc;
			dc.CreateCompatibleDC(NULL);

			HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmapNormal);

			Graphics g(dc);
			g.DrawImage(pIcons, 0, 0);

			dc.SelectObject(hOldBitmap);

			m_Size = Size;
			m_MaxIcons = m_IconCount = pIcons->GetWidth()/Size.cx;

			delete pIcons;
		}
	}
}

void CIcons::Load(UINT nID, INT Size)
{
	ASSERT(Size>0);

	Load(nID, CSize(Size, Size));
}

INT CIcons::Load(UINT nID, UINT Flags, LFFont* pFont)
{
	if (!pFont)
		pFont = (Flags>=LI_SLIGHTLYLARGER) ? &LFGetApp()->m_LargeFont : &LFGetApp()->m_DefaultFont;

	const INT Height = pFont->GetFontHeight();

	UINT Level = (Height>=32) ? 2 : (Height>=24) ? 1 : 0;
	if (Flags==LI_FORTOOLTIPS)
		Level += 2;

#ifdef _DEBUG
	// Increase icon size in debug mode for testing
	if (Flags>=LI_SLIGHTLYLARGER)
		Level++;
#endif

	Load(nID+Level, m_IconSizes[Level]);

	return m_IconSizes[Level];
}

INT CIcons::LoadSmall(UINT nID)
{
	INT Size = GetSystemMetrics(SM_CXSMICON);
	ASSERT(Size==GetSystemMetrics(SM_CYSMICON));

	INT Level = (Size>=32) ? 2 : (Size>=24) ? 1 : 0;

	Load(nID+Level, m_IconSizes[Level]);

	return m_IconSizes[Level];
}


void CIcons::Create(const CSize& Size, UINT MaxIcons)
{
	ASSERT(Size.cx>0);
	ASSERT(Size.cy>0);

	if (!hBitmapNormal)
	{
		hBitmapNormal = CreateTransparentBitmap(Size.cx*MaxIcons, Size.cy);

		m_Size = Size;
		m_MaxIcons = MaxIcons;
	}
}

void CIcons::Create(const CImageList& ImageList, UINT MaxIcons)
{
	INT cx;
	INT cy;

	if (ImageList_GetIconSize(ImageList, &cx, &cy))
		Create(CSize(cx, cy), MaxIcons);
}

INT CIcons::GetIconSize() const
{
	return m_Size.cy;
}

INT CIcons::AddIcon(HICON hIcon)
{
	ASSERT(hIcon);

	if (hBitmapNormal && (m_IconCount<m_MaxIcons))
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmapNormal);

		DrawIconEx(dc, m_IconCount*m_Size.cx, 0, hIcon, m_Size.cx, m_Size.cy, 0, NULL, DI_NORMAL);

		dc.SelectObject(hOldBitmap);

		return m_IconCount++;
	}

	return -1;
}

INT CIcons::AddIcon(CImageList& ImageList, INT nImage)
{
	if (hBitmapNormal && (m_IconCount<m_MaxIcons))
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmapNormal);

		ImageList.Draw(&dc, nImage, CPoint(m_IconCount*m_Size.cx, 0), ILD_TRANSPARENT);

		dc.SelectObject(hOldBitmap);

		return m_IconCount++;
	}

	return -1;
}

void CIcons::Draw(CDC& dc, INT x, INT y, INT nImage, BOOL Hot, BOOL Disabled, BOOL Shadow)
{
	ASSERT(nImage<(INT)m_IconCount);

	if (nImage>=0)
	{
		if (Shadow)
			CreateIconsShadow();

		if (Hot)
			CreateIconsHot();

		if (Disabled)
			CreateIconsDisabled();

		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);

		HBITMAP hOldBitmap;

		if (Shadow)
		{
			hOldBitmap = (HBITMAP)dcMem.SelectObject(hBitmapShadow);
			dc.AlphaBlend(x, y-1, m_Size.cx, m_Size.cy, &dcMem, nImage*m_Size.cx, 0, m_Size.cx, m_Size.cy, BF);
			dcMem.SelectObject(Hot ? hBitmapHot : Disabled ? hBitmapDisabled : hBitmapNormal);
		}
		else
		{
			hOldBitmap = (HBITMAP)dcMem.SelectObject(Disabled ? hBitmapDisabled : Hot ? hBitmapHot : hBitmapNormal);
		}

		dc.AlphaBlend(x, y, m_Size.cx, m_Size.cy, &dcMem, nImage*m_Size.cx, 0, m_Size.cx, m_Size.cy, BF);
		dcMem.SelectObject(hOldBitmap);
	}
}

HBITMAP CIcons::ExtractBitmap(CImageList& ImageList, INT nImage)
{
	INT cx = 128;
	INT cy = 128;
	ImageList_GetIconSize(ImageList, &cx, &cy);

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	HBITMAP hBitmap = CreateTransparentBitmap(cx, cy);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	ImageList.DrawEx(&dc, nImage, CPoint(0, 0), CSize(cx, cy), CLR_NONE, 0xFFFFFF, ILD_TRANSPARENT);

	dc.SelectObject(hOldBitmap);

	return hBitmap;
}

HBITMAP CIcons::ExtractBitmap(INT nImage, BOOL Shadow)
{
	ASSERT(nImage<(INT)m_IconCount);

	HBITMAP hBitmap = NULL;

	if (nImage>=0)
	{
		if (Shadow)
			CreateIconsShadow();

		CDC dc;
		dc.CreateCompatibleDC(NULL);

		hBitmap = CreateTransparentBitmap(m_Size.cx, m_Size.cy+(Shadow ? 1 : 0));
		HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

		Draw(dc, 0, Shadow ? 1 : 0, nImage, FALSE, FALSE, Shadow);

		dc.SelectObject(hOldBitmap);
	}

	return hBitmap;
}

HICON CIcons::ExtractIcon(INT nImage, BOOL Shadow)
{
	ASSERT(nImage<(INT)m_IconCount);

	HICON hIcon = NULL;

	if (nImage>=0)
	{
		HBITMAP hBitmap = ExtractBitmap(nImage, Shadow);
		if (hBitmap)
		{
			ICONINFO ii;
			ZeroMemory(&ii, sizeof(ii));
			ii.fIcon = TRUE;
			ii.hbmColor = hBitmap;
			ii.hbmMask = hBitmap;

			hIcon = CreateIconIndirect(&ii);

			DeleteObject(hBitmap);
		}
	}

	return hIcon;
}

HIMAGELIST CIcons::ExtractImageList() const
{
	HIMAGELIST hImageList = NULL;

	if (hBitmapNormal)
	{
		hImageList = ImageList_Create(m_Size.cx, m_Size.cy, ILC_COLOR32, 0, 1);
		ImageList_Add(hImageList, hBitmapNormal, NULL);
	}

	return hImageList;
}

HBITMAP CIcons::CreateCopy()
{
	HBITMAP hBitmapCopy = NULL;

	BITMAP Bitmap;
	if (GetObject(hBitmapNormal, sizeof(Bitmap), &Bitmap))
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		hBitmapCopy = CreateTransparentBitmap(Bitmap.bmWidth, Bitmap.bmHeight);
		HBITMAP hOldBitmap1 = (HBITMAP)dc.SelectObject(hBitmapCopy);

		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);

		HBITMAP hOldBitmap2 = (HBITMAP)dcMem.SelectObject(hBitmapNormal);
		dc.AlphaBlend(0, 0, Bitmap.bmWidth, Bitmap.bmHeight, &dcMem, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, BF);
		dcMem.SelectObject(hOldBitmap2);

		dc.SelectObject(hOldBitmap1);
	}

	return hBitmapCopy;
}

void CIcons::CreateIconsShadow()
{
	ASSERT(hBitmapNormal);

	if (!hBitmapShadow)
	{
		hBitmapShadow = CreateCopy();
		ASSERT(hBitmapShadow);

		BITMAP Bitmap;
		if (GetObject(hBitmapShadow, sizeof(Bitmap), &Bitmap))
		{
			const LONG Length = Bitmap.bmWidth*Bitmap.bmHeight;
			COLORREF* Ptr = (COLORREF*)Bitmap.bmBits;

			for (LONG Count=0; Count<Length; Count++)
				*(Ptr++) &= 0xFF000000;
		}
	}
}

void CIcons::CreateIconsHot()
{
	ASSERT(hBitmapNormal);

	if (!hBitmapHot)
	{
		// Gamma table
		if (!m_GammaTableCreated)
		{
			m_GammaTableDarkBackground[0x00] = m_GammaTableLightBackground[0x00] = 0x00;
			m_GammaTableDarkBackground[0xFF] = m_GammaTableLightBackground[0xFF] = 0xFF;

			for (BYTE a=1; a<=254; a++)
			{
				m_GammaTableDarkBackground[a] = (BYTE)(255.0*pow(a/255.0, 1/HOTGAMMADARK));
				m_GammaTableLightBackground[a] = (BYTE)(255.0*pow(a/255.0, 1/HOTGAMMALIGHT));
			}
		}

		// Applay gamma
		hBitmapHot = CreateCopy();
		ASSERT(hBitmapHot);

		BITMAP Bitmap;
		if (GetObject(hBitmapHot, sizeof(Bitmap), &Bitmap))
		{
			const LONG Length = Bitmap.bmWidth*Bitmap.bmHeight;
			RGBQUAD* Ptr = (RGBQUAD*)Bitmap.bmBits;
			const LPBYTE pGammaTable = m_UseDarkBackgroundGamma ? m_GammaTableDarkBackground : m_GammaTableLightBackground;

			for (LONG Count=0; Count<Length; Count++)
			{
				Ptr->rgbBlue = pGammaTable[Ptr->rgbBlue];
				Ptr->rgbGreen = pGammaTable[Ptr->rgbGreen];
				Ptr->rgbRed = pGammaTable[Ptr->rgbRed];

				Ptr++;
			}
		}
	}
}

void CIcons::CreateIconsDisabled()
{
	ASSERT(hBitmapNormal);

	if (!hBitmapDisabled)
	{
		hBitmapDisabled = CreateCopy();
		ASSERT(hBitmapDisabled);

		BITMAP Bitmap;
		if (GetObject(hBitmapDisabled, sizeof(Bitmap), &Bitmap))
		{
			const LONG Length = Bitmap.bmWidth*Bitmap.bmHeight;
			RGBQUAD* Ptr = (RGBQUAD*)Bitmap.bmBits;

			for (LONG Count=0; Count<Length; Count++)
			{
				const UINT Value = Ptr->rgbBlue+4*Ptr->rgbRed+5*Ptr->rgbGreen;

				if (m_UseDarkBackgroundGamma)
				{
					Ptr->rgbBlue = (BYTE)(Value*0x99/2550);
					Ptr->rgbGreen = (BYTE)(Value*0x89/2550);
					Ptr->rgbRed = (BYTE)(Value*0x81/2550);
				}
				else
				{
					Ptr->rgbBlue = Ptr->rgbGreen = Ptr->rgbRed = (BYTE)(Value/30);
					Ptr->rgbReserved /= 3;
				}

				Ptr++;
			}
		}
	}
}


// CIcons.cpp: Implementierung der Klasse CIcons
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CIcons

CIcons::CIcons()
{
	hBitmap = hBitmapShadow = NULL;
	m_MaxIcons = m_IconCount = 0;
}

CIcons::~CIcons()
{
	DeleteObject(hBitmap);
	DeleteObject(hBitmapShadow);
}

void CIcons::Load(UINT nID, CSize Size)
{
	ASSERT(Size.cx>0);
	ASSERT(Size.cy>0);

	if (!hBitmap)
	{
		Bitmap* pIcons = LFGetApp()->GetResourceImage(nID);
		if (pIcons)
		{
			hBitmap = CreateTransparentBitmap(pIcons->GetWidth(), pIcons->GetHeight());

			CDC dc;
			dc.CreateCompatibleDC(NULL);

			HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

			Graphics g(dc);
			g.DrawImage(pIcons, 0, 0);

			dc.SelectObject(hOldBitmap);

			m_Size = Size;
			m_MaxIcons = m_IconCount = pIcons->GetWidth()/Size.cx;

			delete pIcons;

			Finish();
		}
	}
}

void CIcons::Load(UINT nID, INT Size)
{
	ASSERT(Size>0);

	Load(nID, CSize(Size, Size));
}

void CIcons::Create(CSize Size, UINT MaxIcons)
{
	ASSERT(Size.cx>0);
	ASSERT(Size.cy>0);

	if (!hBitmap)
	{
		hBitmap = CreateTransparentBitmap(Size.cx*MaxIcons, Size.cy);

		m_Size = Size;
		m_MaxIcons = MaxIcons;
	}
}

void CIcons::Create(CImageList& ImageList, UINT MaxIcons)
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

	if (hBitmap && (m_IconCount<m_MaxIcons))
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

		DrawIconEx(dc, m_IconCount*m_Size.cx, 0, hIcon, m_Size.cx, m_Size.cy, 0, NULL, DI_NORMAL);

		dc.SelectObject(hOldBitmap);

		return m_IconCount++;
	}

	return -1;
}

INT CIcons::AddIcon(CImageList& ImageList, INT nImage)
{
	if (hBitmap && (m_IconCount<m_MaxIcons))
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

		ImageList.Draw(&dc, nImage, CPoint(m_IconCount*m_Size.cx, 0), ILD_TRANSPARENT);

		dc.SelectObject(hOldBitmap);

		return m_IconCount++;
	}

	return -1;
}

void CIcons::Draw(CDC& dc, INT x, INT y, INT nImage, BOOL Shadow)
{
	ASSERT(nImage<(INT)m_IconCount);

	if (nImage>=0)
	{
		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);

		HBITMAP hOldBitmap;

		if (Shadow)
		{
			Finish();

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
}

HICON CIcons::ExtractIcon(INT nImage)
{
	ASSERT(nImage<(INT)m_IconCount);

	HICON hIcon = NULL;

	if (nImage>=0)
	{
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
	}

	return hIcon;
}

HIMAGELIST CIcons::ExtractImageList() const
{
	HIMAGELIST hImageList = NULL;

	if (hBitmap)
	{
		hImageList = ImageList_Create(m_Size.cx, m_Size.cy, ILC_COLOR32, 0, 1);
		ImageList_Add(hImageList, hBitmap, NULL);
	}

	return hImageList;
}

void CIcons::CreateShadow()
{
	ASSERT(hBitmap);

	if (!hBitmapShadow)
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
}

__forceinline void CIcons::Finish()
{
	CreateShadow();
}

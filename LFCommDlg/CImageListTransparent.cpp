
// CImageListTransparent.cpp: Implementierung der Klasse CImageListTransparent
//

#include "stdafx.h"
#include "CImageListTransparent.h"


// CImageListTransparent

void CImageListTransparent::Create(UINT nID, INT cx, INT cy)
{
	CBitmap Bitmap;
	ENSURE(Bitmap.LoadBitmap(nID));

	CImageList::Create(cx, cy, ILC_COLOR32, 0, 1);
	Add(&Bitmap, 0xFF00FF);
}

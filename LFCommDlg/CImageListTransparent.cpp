
// CImageListTransparent.cpp: Implementierung der Klasse CImageListTransparent
//

#include "stdafx.h"
#include "CImageListTransparent.h"


// CImageListTransparent

void CImageListTransparent::Create(UINT nID, INT cx, INT cy)
{
	CBitmap bmp;
	ENSURE(bmp.LoadBitmap(nID));

	CImageList::Create(cx, cy, ILC_COLOR32, 0, 1);
	Add(&bmp, 0xFF00FF);
}

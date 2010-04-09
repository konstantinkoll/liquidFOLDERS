
// CImageListTransparent.cpp: Implementierung der Klasse CImageListTransparent
//

#include "stdafx.h"
#include "CImageListTransparent.h"


// CImageListTransparent

CImageListTransparent::CImageListTransparent()
	: CImageList()
{
}

CImageListTransparent::~CImageListTransparent()
{
}

void CImageListTransparent::CreateFromResource(UINT ID, UINT first, UINT last, int cx, int cy)
{
	Create(cx, cy, ILC_COLOR32, last-first+1, 4);
	AddFromResource(ID, first, last, cx, cy);
}

void CImageListTransparent::AddFromResource(UINT ID, UINT first, UINT last, int cx, int cy)
{
	CMFCToolBarImages tmp;
	tmp.SetImageSize(CSize(cx, cy));
	tmp.Load(ID);
		for (UINT a=first; a<=last; a++)
		{
			HICON h = tmp.ExtractIcon(a);
			Add(h);
			DestroyIcon(h);
		}
}


// CImageListTransparent.cpp: Implementierung der Klasse CImageListTransparent
//

#include "stdafx.h"
#include "CImageListTransparent.h"


// CImageListTransparent

CImageListTransparent::CImageListTransparent()
	: CImageList()
{
}

void CImageListTransparent::Create(UINT ID, HINSTANCE hinstRes, UINT first, int last, int cx, int cy)
{
	CMFCToolBarImages tmp;
	tmp.SetImageSize(CSize(cx, cy));
	tmp.Load(ID, hinstRes);

	if (last==-1)
		last = tmp.GetCount()-1;

	CImageList::Create(cx, cy, ILC_COLOR32, last-first+1, 4);

		for (UINT a=first; (int)a<=last; a++)
		{
			HICON h = tmp.ExtractIcon(a);
			Add(h);
			DestroyIcon(h);
		}
}

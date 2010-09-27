
// CImageListTransparent.h: Schnittstelle der Klasse CImageListTransparent
//

#pragma once


// CImageListTransparent

class AFX_EXT_CLASS CImageListTransparent : public CImageList
{
public:
	CImageListTransparent();

	void Create(UINT ID, HINSTANCE hinstRes=NULL, UINT first=0, int last=-1, int cx=16, int cy=16);
};

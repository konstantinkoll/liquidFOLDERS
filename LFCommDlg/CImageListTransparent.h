
// CImageListTransparent.h: Schnittstelle der Klasse CImageListTransparent
//

#pragma once


// CImageListTransparent

class AFX_EXT_CLASS CImageListTransparent : public CImageList
{
public:
	CImageListTransparent();

	void Create(UINT ID, HINSTANCE hinstRes=NULL, UINT first=0, INT last=-1, INT cx=16, INT cy=16);
};

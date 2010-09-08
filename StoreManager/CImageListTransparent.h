
// CImageListTransparent.h: Schnittstelle der Klasse CImageListTransparent
//

#pragma once


// CImageListTransparent

class CImageListTransparent : public CImageList
{
public:
	CImageListTransparent();

	void CreateFromResource(UINT ID, UINT first, UINT last, int cx=16, int cy=16);
	void AddFromResource(UINT ID, UINT first, UINT last, int cx=16, int cy=16);
};

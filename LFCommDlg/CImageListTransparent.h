
// CImageListTransparent.h: Schnittstelle der Klasse CImageListTransparent
//

#pragma once


// CImageListTransparent

class CImageListTransparent : public CImageList
{
public:
	CImageListTransparent();

	void Create(UINT ID, INT cx=16, INT cy=16);
};

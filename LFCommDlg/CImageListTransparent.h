
// CImageListTransparent.h: Schnittstelle der Klasse CImageListTransparent
//

#pragma once


// CImageListTransparent

class AFX_EXT_CLASS CImageListTransparent : public CImageList
{
public:
	CImageListTransparent();
	virtual ~CImageListTransparent();

	void CreateFromResource(UINT ID, UINT first, UINT last, int cx=16, int cy=16);
	void AddFromResource(UINT ID, UINT first, UINT last, int cx=16, int cy=16);
};

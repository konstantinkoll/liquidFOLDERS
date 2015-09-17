
// CIcons.h: Schnittstelle der Klasse CIcons
//

#pragma once


// CIcons

class CIcons
{
public:
	CIcons();
	~CIcons();

	void Load(UINT nID, CSize Size);
	void Load(UINT nID, INT Size=16);
	void Draw(CDC& dc, INT x, INT y, UINT nImage, BOOL Shadow=FALSE);
	HICON ExtractIcon(UINT nImage);

protected:
	void PreMultiplyAlpha(HBITMAP hBitmap);
	void CreateShadows(HBITMAP hBitmap);

	HBITMAP hBitmap;
	HBITMAP hBitmapShadow;
	CSize m_Size;
	INT m_ShadowSize;
};

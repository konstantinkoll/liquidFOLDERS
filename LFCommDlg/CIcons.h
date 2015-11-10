
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
	void Draw(CDC& dc, INT x, INT y, UINT nImage, BOOL Shadow=FALSE) const;
	HICON ExtractIcon(UINT nImage) const;

protected:
	void PreMultiplyAlpha(HBITMAP hBitmap);
	void CreateShadow(HBITMAP hBitmap);

	HBITMAP hBitmap;
	HBITMAP hBitmapShadow;
	CSize m_Size;
};

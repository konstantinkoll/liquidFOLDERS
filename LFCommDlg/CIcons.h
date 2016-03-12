
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
	void Create(CSize Size, UINT MaxIcons);
	void Create(CImageList& ImageList, UINT MaxIcons);
	INT GetIconSize() const;
	INT AddIcon(HICON hIcon);
	INT AddIcon(CImageList& ImageList, INT nImage);
	void Draw(CDC& dc, INT x, INT y, INT nImage, BOOL Shadow=FALSE);
	HICON ExtractIcon(INT nImage, BOOL Shadow=FALSE);
	HIMAGELIST ExtractImageList() const;

protected:
	HBITMAP hBitmap;
	HBITMAP hBitmapShadow;
	CSize m_Size;

private:
	void CreateShadow();
	void Finish();

	UINT m_IconCount;
	UINT m_MaxIcons;
};

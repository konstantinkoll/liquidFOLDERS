
// GLFont.h: Schnittstelle der Klasse GLFont
//

#pragma once


// GLFont
//

class GLFont
{
public:
	GLFont();
	~GLFont();

	BOOL Create(CFont* pFont);
	UINT Render(CHAR* pStr, INT x, INT y, SIZE_T cCount=-1) const;
	UINT Render(WCHAR* pStr, INT x, INT y, SIZE_T cCount=-1) const;
	UINT GetTextWidth(CHAR* pStr, SIZE_T cCount=-1) const;
	UINT GetTextWidth(WCHAR* pStr, SIZE_T cCount=-1) const;
	UINT GetTextHeight(void* pStr) const;

protected:
	UINT RenderChar(UCHAR Ch, INT x, INT y, UINT& Height) const;

private:
	GLfloat TexCoords[256-32][4];
	UINT m_Spacing;
	UINT m_LineHeight;
	UINT m_TexSize;
	UINT m_TexID;

	enum PAINTRESULT
	{
		FAIL,
		MOREDATA,
		SUCCESS
	};
	PAINTRESULT PaintAlphabet(HDC hDC, BOOL bMeasureOnly=FALSE);
};

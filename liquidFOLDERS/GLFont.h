
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

	BOOL Create(CString Face, UINT Size, BOOL Bold=FALSE, BOOL Italic=FALSE);
	BOOL Create(CFont* pFont);
	UINT Render(CHAR* pStr, INT x, INT y, SIZE_T cCount=-1);
	UINT Render(WCHAR* pStr, INT x, INT y, SIZE_T cCount=-1);
	UINT GetTextWidth(CHAR* pStr, SIZE_T cCount=-1);
	UINT GetTextWidth(WCHAR* pStr, SIZE_T cCount=-1);
	UINT GetTextHeight(void* pStr);

protected:
	UINT RenderChar(UCHAR Ch, INT x, INT y, UINT& Height);

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

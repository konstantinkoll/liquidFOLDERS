
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

	BOOL Create(CString Face, UINT Size, BOOL Bold, BOOL Italic);
	BOOL Create(CFont* Font);
	UINT Render(CHAR* pStr, INT x, INT y, INT cCount=-1);
	UINT Render(WCHAR* pStr, INT x, INT y, INT cCount=-1);
	UINT GetTextWidth(CHAR* pStr, INT cCount=-1);
	UINT GetTextWidth(WCHAR* pStr, INT cCount=-1);
	UINT GetTextHeight(void* pStr);

protected:
	BOOL Initialize(HFONT hFont);
	UINT RenderChar(UCHAR ch, INT x, INT y, UINT* pHeight);

private:
	GLfloat TexCoords[256-32][4];
	UINT m_Spacing;
	UINT m_LineHeight;
	UINT m_TexSize;
	UINT m_TexID;

	enum PAINTRESULT
	{
		Fail,
		MoreData,
		Success
	};
	PAINTRESULT PaintAlphabet(HDC hDC, BOOL bMeasureOnly=FALSE);
};
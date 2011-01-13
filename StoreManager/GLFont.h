
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

	BOOL Create(CString face, UINT size, BOOL bold, BOOL italic);
	BOOL Create(CFont* font);
	UINT Render(WCHAR* pStr, INT x, INT y, INT cCount=-1);
	UINT GetTextWidth(WCHAR* pStr, INT cCount=-1);
	UINT GetTextHeight(WCHAR* pStr);

protected:
	BOOL Initialize(HFONT hFont);
	UINT RenderChar(UCHAR ch, INT x, INT y, UINT* pHeight);

private:
	GLfloat TexCoords[256-32][4];
	UINT m_Spacing;
	UINT m_LineHeight;
	UINT m_TexSize;
	UINT m_TexID;

	enum PaintResult
	{
		Fail,
		MoreData,
		Success
	};
	PaintResult PaintAlphabet(HDC hDC, BOOL bMeasureOnly=FALSE);
};

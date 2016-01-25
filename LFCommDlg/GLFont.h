
// GLFont.h: Schnittstelle der Klasse GLFont
//

#pragma once
#include "GLRenderer.h"
#include <GL/gl.h>
#include <GL/glu.h>


// GLFont
//

class GLFont
{
public:
	GLFont();
	~GLFont();

	BOOL Create(CFont* pFont);
	void Begin(const GLcolor& SrcColor, GLfloat Alpha=1.0f) const;
	void SetColor(const GLcolor& SrcColor, GLfloat Alpha=1.0f) const;
	static void End();
	UINT Render(CHAR* pStr, INT x, INT y, SIZE_T cCount=-1) const;
	UINT Render(WCHAR* pStr, INT x, INT y, SIZE_T cCount=-1) const;
	UINT GetTextWidth(CHAR* pStr, SIZE_T cCount=-1) const;
	UINT GetTextWidth(WCHAR* pStr, SIZE_T cCount=-1) const;
	UINT GetTextHeight(void* pStr) const;

protected:
	UINT RenderChar(UCHAR Ch, INT x, INT y, UINT& LineHeight) const;

private:
	GLfloat TexCoords[256-32][4];
	UINT m_Spacing;
	UINT m_LineHeight;
	UINT m_TextureSize;
	UINT m_TextureID;

	enum PAINTRESULT
	{
		FAIL,
		MOREDATA,
		SUCCESS
	};
	PAINTRESULT PaintAlphabet(HDC hDC, BOOL bMeasureOnly=FALSE);
};

inline void GLFont::SetColor(const GLcolor& SrcColor, GLfloat Alpha) const
{
	theRenderer.SetColor(SrcColor, Alpha);
}

inline UINT GLFont::GetTextHeight(void* pStr) const
{
	return (pStr ? m_LineHeight : 0);
}

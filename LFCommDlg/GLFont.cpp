
// GLFont.cpp: Implementierung der Klasse GLFont
//

#pragma once
#include "stdafx.h"
#include "LFCommDlg.h"
#include <math.h>


// GLFont
//

GLFont::GLFont()
{
	m_TextureID = m_LineHeight = 0;
	m_TextureSize = 256;
}

GLFont::~GLFont()
{
	if (m_TextureID)
		glDeleteTextures(1, &m_TextureID);
}

BOOL GLFont::Create(CFont* pFont)
{
	ASSERT(pFont);

	CDC dc;
	dc.CreateCompatibleDC(NULL);
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(0xFFFFFF);

	CFont* pOldFont = dc.SelectObject(pFont);

	PAINTRESULT Result;
	while (MOREDATA==(Result=PaintAlphabet(dc, TRUE)))
		m_TextureSize *= 2;

	if (Result==SUCCESS)
	{
		HBITMAP hBitmap = CreateTransparentBitmap(m_TextureSize, m_TextureSize);
		HGDIOBJ hOldBitmap = dc.SelectObject(hBitmap);

		Result = PaintAlphabet(dc);
		if (Result==SUCCESS)
		{
			BITMAP Bitmap;
			GetObject(hBitmap, sizeof(BITMAP), &Bitmap);

			BYTE* PtrSrc = (BYTE*)Bitmap.bmBits;
			BYTE* PtrDst = (BYTE*)Bitmap.bmBits;
			UINT Size = m_TextureSize*m_TextureSize;

			for (UINT a=0; a<Size; a++)
			{
				*PtrDst = (PtrSrc[0]+PtrSrc[1]+PtrSrc[2]*2)/4;

				PtrSrc += 4;
				PtrDst++;
			}

			glGenTextures(1, &m_TextureID);
			glBindTexture(GL_TEXTURE_2D, m_TextureID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, m_TextureSize, m_TextureSize, 0, GL_ALPHA, GL_UNSIGNED_BYTE, Bitmap.bmBits);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}

		dc.SelectObject(hOldBitmap);
		DeleteObject(hBitmap);
	}

	dc.SelectObject(pOldFont);

	return Result==SUCCESS;
}

void GLFont::Begin(const GLcolor& SrcColor, GLfloat Alpha) const
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBegin(GL_QUADS);

	theRenderer.SetColor(SrcColor, Alpha);
}

void GLFont::End()
{
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

UINT GLFont::Render(CHAR* pStr, INT x, INT y, SIZE_T cCount) const
{
	if (!pStr)
		return 0;

	if (cCount<0)
		cCount = MAXINT;

	UINT Height = 0;

	while ((cCount-->0) && (*pStr))
	{
		UCHAR Ch = *pStr++;

		x += (Ch==' ') ? m_Spacing : (Ch>32) ? RenderChar(Ch-32, x, y, Height) : 0;
	}

	return Height;
}

UINT GLFont::Render(WCHAR* pStr, INT x, INT y, SIZE_T cCount) const
{
	if (!pStr)
		return 0;

	if (cCount<0)
		cCount = MAXINT;

	UINT Height = 0;

	while ((cCount-->0) && (*pStr))
	{
		WCHAR Ch = *pStr++;

		switch (Ch)
		{
		case 2013:
		case 8211:
			Ch = 150;
			break;

		case 2014:
		case 8212:
			Ch = 151;
			break;
		}

		x += (Ch==L' ') ? m_Spacing : (Ch>32) && (Ch<256) ? RenderChar((UCHAR)Ch-32, x, y, Height) : 0;
	}

	return Height;
}

UINT GLFont::RenderChar(UCHAR Ch, INT x, INT y, UINT& LineHeight) const
{
	GLfloat s1 = TexCoords[Ch][0];
	GLfloat t1 = TexCoords[Ch][1];
	GLfloat s2 = TexCoords[Ch][2];
	GLfloat t2 = TexCoords[Ch][3];

	UINT Width = (INT)((s2-s1)*m_TextureSize);
	UINT Height = (INT)((t2-t1)*m_TextureSize);

	glTexCoord2f(s1, t2);
	glVertex2i(x, y+Height);
	glTexCoord2f(s2, t2);
	glVertex2i(x+Width, y+Height);
	glTexCoord2f(s2, t1);
	glVertex2i(x+Width, y);
	glTexCoord2f(s1, t1);
	glVertex2i(x, y);

	if (Height>LineHeight)
		LineHeight = Height;

	return Width;
}

UINT GLFont::GetTextWidth(CHAR* pStr, SIZE_T cCount) const
{
	if (!pStr)
		return 0;

	if (cCount<0)
		cCount = MAXINT;

	GLfloat Width = 0;

	while ((cCount-->0) && (*pStr))
	{
		UCHAR Ch = *pStr++;

		Width += (Ch==' ') ? m_Spacing : (Ch>32) ? (TexCoords[Ch-32][2]-TexCoords[Ch-32][0])*m_TextureSize : 0;
	}

	return (UINT)Width;
}

UINT GLFont::GetTextWidth(WCHAR* pStr, SIZE_T cCount) const
{
	if (!pStr)
		return 0;

	if (cCount<0)
		cCount = MAXINT;

	GLfloat Width = 0;

	while ((cCount-->0) && (*pStr))
	{
		WCHAR Ch = *pStr++;

		switch (Ch)
		{
		case 2013:
		case 8211:
			Ch = 150;
			break;

		case 2014:
		case 8212:
			Ch = 151;
			break;
		}

		Width += (Ch==L' ') ? m_Spacing : (Ch>32) && (Ch<256) ? (TexCoords[Ch-32][2]-TexCoords[Ch-32][0])*m_TextureSize : 0;
	}

	return (UINT)Width;
}

GLFont::PAINTRESULT GLFont::PaintAlphabet(HDC hDC, BOOL bMeasureOnly)
{
	SIZE Size;
	CHAR Str[2] = "?";

	if (!GetTextExtentPoint32A(hDC, Str, 1, &Size))
		return FAIL;

	m_Spacing = (INT)ceil((DOUBLE)Size.cy/3);
	m_LineHeight = max(m_LineHeight, (UINT)Size.cy);

	INT x = m_Spacing;
	INT y = 0;

	for (UCHAR Ch=32; Ch<255; Ch++)
	{
		Str[0] = Ch;
		if (!GetTextExtentPoint32A(hDC, Str, 1, &Size))
			continue;

		if (x+Size.cx+m_Spacing>m_TextureSize)
		{
			x = m_Spacing;
			y += Size.cy+1;
		}

		if (y+Size.cy>(INT)m_TextureSize)
			return MOREDATA;

		if (!bMeasureOnly)
		{
			if (!ExtTextOutA(hDC, x, y, ETO_OPAQUE, NULL, Str, 1, NULL))
				continue;

			TexCoords[Ch-32][0] = ((GLfloat)(x))/m_TextureSize;
			TexCoords[Ch-32][1] = ((GLfloat)(y))/m_TextureSize;
			TexCoords[Ch-32][2] = ((GLfloat)(x+Size.cx))/m_TextureSize;
			TexCoords[Ch-32][3] = ((GLfloat)(y+Size.cy))/m_TextureSize;
		}

		x += Size.cx+2;
	}

	return SUCCESS;
}


// GLFont.cpp: Implementierung der Klasse GLFont
//

#pragma once
#include "stdafx.h"
#include "GLFont.h"
#include "liquidFOLDERS.h"
#include <math.h>


// GLFont
//

GLFont::GLFont()
{
	m_TexID = 0;
	m_TexSize = 128;
	m_LineHeight = 0;
}

GLFont::~GLFont()
{
	if (m_TexID)
		glDeleteTextures(1, &m_TexID);
}

BOOL GLFont::Create(CFont* pFont)
{
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	dc.SetMapMode(MM_TEXT);
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(0xFFFFFF);
	dc.SetBkColor(0x000000);
	dc.SetTextAlign(TA_TOP);

	CFont* pOldFont = dc.SelectObject(pFont);

	PAINTRESULT Result;
	while (MOREDATA==(Result = PaintAlphabet(dc, TRUE)))
		m_TexSize *= 2;

	if (Result==SUCCESS)
	{
		HBITMAP hBitmap = CreateTransparentBitmap(m_TexSize, m_TexSize);
		HGDIOBJ hOldBitmap = dc.SelectObject(hBitmap);

		Result = PaintAlphabet(dc);
		if (Result==SUCCESS)
		{
			glGenTextures(1, &m_TexID);

			BITMAP bitmap;
			GetObject(hBitmap, sizeof(BITMAP), &bitmap);

			BYTE* Ptr = (BYTE*)bitmap.bmBits;
			UINT Size = m_TexSize*m_TexSize;

			for (UINT a=0; a<Size; a++)
			{
				Ptr[3] = (Ptr[0]+Ptr[1]+Ptr[2]*2)/4;
				Ptr[0] = Ptr[1] = Ptr[2] = 0xFF;

				Ptr += 4;
			}

			glBindTexture(GL_TEXTURE_2D, m_TexID);
			glTexImage2D(GL_TEXTURE_2D, 0, 4, m_TexSize, m_TexSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, bitmap.bmBits);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		dc.SelectObject(hOldBitmap);
		DeleteObject(hBitmap);
	}

	dc.SelectObject(pOldFont);

	return Result==SUCCESS;
}

UINT GLFont::Render(CHAR* pStr, INT x, INT y, SIZE_T cCount) const
{
	if (!pStr)
		return 0;

	if (cCount<0)
		cCount = MAXINT;

	UINT Height = 0;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_TexID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBegin (GL_QUADS);

	while ((cCount-->0) && (*pStr))
	{
		UCHAR Ch = *pStr++;

		x += (Ch==' ') ? m_Spacing : (Ch>32) ? RenderChar(Ch-32, x, y, Height) : 0;
	}

	glEnd();

	glDisable(GL_TEXTURE_2D);

	return Height;
}

UINT GLFont::Render(WCHAR* pStr, INT x, INT y, SIZE_T cCount) const
{
	if (!pStr)
		return 0;

	if (cCount<0)
		cCount = MAXINT;

	UINT Height = 0;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_TexID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBegin (GL_QUADS);

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

	glEnd();

	glDisable(GL_TEXTURE_2D);

	return Height;
}

UINT GLFont::RenderChar(UCHAR Ch, INT x, INT y, UINT& Height) const
{
	GLfloat tx1 = TexCoords[Ch][0];
	GLfloat ty1 = TexCoords[Ch][1];
	GLfloat tx2 = TexCoords[Ch][2];
	GLfloat ty2 = TexCoords[Ch][3];

	UINT w = (INT)((tx2-tx1)*m_TexSize);
	UINT h = (INT)((ty2-ty1)*m_TexSize);

	glTexCoord2f(tx1, ty2);
	glVertex2i(x, y+h);
	glTexCoord2f(tx2, ty2);
	glVertex2i(x+w, y+h);
	glTexCoord2f(tx2, ty1);
	glVertex2i(x+w, y);
	glTexCoord2f(tx1, ty1);
	glVertex2i(x, y);

	if (h>Height)
		Height = h;

	return w;
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

		Width += (Ch==' ') ? m_Spacing : (Ch>32) ? (TexCoords[Ch-32][2]-TexCoords[Ch-32][0])*m_TexSize : 0;
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

		Width += (Ch==L' ') ? m_Spacing : (Ch>32) && (Ch<256) ? (TexCoords[Ch-32][2]-TexCoords[Ch-32][0])*m_TexSize : 0;
	}

	return (UINT)Width;
}

UINT GLFont::GetTextHeight(void* pStr) const
{
	ASSERT(pStr);

	return (pStr ? m_LineHeight : 0);
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

		if (x+Size.cx+m_Spacing>m_TexSize)
		{
			x = m_Spacing;
			y += Size.cy+1;
		}
		if (y+Size.cy>(INT)m_TexSize)
			return MOREDATA;

		if (!bMeasureOnly)
		{
			if (!ExtTextOutA(hDC, x, y, ETO_OPAQUE, NULL, Str, 1, NULL))
				continue;

			TexCoords[Ch-32][0] = ((GLfloat)(x))/m_TexSize;
			TexCoords[Ch-32][1] = ((GLfloat)(y))/m_TexSize;
			TexCoords[Ch-32][2] = ((GLfloat)(x+Size.cx))/m_TexSize;
			TexCoords[Ch-32][3] = ((GLfloat)(y+Size.cy))/m_TexSize;
		}

		x += Size.cx+2;
	}

	return SUCCESS;
}


// GLFont.cpp: Implementierung der Klasse GLFont
//

#pragma once
#include "stdafx.h"
#include "GLFont.h"
#include <math.h>


// CTimelineView
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

BOOL GLFont::Create(CString face, UINT size, BOOL bold, BOOL italic)
{
	CFont font;
	font.CreateFont(-(INT)size, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, (BYTE)italic, 0, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);

	return Create(&font);
}

BOOL GLFont::Create(CFont* font)
{
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	dc.SetMapMode(MM_TEXT);
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(255, 255, 255));
	dc.SetBkColor(RGB(0, 0, 0));
	dc.SetTextAlign(TA_TOP);

	HFONT hFontOld = (HFONT)dc.SelectObject(font);

	PaintResult p;
	while (MoreData==(p = PaintAlphabet(dc.m_hDC, TRUE)))
		m_TexSize *= 2;

	BOOL ok = (p==Success);
	if (ok)
	{
		UCHAR* pBitmapBits;
		BITMAPINFO bmi;
		ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = (INT)m_TexSize;
		bmi.bmiHeader.biHeight = -(INT)m_TexSize;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biBitCount = 32;

		HBITMAP hbmBitmap = CreateDIBSection(dc.m_hDC, &bmi, DIB_RGB_COLORS, (void**)&pBitmapBits, NULL, 0);
		HGDIOBJ hbmOld = dc.SelectObject(hbmBitmap);

		ok = (PaintAlphabet(dc.m_hDC)==Success);
		if (ok)
		{
			glGenTextures(1, &m_TexID);
			BITMAP bitmap;
			GetObject(hbmBitmap, sizeof(BITMAP), &bitmap);

			UCHAR* x = (UCHAR*)bitmap.bmBits;
			UINT size = m_TexSize*m_TexSize*4;
			for (UINT pos=0; pos<size; pos+=4)
			{
				x[3] = (x[0]+x[1]+x[2])/3;
				x[0] = x[1] = x[2] = 0xff;
				x += 4;
			}

			glBindTexture(GL_TEXTURE_2D, m_TexID);
			glTexImage2D(GL_TEXTURE_2D, 0, 4, m_TexSize, m_TexSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, bitmap.bmBits);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		dc.SelectObject(hbmOld);
		dc.SelectObject(hFontOld);
		DeleteObject(hbmBitmap);
	}

	return ok;
}

UINT GLFont::Render(WCHAR* pStr, INT xs, INT ys, INT cCount)
{
	if (!pStr)
		return 0;

	if (cCount<0)
		cCount = (INT)wcslen(pStr);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_TexID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBegin (GL_QUADS);
	INT x = xs;
	INT y = ys;
	UINT h = 0;

	while ((cCount>0) && (*pStr))
	{
		WCHAR ch = *pStr;
		if ((ch==2013) || (ch==8211))
			ch = 150;
		if ((ch==2014) || (ch==8212))
			ch = 151;

		if (ch==' ')
		{
			x += m_Spacing;
		}
		else
			if (((ch-32)>0) && ((ch-32)<256-32))
			{
				x += RenderChar((UCHAR)(ch-32), x, y, &h);
			}

		pStr++;
		cCount--;
	}
	glEnd();

	glDisable(GL_TEXTURE_2D);
	return h;
}

UINT GLFont::RenderChar(UCHAR ch, INT x, INT y, UINT* pHeight)
{
	ASSERT(pHeight);

	GLfloat tx1 = TexCoords[ch][0];
	GLfloat ty1 = TexCoords[ch][1];
	GLfloat tx2 = TexCoords[ch][2];
	GLfloat ty2 = TexCoords[ch][3];

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

	*pHeight = max(*pHeight, h);
	return w;
}

UINT GLFont::GetTextWidth(WCHAR* pStr, INT cCount)
{
	if (!pStr)
		return 0;

	if (cCount<0)
		cCount = (INT)wcslen(pStr);

	GLfloat w = 0;

	while ((cCount>0) && (*pStr))
	{
		WCHAR ch = *pStr;
		if ((ch==2013) || (ch==8211))
			ch = 150;
		if ((ch==2014) || (ch==8212))
			ch = 151;

		if (ch==' ')
		{
			w += m_Spacing;
		}
		else
			if (((ch-32)>0) && ((ch-32)<256-32))
			{
				w += (TexCoords[ch-32][2]-TexCoords[ch-32][0])*m_TexSize;
			}

		pStr++;
		cCount--;
	}

	return (UINT)w;
}

UINT GLFont::GetTextHeight(WCHAR* pStr)
{
	return (pStr ? m_LineHeight : 0);
}

GLFont::PaintResult GLFont::PaintAlphabet(HDC hDC, BOOL bMeasureOnly)
{
	SIZE size;
	CHAR str[2] = "?";

	if (!GetTextExtentPoint32A(hDC, str, 1, &size))
		return Fail;
	m_Spacing = (INT)ceil((double)size.cy/3);
	m_LineHeight = max(m_LineHeight, (UINT)size.cy);

	INT x = m_Spacing;
	INT y = 0;

	for (UCHAR c = 32; c < 255; c++)
	{
		str[0] = c;
		if (!GetTextExtentPoint32A(hDC, str, 1, &size))
			continue;

		if (x+size.cx+m_Spacing>m_TexSize)
		{
			x = m_Spacing;
			y += size.cy+1;
		}
		if (y+size.cy>(INT)m_TexSize)
			return MoreData;

		if (!bMeasureOnly)
		{
			if (!ExtTextOutA(hDC, x, y, ETO_OPAQUE, NULL, str, 1, NULL))
				continue;

			TexCoords[c-32][0] = ((GLfloat)(x))/m_TexSize;
			TexCoords[c-32][1] = ((GLfloat)(y))/m_TexSize;
			TexCoords[c-32][2] = ((GLfloat)(x+size.cx))/m_TexSize;
			TexCoords[c-32][3] = ((GLfloat)(y+size.cy))/m_TexSize;
		}

		x += size.cx+(2*m_Spacing);
	}

	return Success;
}

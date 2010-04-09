#include "StdAfx.h"
#include "CTextures.h"
#include "StoreManager.h"


// CTexture
//

CTexture::CTexture()
{
	m_ID = 0;
	m_SGISMIP = strstr((char*)glGetString(GL_EXTENSIONS), "SGIS_generate_mipmap")!=NULL;
}

CTexture::~CTexture()
{
	if (m_ID)
		glDeleteTextures(1, &m_ID);
}

GLuint CTexture::GetID()
{
	return m_ID;
}

void CTexture::SetTexture(UINT width, UINT height, UINT bpp, void* data)
{
	// Textur erzeugen
	UINT pixel_mode = (bpp==4 ? GL_BGRA : GL_BGR);
	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);

	// Textur-Parameter und -Daten
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (m_SGISMIP)
	{
		// Hardware-Erzeugung von Mipmaps aktiv
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, bpp, width, height, 0, pixel_mode, GL_UNSIGNED_BYTE, data);
	}
	else
		if (width!=height)
		{
			// Breite!=Höhe, daher kein Mipmapping
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, bpp, width, height, 0, pixel_mode, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			// Mipmaps von der CPU erzeugen lassen
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			gluBuild2DMipmaps(GL_TEXTURE_2D, bpp, width, height, pixel_mode, GL_UNSIGNED_BYTE, data);
		}
}


// CTextureBitmap
//

CTextureBitmap::CTextureBitmap()
{
}

CTextureBitmap::CTextureBitmap(HBITMAP hBMP)
{
	SetTextureBitmap(hBMP);
}

CTextureBitmap::~CTextureBitmap()
{
}

void CTextureBitmap::SetTextureBitmap(HBITMAP hBMP)
{
	BITMAP BMP;
	GetObject(hBMP, sizeof(BMP), &BMP);
	SetTexture(BMP.bmWidth, BMP.bmHeight, BMP.bmBitsPixel>>3, BMP.bmBits);
}


// CTextureGdiPlusBitmap
//

CTextureGdiPlusBitmap::CTextureGdiPlusBitmap()
{
}

CTextureGdiPlusBitmap::CTextureGdiPlusBitmap(CGdiPlusBitmap* texture)
{
	SetTextureGdiPlusBitmap(texture);
}

CTextureGdiPlusBitmap::~CTextureGdiPlusBitmap()
{
}

void CTextureGdiPlusBitmap::SetTextureGdiPlusBitmap(CGdiPlusBitmap* texture)
{
	HBITMAP hBMP;
	texture->m_pBitmap->GetHBITMAP(NULL, &hBMP);

	if (hBMP)
	{
		SetTextureBitmap(hBMP);
		DeleteObject(hBMP);
	}
}


// CTextureEarthmap
//

CTextureBlueMarble::CTextureBlueMarble(UINT _nID)
{
	SetTextureBitmap(theApp.GetGLTexture(_nID));
	theApp.FreeGLTexture(_nID);
}

CTextureBlueMarble::~CTextureBlueMarble()
{
}

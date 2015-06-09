
// GLTexture.cpp: Implementierung der OpenGL Textur-Klassen
//

#include "stdafx.h"
#include "GLTexture.h"
#include "resource.h"


// GLTexture
//

GLTexture::GLTexture()
{
	m_ID = 0;
	m_SGISMIP = (strstr((CHAR*)glGetString(GL_EXTENSIONS), "SGIS_generate_mipmap")!=NULL) &&
			(strcmp((CHAR*)glGetString(GL_VENDOR), "Intel")!=0);
}

GLTexture::~GLTexture()
{
	if (m_ID)
		glDeleteTextures(1, &m_ID);
}

GLuint GLTexture::GetID()
{
	return m_ID;
}

void GLTexture::SetTexture(UINT width, UINT height, UINT bpp, void* data)
{
	// Textur erzeugen
	UINT pixel_mode = (bpp==4) ? GL_BGRA : GL_BGR;
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


// GLTextureBitmap
//

GLTextureBitmap::GLTextureBitmap()
{
}

GLTextureBitmap::GLTextureBitmap(HBITMAP hBMP)
{
	SetTextureBitmap(hBMP);
}

void GLTextureBitmap::SetTextureBitmap(HBITMAP hBMP)
{
	BITMAP BMP;
	GetObject(hBMP, sizeof(BMP), &BMP);
	SetTexture(BMP.bmWidth, BMP.bmHeight, BMP.bmBitsPixel>>3, BMP.bmBits);
}


// GLTextureGdiPlusBitmap
//

GLTextureGdiPlusBitmap::GLTextureGdiPlusBitmap()
{
}

GLTextureGdiPlusBitmap::GLTextureGdiPlusBitmap(CGdiPlusBitmap* Texture)
{
	SetTextureGdiPlusBitmap(Texture);
}

void GLTextureGdiPlusBitmap::SetTextureGdiPlusBitmap(CGdiPlusBitmap* Texture)
{
	HBITMAP hBMP;
	Texture->m_pBitmap->GetHBITMAP(NULL, &hBMP);

	if (hBMP)
	{
		SetTextureBitmap(hBMP);
		DeleteObject(hBMP);
	}
}


// GLTextureGdiPlusBitmap
//

GLTextureCombine::GLTextureCombine()
{
}

GLTextureCombine::GLTextureCombine(CGdiPlusBitmap* Texture0, CGdiPlusBitmap* Texture1)
{
	SetTextureCombine(Texture0, Texture1);
}

void GLTextureCombine::SetTextureCombine(CGdiPlusBitmap* Texture0, CGdiPlusBitmap* Texture1)
{
	HBITMAP hBMP0;
	HBITMAP hBMP1;
	Texture0->m_pBitmap->GetHBITMAP(NULL, &hBMP0);
	Texture1->m_pBitmap->GetHBITMAP(NULL, &hBMP1);

	if (hBMP1)
	{
		BITMAP BMP0;
		BITMAP BMP1;
		GetObject(hBMP0, sizeof(BMP0), &BMP0);
		GetObject(hBMP1, sizeof(BMP1), &BMP1);

		ASSERT(BMP0.bmHeight==BMP1.bmHeight);
		ASSERT(BMP0.bmWidth==BMP1.bmWidth);

		UINT sz = BMP0.bmHeight*BMP0.bmWidth;
		BYTE* Ptr0 = (BYTE*)BMP0.bmBits;
		BYTE* Ptr1 = (BYTE*)BMP1.bmBits;
		for (UINT a=0; a<sz; a++)
		{
			Ptr0[3] = Ptr1[0];
			Ptr0 += 4;
			Ptr1 += 4;
		}

		DeleteObject(hBMP1);
	}

	if (hBMP0)
	{
		SetTextureBitmap(hBMP0);
		DeleteObject(hBMP0);
	}
}


// GLTextureEarthmap
//

UINT GLTextureBlueMarble::m_nIDLoaded = 0;
IPicture* GLTextureBlueMarble::p_BlueMarble = NULL;

GLTextureBlueMarble::GLTextureBlueMarble(UINT nID)
{
	nID += IDB_BLUEMARBLE_1024-1;

	if (m_nIDLoaded!=nID)
	{
		if (m_nIDLoaded)
		{
			p_BlueMarble->Release();
			p_BlueMarble = NULL;
			m_nIDLoaded = 0;
		}

		HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(nID), L"JPG");
		if (!hResource)
			return;

		HGLOBAL hMemory = LoadResource(NULL, hResource);
		if (!hMemory)
			return;

		LPVOID pResourceData = LockResource(hMemory);
		if (!pResourceData)
			return;

		DWORD Size = SizeofResource(NULL, hResource);
		if (!Size)
			return;

		IStream* pStream = SHCreateMemStream((BYTE*)pResourceData, Size);
		OleLoadPicture(pStream, 0, FALSE, IID_IPicture, (void**)&p_BlueMarble);
		pStream->Release();

		UnlockResource(hMemory);

		if (p_BlueMarble)
			m_nIDLoaded = nID;
	}

	if (p_BlueMarble)
	{
		HBITMAP hBitmap = NULL;
		if (SUCCEEDED(p_BlueMarble->get_Handle((OLE_HANDLE*)&hBitmap)))
			SetTextureBitmap(hBitmap);
	}
}

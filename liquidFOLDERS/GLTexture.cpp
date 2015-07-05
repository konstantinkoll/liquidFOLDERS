
// GLTexture.cpp: Implementierung der OpenGL Textur-Klassen
//

#include "stdafx.h"
#include "GLTexture.h"
#include "Resource.h"


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

void GLTexture::SetTexture(UINT Width, UINT Height, UINT BPP, void* pData)
{
	// Textur erzeugen
	UINT PixelMode = (BPP==4) ? GL_BGRA : GL_BGR;
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
		glTexImage2D(GL_TEXTURE_2D, 0, BPP, Width, Height, 0, PixelMode, GL_UNSIGNED_BYTE, pData);
	}
	else
		if (Width!=Height)
		{
			// Breite!=Höhe, daher kein Mipmapping
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, BPP, Width, Height, 0, PixelMode, GL_UNSIGNED_BYTE, pData);
		}
		else
		{
			// Mipmaps von der CPU erzeugen lassen
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			gluBuild2DMipmaps(GL_TEXTURE_2D, BPP, Width, Height, PixelMode, GL_UNSIGNED_BYTE, pData);
		}
}


// GLTextureBitmap
//

GLTextureBitmap::GLTextureBitmap()
{
}

GLTextureBitmap::GLTextureBitmap(HBITMAP hBitmap)
{
	SetTextureBitmap(hBitmap);
}

void GLTextureBitmap::SetTextureBitmap(HBITMAP hBitmap)
{
	BITMAP BMP;
	GetObject(hBitmap, sizeof(BMP), &BMP);

	SetTexture(BMP.bmWidth, BMP.bmHeight, BMP.bmBitsPixel>>3, BMP.bmBits);
}


// GLTextureGdiPlusBitmap
//

GLTextureGdiPlusBitmap::GLTextureGdiPlusBitmap()
{
}

GLTextureGdiPlusBitmap::GLTextureGdiPlusBitmap(CGdiPlusBitmap* pTexture)
{
	SetTextureGdiPlusBitmap(pTexture);
}

void GLTextureGdiPlusBitmap::SetTextureGdiPlusBitmap(CGdiPlusBitmap* pTexture)
{
	HBITMAP hBitmap;
	pTexture->m_pBitmap->GetHBITMAP(NULL, &hBitmap);

	if (hBitmap)
	{
		SetTextureBitmap(hBitmap);
		DeleteObject(hBitmap);
	}
}


// GLTextureGdiPlusBitmap
//

GLTextureCombine::GLTextureCombine()
{
}

GLTextureCombine::GLTextureCombine(CGdiPlusBitmap* pTexture0, CGdiPlusBitmap* pTexture1)
{
	SetTextureCombine(pTexture0, pTexture1);
}

void GLTextureCombine::SetTextureCombine(CGdiPlusBitmap* pTexture0, CGdiPlusBitmap* pTexture1)
{
	HBITMAP hBitmap0;
	pTexture0->m_pBitmap->GetHBITMAP(NULL, &hBitmap0);

	HBITMAP hBitmap1;
	pTexture1->m_pBitmap->GetHBITMAP(NULL, &hBitmap1);

	if (hBitmap1)
	{
		BITMAP BMP0;
		GetObject(hBitmap0, sizeof(BMP0), &BMP0);

		BITMAP BMP1;
		GetObject(hBitmap1, sizeof(BMP1), &BMP1);

		ASSERT(BMP0.bmHeight==BMP1.bmHeight);
		ASSERT(BMP0.bmWidth==BMP1.bmWidth);

		BYTE* Ptr0 = (BYTE*)BMP0.bmBits;
		BYTE* Ptr1 = (BYTE*)BMP1.bmBits;
		UINT Size = BMP0.bmHeight*BMP0.bmWidth;

		for (UINT a=0; a<Size; a++)
		{
			Ptr0[3] = Ptr1[0];
			Ptr0 += 4;
			Ptr1 += 4;
		}

		DeleteObject(hBitmap1);
	}

	if (hBitmap0)
	{
		SetTextureBitmap(hBitmap0);
		DeleteObject(hBitmap0);
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

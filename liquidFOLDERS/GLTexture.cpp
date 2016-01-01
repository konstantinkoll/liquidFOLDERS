
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

GLuint GLTexture::GetID() const
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

GLTextureGdiPlusBitmap::GLTextureGdiPlusBitmap(Bitmap* pTexture)
{
	SetTextureGdiPlusBitmap(pTexture);
}

void GLTextureGdiPlusBitmap::SetTextureGdiPlusBitmap(Bitmap* pTexture)
{
	HBITMAP hBitmap = NULL;
	pTexture->GetHBITMAP(NULL, &hBitmap);

	if (hBitmap)
	{
		SetTextureBitmap(hBitmap);
		DeleteObject(hBitmap);
	}
}


// GLTextureCombine
//

GLTextureCombine::GLTextureCombine()
{
}

GLTextureCombine::GLTextureCombine(Bitmap* pTexture0, Bitmap* pTexture1)
{
	SetTextureCombine(pTexture0, pTexture1);
}

void GLTextureCombine::SetTextureCombine(Bitmap* pTexture0, Bitmap* pTexture1)
{
	HBITMAP hBitmap0 = NULL;
	pTexture0->GetHBITMAP(NULL, &hBitmap0);

	HBITMAP hBitmap1 = NULL;
	pTexture1->GetHBITMAP(NULL, &hBitmap1);

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
IPicture* GLTextureBlueMarble::m_pBlueMarble = NULL;

GLTextureBlueMarble::GLTextureBlueMarble(UINT nID)
{
	nID += IDB_BLUEMARBLE_1024-1;

	if (m_nIDLoaded!=nID)
	{
		if (m_pBlueMarble)
		{
			m_pBlueMarble->Release();
			m_pBlueMarble = NULL;

			m_nIDLoaded = nID;
		}

		HRSRC hResource = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), RT_RCDATA);
		if (hResource)
		{
			HGLOBAL hMemory = LoadResource(AfxGetResourceHandle(), hResource);
			if (hMemory)
			{
				LPVOID pResourceData = LockResource(hMemory);
				if (pResourceData)
				{
					DWORD Size = SizeofResource(AfxGetResourceHandle(), hResource);
					if (Size)
					{
						IStream* pStream = SHCreateMemStream((BYTE*)pResourceData, Size);

						OleLoadPicture(pStream, 0, FALSE, IID_IPicture, (void**)&m_pBlueMarble);

						pStream->Release();
					}
				}
			}
		}

		if (m_pBlueMarble)
			m_nIDLoaded = nID;
	}

	if (m_pBlueMarble)
	{
		HBITMAP hBitmap;

		if (SUCCEEDED(m_pBlueMarble->get_Handle((OLE_HANDLE*)&hBitmap)))
			SetTextureBitmap(hBitmap);
	}
}

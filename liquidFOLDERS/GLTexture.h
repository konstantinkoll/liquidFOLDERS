
// GLTexture.h: Schnittstelle der OpenGL Textur-Klassen
//

#pragma once
#include "stdafx.h"
#include "LFCommDlg.h"


// GLTexture
//

class GLTexture
{
public:
	GLTexture();
	virtual ~GLTexture();

	GLuint GetID() const;
	void SetTexture(UINT Width, UINT Height, UINT BPP, void* pData);

protected:
	GLuint m_ID;
	BOOL m_SGISMIP;
};


// GLTextureBitmap
//

class GLTextureBitmap : public GLTexture
{
public:
	GLTextureBitmap();
	GLTextureBitmap(HBITMAP hBitmap);

	void SetTextureBitmap(HBITMAP hBitmap);
};


// GLTextureGdiPlusBitmap
//

class GLTextureGdiPlusBitmap : public GLTextureBitmap
{
public:
	GLTextureGdiPlusBitmap();
	GLTextureGdiPlusBitmap(Bitmap* pTexture);

	void SetTextureGdiPlusBitmap(Bitmap* pTexture);
};


// GLTextureCombine
//

class GLTextureCombine : public GLTextureGdiPlusBitmap
{
public:
	GLTextureCombine();
	GLTextureCombine(Bitmap* pTexture0, Bitmap* pTexture1);

	void SetTextureCombine(Bitmap* pTexture0, Bitmap* pTexture1);
};


// GLTextureEarthmap
//

class GLTextureBlueMarble : public GLTextureBitmap
{
public:
	GLTextureBlueMarble(UINT nID);

private:
	static UINT m_nIDLoaded;
	static IPicture* m_pBlueMarble;
};

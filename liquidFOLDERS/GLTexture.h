
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

	GLuint GetID();
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
	GLTextureGdiPlusBitmap(CGdiPlusBitmap* pTexture);

	void SetTextureGdiPlusBitmap(CGdiPlusBitmap* pTexture);
};


// GLTextureCombine
//

class GLTextureCombine : public GLTextureGdiPlusBitmap
{
public:
	GLTextureCombine();
	GLTextureCombine(CGdiPlusBitmap* pTexture0, CGdiPlusBitmap* pTexture1);

	void SetTextureCombine(CGdiPlusBitmap* pTexture0, CGdiPlusBitmap* pTexture1);
};


// GLTextureEarthmap
//

class GLTextureBlueMarble : public GLTextureBitmap
{
public:
	GLTextureBlueMarble(UINT nID);

private:
	static UINT m_nIDLoaded;
	static IPicture* p_BlueMarble;
};

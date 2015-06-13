
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
	void SetTexture(UINT Width, UINT Height, UINT BPP, void* Data);

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
	GLTextureBitmap(HBITMAP hBMP);

	void SetTextureBitmap(HBITMAP hBMP);
};


// GLTextureGdiPlusBitmap
//

class GLTextureGdiPlusBitmap : public GLTextureBitmap
{
public:
	GLTextureGdiPlusBitmap();
	GLTextureGdiPlusBitmap(CGdiPlusBitmap* Texture);

	void SetTextureGdiPlusBitmap(CGdiPlusBitmap* Texture);
};


// GLTextureCombine
//

class GLTextureCombine : public GLTextureGdiPlusBitmap
{
public:
	GLTextureCombine();
	GLTextureCombine(CGdiPlusBitmap* Texture0, CGdiPlusBitmap* Texture1);

	void SetTextureCombine(CGdiPlusBitmap* Texture0, CGdiPlusBitmap* Texture1);
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


#pragma once
#include "stdafx.h"
#include "LFCommDlg.h"


// CTexture
//

class CTexture
{
public:
	CTexture();
	virtual ~CTexture();

	GLuint GetID();
	void SetTexture(UINT width, UINT height, UINT bpp, void* data);

protected:
	GLuint m_ID;
	BOOL m_SGISMIP;
};


// CTextureBitmap
//

class CTextureBitmap : public CTexture
{
public:
	CTextureBitmap();
	CTextureBitmap(HBITMAP hBMP);
	~CTextureBitmap();

	void SetTextureBitmap(HBITMAP hBMP);
};


// CTextureGdiPlusBitmap
//

class CTextureGdiPlusBitmap : public CTextureBitmap
{
public:
	CTextureGdiPlusBitmap();
	CTextureGdiPlusBitmap(CGdiPlusBitmap* texture);
	~CTextureGdiPlusBitmap();

	void SetTextureGdiPlusBitmap(CGdiPlusBitmap* texture);
};


// CTextureEarthmap
//

class CTextureBlueMarble : public CTextureBitmap
{
public:
	CTextureBlueMarble(UINT _nID);
	~CTextureBlueMarble();
};

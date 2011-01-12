
#pragma once


// CGdiPlusBitmap
//

class AFX_EXT_CLASS CGdiPlusBitmap
{
public:
	CGdiPlusBitmap();
	CGdiPlusBitmap(LPCWSTR pFile);
	~CGdiPlusBitmap();

	virtual void Empty();

	BOOL Load(LPCWSTR pFile);

	Gdiplus::Bitmap* m_pBitmap;
};


// CGdiPlusBitmapResource
//

class AFX_EXT_CLASS CGdiPlusBitmapResource : public CGdiPlusBitmap
{
public:
	CGdiPlusBitmapResource();
	CGdiPlusBitmapResource(LPCTSTR pName, LPCTSTR pType=RT_RCDATA, HMODULE hInst=NULL);
	CGdiPlusBitmapResource(UINT id, LPCTSTR pType=RT_RCDATA, HMODULE hInst=NULL);
	CGdiPlusBitmapResource(UINT id, UINT type, HMODULE hInst=NULL);

	virtual void Empty();

	BOOL Load(LPCTSTR pName, LPCTSTR pType = RT_RCDATA, HMODULE hInst=NULL);
	BOOL Load(UINT id, LPCTSTR pType = RT_RCDATA, HMODULE hInst=NULL);
	BOOL Load(UINT id, UINT type, HMODULE hInst=NULL);

protected:
	HGLOBAL m_hBuffer;
};

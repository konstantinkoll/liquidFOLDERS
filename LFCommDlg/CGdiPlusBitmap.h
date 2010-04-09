
#pragma once


// CGdiPlusBitmap
//

class AFX_EXT_CLASS CGdiPlusBitmap
{
public:
	CGdiPlusBitmap();
	CGdiPlusBitmap(LPCWSTR pFile);
	virtual ~CGdiPlusBitmap();

	void Empty();
	bool Load(LPCWSTR pFile);

	operator Gdiplus::Bitmap*() const;

	Gdiplus::Bitmap* m_pBitmap;
};


// CGdiPlusBitmapResource
//

class AFX_EXT_CLASS CGdiPlusBitmapResource : public CGdiPlusBitmap
{
public:
	CGdiPlusBitmapResource();
	CGdiPlusBitmapResource(LPCTSTR pName, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
	CGdiPlusBitmapResource(UINT id, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
	CGdiPlusBitmapResource(UINT id, UINT type, HMODULE hInst = NULL);
	virtual ~CGdiPlusBitmapResource();

	void Empty();
	bool Load(LPCTSTR pName, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
	bool Load(UINT id, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
	bool Load(UINT id, UINT type, HMODULE hInst = NULL);

protected:
	HGLOBAL m_hBuffer;
};

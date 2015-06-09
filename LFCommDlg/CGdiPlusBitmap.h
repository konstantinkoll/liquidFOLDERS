
#pragma once


// CGdiPlusBitmap
//

class CGdiPlusBitmap
{
public:
	CGdiPlusBitmap();
	CGdiPlusBitmap(LPCWSTR pFile);
	~CGdiPlusBitmap();

	virtual void Empty();

	BOOL Load(LPCWSTR pFile);

	Bitmap* m_pBitmap;
};


// CGdiPlusBitmapMemory
//

class CGdiPlusBitmapMemory : public CGdiPlusBitmap
{
public:
	CGdiPlusBitmapMemory();
	CGdiPlusBitmapMemory(LPVOID pMemory, DWORD Size);

	BOOL Load(LPVOID pMemory, DWORD Size);
};


// CGdiPlusBitmapResource
//

class CGdiPlusBitmapResource : public CGdiPlusBitmapMemory
{
public:
	CGdiPlusBitmapResource();
	CGdiPlusBitmapResource(LPCTSTR pName, LPCTSTR pType=RT_RCDATA, HMODULE hInst=NULL);
	CGdiPlusBitmapResource(UINT id, LPCTSTR pType=RT_RCDATA, HMODULE hInst=NULL);
	CGdiPlusBitmapResource(UINT id, UINT type, HMODULE hInst=NULL);

	BOOL Load(LPCTSTR pName, LPCTSTR pType=RT_RCDATA, HMODULE hInst=NULL);
	BOOL Load(UINT id, LPCTSTR pType=RT_RCDATA, HMODULE hInst=NULL);
	BOOL Load(UINT id, UINT type, HMODULE hInst=NULL);
};

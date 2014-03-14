
#pragma once
#include "stdafx.h"
#include "CGdiPlusBitmap.h"


// CGdiPlusBitmap
//

CGdiPlusBitmap::CGdiPlusBitmap()
{
	m_pBitmap = NULL;
}

CGdiPlusBitmap::CGdiPlusBitmap(LPCWSTR pFile)
{
	m_pBitmap = NULL;
	Load(pFile);
}

CGdiPlusBitmap::~CGdiPlusBitmap()
{
	Empty();
}

void CGdiPlusBitmap::Empty()
{
	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
}

BOOL CGdiPlusBitmap::Load(LPCWSTR pFile)
{
	Empty();

	m_pBitmap = Gdiplus::Bitmap::FromFile(pFile);
	return SUCCEEDED(m_pBitmap->GetLastStatus()==Gdiplus::Ok);
}


// CGdiPlusBitmapMemory
//

CGdiPlusBitmapMemory::CGdiPlusBitmapMemory()
{
}

CGdiPlusBitmapMemory::CGdiPlusBitmapMemory(LPVOID pMemory, DWORD Size)
{
	Load(pMemory, Size);
}

BOOL CGdiPlusBitmapMemory::Load(LPVOID pMemory, DWORD Size)
{
	Empty();

	if (!pMemory)
		return FALSE;

	IStream* pStream = SHCreateMemStream((BYTE*)pMemory, Size);
	m_pBitmap = Gdiplus::Bitmap::FromStream(pStream);
	pStream->Release();

	if (m_pBitmap)
		if (m_pBitmap->GetLastStatus()==Gdiplus::Ok)
			return TRUE;

	delete m_pBitmap;
	m_pBitmap = NULL;
	return FALSE;
}


// CGdiPlusBitmapResource
//

CGdiPlusBitmapResource::CGdiPlusBitmapResource()
{
}

CGdiPlusBitmapResource::CGdiPlusBitmapResource(LPCTSTR pName, LPCTSTR pType, HMODULE hInst)
{
	Load(pName, pType, hInst);
}

CGdiPlusBitmapResource::CGdiPlusBitmapResource(UINT id, LPCTSTR pType, HMODULE hInst)
{
	Load(id, pType, hInst);
}

CGdiPlusBitmapResource::CGdiPlusBitmapResource(UINT id, UINT type, HMODULE hInst)
{
	Load(id, type, hInst);
}

BOOL CGdiPlusBitmapResource::Load(LPCTSTR pName, LPCTSTR pType, HMODULE hInst)
{
	Empty();

	HRSRC hResource = FindResource(hInst, pName, pType);
	if (!hResource)
		return FALSE;

	LPVOID pResourceData = LockResource(LoadResource(hInst, hResource));
	if (!pResourceData)
		return FALSE;

	DWORD Size = SizeofResource(hInst, hResource);
	if (!Size)
		return FALSE;

	return CGdiPlusBitmapMemory::Load(pResourceData, Size);
}

BOOL CGdiPlusBitmapResource::Load(UINT id, LPCTSTR pType, HMODULE hInst)
{
	return Load(MAKEINTRESOURCE(id), pType, hInst);
}

BOOL CGdiPlusBitmapResource::Load(UINT id, UINT type, HMODULE hInst)
{
	return Load(MAKEINTRESOURCE(id), MAKEINTRESOURCE(type), hInst);
}

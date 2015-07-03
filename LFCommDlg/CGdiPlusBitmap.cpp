
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
	delete m_pBitmap;
	m_pBitmap = NULL;
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
	{
		if (m_pBitmap->GetLastStatus()==Gdiplus::Ok)
			return TRUE;

		delete m_pBitmap;
		m_pBitmap = NULL;
	}

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

CGdiPlusBitmapResource::CGdiPlusBitmapResource(UINT nID, LPCTSTR pType, HMODULE hInst)
{
	Load(nID, pType, hInst);
}

CGdiPlusBitmapResource::CGdiPlusBitmapResource(UINT nID, UINT Type, HMODULE hInst)
{
	Load(nID, Type, hInst);
}

BOOL CGdiPlusBitmapResource::Load(LPCTSTR pName, LPCTSTR pType, HMODULE hInst)
{
	Empty();

	HRSRC hResource = FindResource(hInst, pName, pType);
	if (!hResource)
		return FALSE;

	HGLOBAL hMemory = LoadResource(hInst, hResource);
	if (!hMemory)
		return FALSE;

	LPVOID pResourceData = LockResource(hMemory);
	if (!pResourceData)
		return FALSE;

	DWORD Size = SizeofResource(hInst, hResource);
	if (!Size)
		return FALSE;

	BOOL Result = CGdiPlusBitmapMemory::Load(pResourceData, Size);

	UnlockResource(hMemory);
	return Result;
}

BOOL CGdiPlusBitmapResource::Load(UINT nID, LPCTSTR pType, HMODULE hInst)
{
	return Load(MAKEINTRESOURCE(nID), pType, hInst);
}

BOOL CGdiPlusBitmapResource::Load(UINT nID, UINT Type, HMODULE hInst)
{
	return Load(MAKEINTRESOURCE(nID), MAKEINTRESOURCE(Type), hInst);
}

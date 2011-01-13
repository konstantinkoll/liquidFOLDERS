
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


// CGdiPlusBitmapResource
//

CGdiPlusBitmapResource::CGdiPlusBitmapResource()
{
	m_hBuffer = NULL;
}

CGdiPlusBitmapResource::CGdiPlusBitmapResource(LPCTSTR pName, LPCTSTR pType, HMODULE hInst)
{
	m_hBuffer = NULL;
	Load(pName, pType, hInst);
}

CGdiPlusBitmapResource::CGdiPlusBitmapResource(UINT id, LPCTSTR pType, HMODULE hInst)
{
	m_hBuffer = NULL;
	Load(id, pType, hInst);
}

CGdiPlusBitmapResource::CGdiPlusBitmapResource(UINT id, UINT type, HMODULE hInst)
{
	m_hBuffer = NULL;
	Load(id, type, hInst);
}

void CGdiPlusBitmapResource::Empty()
{
	CGdiPlusBitmap::Empty();

	if (m_hBuffer)
	{
		GlobalUnlock(m_hBuffer);
		GlobalFree(m_hBuffer);
		m_hBuffer = NULL;
	}
}

BOOL CGdiPlusBitmapResource::Load(LPCTSTR pName, LPCTSTR pType, HMODULE hInst)
{
	Empty();

	HRSRC hResource = FindResource(hInst, pName, pType);
	if (!hResource)
		return FALSE;

	DWORD imageSize = SizeofResource(hInst, hResource);
	if (!imageSize)
		return FALSE;

	const void* pResourceData = LockResource(LoadResource(hInst, hResource));
	if (!pResourceData)
		return FALSE;

	m_hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
	if (m_hBuffer)
	{
		void* pBuffer = GlobalLock(m_hBuffer);
		if (pBuffer)
		{
			CopyMemory(pBuffer, pResourceData, imageSize);

			IStream* pStream = NULL;
			if (SUCCEEDED(CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream)))
			{
				m_pBitmap = Gdiplus::Bitmap::FromStream(pStream);
				pStream->Release();
				if (m_pBitmap)
				{
					if (m_pBitmap->GetLastStatus()==Gdiplus::Ok)
						return TRUE;

					delete m_pBitmap;
					m_pBitmap = NULL;
				}
			}
			GlobalUnlock(m_hBuffer);
		}

		GlobalFree(m_hBuffer);
		m_hBuffer = NULL;
	}

	return FALSE;
}

BOOL CGdiPlusBitmapResource::Load(UINT id, LPCTSTR pType, HMODULE hInst)
{
	return Load(MAKEINTRESOURCE(id), pType, hInst);
}

BOOL CGdiPlusBitmapResource::Load(UINT id, UINT type, HMODULE hInst)
{
	return Load(MAKEINTRESOURCE(id), MAKEINTRESOURCE(type), hInst);
}

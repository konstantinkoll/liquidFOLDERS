
// LFStoreDataObject.cpp: Implementierung der Klasse LFStoreDataObject
//

#include "stdafx.h"
#include "LFCommDlg.h"


BOOL CreateGlobalMemory(const void* pSrc, SIZE_T sz, HGLOBAL& hDst)
{
	if (!pSrc)
	{
		hDst = NULL;
		return FALSE;
	}

	hDst = GlobalAlloc(GMEM_MOVEABLE, sz);
	if (!hDst)
		return FALSE;

	void* pDst = GlobalLock(hDst);
	memcpy(pDst, pSrc, sz);
	GlobalUnlock(hDst);

	return TRUE;
}


// LFStoreDataObject
//

LFStoreDataObject::LFStoreDataObject(LFItemDescriptor* pItemDescriptor)
{
	m_lRefCount = 1;
	m_hDescriptor = m_hShellLink = NULL;

	IShellLink* pShellLink = LFGetShortcutForStore(pItemDescriptor);
	if (pShellLink)
	{
		CreateGlobals(pShellLink, pItemDescriptor->CoreAttributes.FileName);
		pShellLink->Release();
	}
}

void LFStoreDataObject::CreateGlobals(IShellLink* pShellLink, WCHAR* Name)
{
	WCHAR Path[MAX_PATH];
	if (GetTempPath(MAX_PATH, Path))
	{
		WCHAR Filename[MAX_PATH];
		if (GetTempFileName(Path, L"LNK", 0, Filename))
		{
			// Datei erzeugen
			IPersistFile* pPersistFile = NULL;
			if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile)))
			{
				// Save the link by calling IPersistFile::Save
				pPersistFile->Save(Filename, TRUE);
				pPersistFile->Release();

				HANDLE hFile = CreateFile(Filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if (hFile!=INVALID_HANDLE_VALUE)
				{
					LARGE_INTEGER Size;
					Size.QuadPart = 0;
					GetFileSizeEx(hFile, &Size);

					ASSERT(Size.HighPart==0);

					m_hShellLink = GlobalAlloc(GMEM_MOVEABLE, Size.LowPart);
					if (m_hShellLink)
					{
						void* pDst = GlobalLock(m_hShellLink);
						DWORD Read;
						ReadFile(hFile, pDst, Size.LowPart, &Read, NULL);
						GlobalUnlock(m_hShellLink);

						// Descriptor
						FILEGROUPDESCRIPTOR fgd;
						ZeroMemory(&fgd, sizeof(fgd));
						fgd.cItems = 1;
						fgd.fgd[0].dwFlags = FD_FILESIZE | FD_WRITESTIME;
						fgd.fgd[0].nFileSizeLow = Size.LowPart;
						fgd.fgd[0].nFileSizeHigh = Size.HighPart;
						fgd.fgd[0].ftLastWriteTime.dwLowDateTime = 0x256D4000;
						fgd.fgd[0].ftLastWriteTime.dwHighDateTime = 0x01BF53EB;

						wcscpy_s(fgd.fgd[0].cFileName, MAX_PATH, Name);

						WCHAR* Ptr = fgd.fgd[0].cFileName;
						while (*Ptr!=L'\0')
						{
							if ((*Ptr<L' ') || (wcschr(L"<>:\"/\\|?*", *Ptr)))
								*Ptr = L'_';

							Ptr++;
						}

						wcscat_s(fgd.fgd[0].cFileName, MAX_PATH, L".lnk");

						CreateGlobalMemory(&fgd, sizeof(fgd), m_hDescriptor);
						CloseHandle(hFile);
					}

					// Datei löschen
					DeleteFile(Filename);
				}
			}
		}
	}
}

STDMETHODIMP LFStoreDataObject::QueryInterface(REFIID iid, void** ppvObject)
{
	if ((iid==IID_IDataObject) || (iid==IID_IUnknown))
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE LFStoreDataObject::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE LFStoreDataObject::Release()
{
	LONG Count = InterlockedDecrement(&m_lRefCount);
	if (!Count)
	{
		if (m_hDescriptor)
			GlobalFree(m_hDescriptor);

		if (m_hShellLink)
			GlobalFree(m_hShellLink);

		delete this;
		return 0;
	}

	return Count;
}

STDMETHODIMP LFStoreDataObject::GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
{
	if ((!pFormatEtc) || (!pMedium))
		return DV_E_FORMATETC;

	if ((pFormatEtc->tymed & TYMED_HGLOBAL)==0)
		return DV_E_FORMATETC;

	if (pFormatEtc->cfFormat==LFGetApp()->CF_FILEDESCRIPTOR)
	{
		if (!DuplicateGlobalMemory(m_hDescriptor, pMedium->hGlobal))
			return STG_E_MEDIUMFULL;

		pMedium->tymed = TYMED_HGLOBAL;
		pMedium->pUnkForRelease = NULL;
		return S_OK;
	}

	if (pFormatEtc->cfFormat==LFGetApp()->CF_FILECONTENTS)
	{
		if (!DuplicateGlobalMemory(m_hShellLink, pMedium->hGlobal))
			return STG_E_MEDIUMFULL;

		pMedium->tymed = TYMED_HGLOBAL;
		pMedium->pUnkForRelease = NULL;
		return S_OK;
	}

	return DV_E_FORMATETC;
}

STDMETHODIMP LFStoreDataObject::GetDataHere(FORMATETC* /*pFormatEtc*/, STGMEDIUM* /*pMedium*/)
{
	return DATA_E_FORMATETC;
}

STDMETHODIMP LFStoreDataObject::QueryGetData(FORMATETC* pFormatEtc)
{
	return ((pFormatEtc->cfFormat==LFGetApp()->CF_FILEDESCRIPTOR) || (pFormatEtc->cfFormat==LFGetApp()->CF_FILECONTENTS)) &&
		(pFormatEtc->tymed & TYMED_HGLOBAL) ? S_OK : DV_E_FORMATETC;
}

STDMETHODIMP LFStoreDataObject::GetCanonicalFormatEtc(FORMATETC* /*pFormatEtcIn*/, FORMATETC* pFormatEtcOut)
{
	pFormatEtcOut->ptd = NULL;

	return E_NOTIMPL;
}

STDMETHODIMP LFStoreDataObject::SetData(FORMATETC* /*pFormatEtc*/, STGMEDIUM* /*pMedium*/, BOOL /*fRelease*/)
{
	return E_INVALIDARG;
}

STDMETHODIMP LFStoreDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
	if (dwDirection==DATADIR_GET)
	{
		FORMATETC fmt[2];
		ZeroMemory(&fmt, sizeof(fmt));

		fmt[0].cfFormat = LFGetApp()->CF_FILEDESCRIPTOR;
		fmt[0].dwAspect = DVASPECT_CONTENT;
		fmt[0].lindex = -1;
		fmt[0].tymed = TYMED_HGLOBAL;

		fmt[1].cfFormat = LFGetApp()->CF_FILECONTENTS;
		fmt[1].dwAspect = DVASPECT_CONTENT;
		fmt[1].lindex = -1;
		fmt[1].tymed = TYMED_HGLOBAL;

		return SHCreateStdEnumFmtEtc(2, fmt, ppenumFormatEtc);
	}

	return E_NOTIMPL;
}

STDMETHODIMP LFStoreDataObject::DAdvise(FORMATETC* /*pFormatEtc*/, DWORD /*advf*/, IAdviseSink* /*pAdvSink*/, DWORD* /*pdwConnection*/)
{
	return OLE_E_ADVISENOTSUPPORTED;;
}

STDMETHODIMP LFStoreDataObject::DUnadvise(DWORD /*dwConnection*/)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP LFStoreDataObject::EnumDAdvise(IEnumSTATDATA** /*ppenumAdvise*/)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

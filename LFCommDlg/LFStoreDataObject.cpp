
// LFStoreDataObject.cpp: Implementierung der Klasse LFStoreDataObject
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFStoreDataObject
//

LFStoreDataObject::LFStoreDataObject(LFItemDescriptor* pItemDescriptor)
{
	m_lRefCount = 1;
	m_hDescriptor = m_hShellLink = NULL;

	IShellLink* pShellLink;
	if (LFGetShortcutForItem(pItemDescriptor, pShellLink)==LFOk)
	{
		CreateGlobals(pShellLink, pItemDescriptor->CoreAttributes.FileName);

		pShellLink->Release();
	}
}

BOOL LFStoreDataObject::CreateGlobalMemory(LPCVOID pSrc, SIZE_T Size, HGLOBAL& hDst)
{
	ASSERT(pSrc);
	ASSERT(Size>0);

	if ((hDst=GlobalAlloc(GMEM_MOVEABLE, Size))==NULL)
		return FALSE;

	LPVOID pDst = GlobalLock(hDst);

	memcpy(pDst, pSrc, Size);
	GlobalUnlock(hDst);

	return TRUE;
}

void LFStoreDataObject::CreateGlobals(IShellLink* pShellLink, LPCWSTR Name)
{
	if ((m_hShellLink=GlobalAlloc(GMEM_MOVEABLE, 0))!=NULL)
	{
		// Create stream
		IStream* pStream;
		if (SUCCEEDED(CreateStreamOnHGlobal(m_hShellLink, FALSE, &pStream)))
		{
			// Get IPersist object
			IPersistStream* pPersistStream = NULL;
			if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistStream, (void**)&pPersistStream)))
			{
				// Save the link to stream by calling IPersistStream::Save
				pPersistStream->Save(pStream, TRUE);

				// Stream size
				LARGE_INTEGER Move;
				Move.LowPart = Move.HighPart = 0;

				ULARGE_INTEGER Size;
				if (SUCCEEDED(pStream->Seek(Move, STREAM_SEEK_CUR, &Size)))
				{
					// File group descriptor
					FILEGROUPDESCRIPTOR FileGroupDescriptor;
					ZeroMemory(&FileGroupDescriptor, sizeof(FileGroupDescriptor));

					FileGroupDescriptor.cItems = 1;
					FileGroupDescriptor.fgd[0].dwFlags = FD_FILESIZE | FD_WRITESTIME;
					FileGroupDescriptor.fgd[0].nFileSizeLow = Size.LowPart;
					FileGroupDescriptor.fgd[0].nFileSizeHigh = Size.HighPart;
					FileGroupDescriptor.fgd[0].ftLastWriteTime.dwLowDateTime = 0x256D4000;
					FileGroupDescriptor.fgd[0].ftLastWriteTime.dwHighDateTime = 0x01BF53EB;

					// Link name
					wcscpy_s(FileGroupDescriptor.fgd[0].cFileName, MAX_PATH, Name);

					WCHAR* pChar = FileGroupDescriptor.fgd[0].cFileName;
					while (*pChar!=L'\0')
					{
						if ((*pChar<L' ') || (wcschr(L"<>:\"/\\|?*", *pChar)))
							*pChar = L'_';

						pChar++;
					}

					wcscat_s(FileGroupDescriptor.fgd[0].cFileName, MAX_PATH, L".lnk");

					CreateGlobalMemory(&FileGroupDescriptor, sizeof(FileGroupDescriptor), m_hDescriptor);
				}

				// Release IPersist object
				pPersistStream->Release();
			}

			// Release stream
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

STDMETHODIMP LFStoreDataObject::DAdvise(FORMATETC* /*pFormatEtc*/, DWORD /*advf*/, IAdviseSink* /*pAdvSink*/, LPDWORD /*pdwConnection*/)
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

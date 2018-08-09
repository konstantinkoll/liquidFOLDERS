
// LFDataSource.cpp: Implementierung der Klasse LFDataSource
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFDataSource
//

LFDataSource::LFDataSource(LFItemDescriptor* pItemDescriptor)
	: COleDataSource()
{
	ASSERT(pItemDescriptor);

	IShellLink* pShellLink;
	if (LFGetShortcutForItem(pItemDescriptor, pShellLink)==LFOk)
	{
		HGLOBAL hShellLink;
		if ((hShellLink=GlobalAlloc(GMEM_MOVEABLE, 0))!=NULL)
		{
			// Create stream
			IStream* pStream;
			if (SUCCEEDED(CreateStreamOnHGlobal(hShellLink, FALSE, &pStream)))
			{
				// Get IPersist object
				IPersistStream* pPersistStream = NULL;
				if (SUCCEEDED(pShellLink->QueryInterface(IID_PPV_ARGS(&pPersistStream))))
				{
					// Save the link to stream by calling IPersistStream::Save
					pPersistStream->Save(pStream, TRUE);

					// Stream size
					LARGE_INTEGER Move;
					Move.LowPart = Move.HighPart = 0;

					ULARGE_INTEGER Size;
					if (SUCCEEDED(pStream->Seek(Move, STREAM_SEEK_CUR, &Size)))
					{
						// Cache file contents and detach handle
						COleDataSource::CacheGlobalData(LFGetApp()->CF_FILECONTENTS, hShellLink);
						hShellLink = NULL;

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
						wcscpy_s(FileGroupDescriptor.fgd[0].cFileName, MAX_PATH, pItemDescriptor->CoreAttributes.FileName);

						WCHAR* pChar = FileGroupDescriptor.fgd[0].cFileName;
						while (*pChar!=L'\0')
						{
							if ((*pChar<L' ') || (wcschr(L"<>:\"/\\|?*", *pChar)))
								*pChar = L'_';

							pChar++;
						}

						wcscat_s(FileGroupDescriptor.fgd[0].cFileName, MAX_PATH, L".lnk");

						CacheGlobalData(LFGetApp()->CF_FILEDESCRIPTOR, &FileGroupDescriptor, sizeof(FileGroupDescriptor));
					}

					// Release IPersist object
					pPersistStream->Release();
				}
			}

			// Free hShellLink
			if (hShellLink)
				GlobalFree(hShellLink);
		}

		pShellLink->Release();
	}

	InitDragSourceHelper(pItemDescriptor);
}

LFDataSource::LFDataSource(LFTransactionList* pTransactionList)
	: COleDataSource()
{
	ASSERT(pTransactionList);

	COleDataSource::CacheGlobalData(CF_HDROP, LFCreateDropFiles(pTransactionList));
	COleDataSource::CacheGlobalData(LFGetApp()->CF_LIQUIDFILES, LFCreateLiquidFiles(pTransactionList));

	InitDragSourceHelper(IDB_MULTIPLE_48);
}

void LFDataSource::InitDragSourceHelper(HBITMAP hBitmap)
{
	ASSERT(hBitmap);

	m_pDragSourceHelper = NULL;
	m_pDragSourceHelper2 = NULL;

	// DragSourceHelper
	CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pDragSourceHelper));

	if (m_pDragSourceHelper)
	{
		if (SUCCEEDED(m_pDragSourceHelper->QueryInterface(IID_PPV_ARGS(&m_pDragSourceHelper2))))
		{
			m_pDragSourceHelper2->SetFlags(DSH_ALLOWDROPDESCRIPTIONTEXT);

			// Replace background color with mask color, as DragSourceHelper::InitializeFromBitmap enforces color masking :(
			BITMAP Bitmap;
			GetObject(hBitmap, sizeof(Bitmap), &Bitmap);

			if (Bitmap.bmBitsPixel==32)
			{
				ASSERT(Bitmap.bmBits);

				for (INT Row=0; Row<Bitmap.bmHeight; Row++)
				{
					LPBYTE pByte = (LPBYTE)Bitmap.bmBits+Bitmap.bmWidthBytes*Row;

					for (INT Column=0; Column<Bitmap.bmWidth; Column++)
					{
						if (*((COLORREF*)pByte)==0xFFFFFF)
							*((COLORREF*)pByte) = 0xFF00FF;

						pByte += 4;
					}
				}
			}

			// Initialize
			SHDRAGIMAGE DragImage;
			DragImage.crColorKey = 0xFFFF00FF;
			DragImage.hbmpDragImage = hBitmap;
			DragImage.ptOffset.x = (DragImage.sizeDragImage.cx=Bitmap.bmWidth)*3/5;
			DragImage.ptOffset.y = (DragImage.sizeDragImage.cy=Bitmap.bmHeight)*3/5;

			m_pDragSourceHelper2->InitializeFromBitmap(&DragImage, GetDataObject());

			DeleteObject(hBitmap);
		}
	}
}

void LFDataSource::InitDragSourceHelper(LFItemDescriptor* pItemDescriptor)
{
	ASSERT(pItemDescriptor);

	InitDragSourceHelper(CIcons::ExtractBitmap(LFGetApp()->m_CoreImageListExtraLarge, pItemDescriptor->IconID-1, TRUE));
}

void LFDataSource::InitDragSourceHelper(UINT nID)
{
	Bitmap* pBitmap = LFGetApp()->GetCachedResourceImage(nID);

	HBITMAP hBitmap;
	if (pBitmap->GetHBITMAP(Color(0xFFFFFFFF), &hBitmap)==Ok)
		InitDragSourceHelper(hBitmap);
}

void LFDataSource::CacheGlobalData(CLIPFORMAT cfFormat, LPVOID pData, SIZE_T szData, LPFORMATETC lpFormatEtc)
{
	ASSERT(pData);
	ASSERT(szData>0);

	HGLOBAL hGlobal;
	if ((hGlobal=GlobalAlloc(GMEM_MOVEABLE, szData))==NULL)
		return;

	LPVOID pGlobalMemory = GlobalLock(hGlobal);

	memcpy(pGlobalMemory, pData, szData);
	GlobalUnlock(hGlobal);

	COleDataSource::CacheGlobalData(cfFormat, hGlobal, lpFormatEtc);
}


BEGIN_INTERFACE_MAP(LFDataSource, COleDataSource)
	INTERFACE_PART(LFDataSource, IID_IDataObject, LFDataObject)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) LFDataSource::XLFDataObject::AddRef()
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) LFDataSource::XLFDataObject::Release()
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	return pThis->ExternalRelease();
}

STDMETHODIMP LFDataSource::XLFDataObject::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP LFDataSource::XLFDataObject::GetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium)
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	return pThis->m_xDataObject.GetData(lpFormatEtc, lpStgMedium);
}

STDMETHODIMP LFDataSource::XLFDataObject::GetDataHere(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium)
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	return pThis->m_xDataObject.GetDataHere(lpFormatEtc, lpStgMedium);
}

STDMETHODIMP LFDataSource::XLFDataObject::QueryGetData(LPFORMATETC lpFormatEtc)
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	return pThis->m_xDataObject.QueryGetData(lpFormatEtc);
}

STDMETHODIMP LFDataSource::XLFDataObject::GetCanonicalFormatEtc(LPFORMATETC lpFormatEtcIn, LPFORMATETC lpFormatEtcOut)
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	return pThis->m_xDataObject.GetCanonicalFormatEtc(lpFormatEtcIn, lpFormatEtcOut);
}

STDMETHODIMP LFDataSource::XLFDataObject::SetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium, BOOL bRelease)
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	HRESULT hResult = pThis->m_xDataObject.SetData(lpFormatEtc, lpStgMedium, bRelease);

	if ((hResult==DATA_E_FORMATETC) && (lpFormatEtc->tymed & (TYMED_HGLOBAL | TYMED_ISTREAM)) && (lpFormatEtc->cfFormat>=0xC000))
	{
		pThis->CacheData(lpFormatEtc->cfFormat, lpStgMedium, lpFormatEtc);

		return S_OK;
	}

	return hResult;
}

STDMETHODIMP LFDataSource::XLFDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC* ppenumFormatetc)
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	return pThis->m_xDataObject.EnumFormatEtc(dwDirection, ppenumFormatetc);
}

STDMETHODIMP LFDataSource::XLFDataObject::DAdvise(LPFORMATETC lpFormatEtc, DWORD advf, LPADVISESINK pAdvSink, LPDWORD pdwConnection)
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	return pThis->m_xDataObject.DAdvise(lpFormatEtc, advf, pAdvSink, pdwConnection);
}

STDMETHODIMP LFDataSource::XLFDataObject::DUnadvise(DWORD dwConnection)
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	return pThis->m_xDataObject.DUnadvise(dwConnection);
}

STDMETHODIMP LFDataSource::XLFDataObject::EnumDAdvise(LPENUMSTATDATA* ppenumAdvise)
{
	METHOD_PROLOGUE(LFDataSource, LFDataObject)

	return pThis->m_xDataObject.EnumDAdvise(ppenumAdvise);
}

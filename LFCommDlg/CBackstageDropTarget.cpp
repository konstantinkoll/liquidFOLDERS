
// CBackstageDropTarget.cpp: Implementierung der Klasse CBackstageDropTarget
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CBackstageDropTarget
//

CBackstageDropTarget::CBackstageDropTarget()
	: COleDropTarget()
{
	m_pDropTargetHelper = NULL;

	// Helper
	CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pDropTargetHelper));
}

CBackstageDropTarget::~CBackstageDropTarget()
{
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->Release();
}

DROPIMAGETYPE CBackstageDropTarget::DropEffectToDropImage(DROPEFFECT DropEffect)
{
	DropEffect &= ~DROPEFFECT_SCROLL;

	if (DropEffect==DROPEFFECT_NONE)
		return DROPIMAGE_NONE;

	if (DropEffect & DROPEFFECT_MOVE)
		return DROPIMAGE_MOVE;
	
	if (DropEffect & DROPEFFECT_COPY)
		return DROPIMAGE_COPY;
	
	if (DropEffect & DROPEFFECT_LINK)
		return DROPIMAGE_LINK;

	return DROPIMAGE_INVALID;
}

void CBackstageDropTarget::SetDropDescription(const DROPDESCRIPTION& DropDescription)
{
	ASSERT(m_lpDataObject);

	FORMATETC Format;
	ZeroMemory(&Format, sizeof(Format));
	Format.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DROPDESCRIPTION);
	Format.dwAspect = DVASPECT_CONTENT;
	Format.lindex = -1;
	Format.tymed = TYMED_HGLOBAL;

	STGMEDIUM Medium;
	ZeroMemory(&Medium, sizeof(Medium));
	Medium.tymed = TYMED_HGLOBAL;
	Medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE, sizeof(DROPDESCRIPTION));

	if (Medium.hGlobal)
	{
		*((DROPDESCRIPTION*)GlobalLock(Medium.hGlobal)) = DropDescription;
		GlobalUnlock(Medium.hGlobal);

		m_lpDataObject->SetData(&Format, &Medium, TRUE);
	}
}

void CBackstageDropTarget::SetDropDescription(DROPIMAGETYPE type, LPCWSTR lpszMessage, LPCWSTR lpszInsert)
{
	DROPDESCRIPTION DropDescription;

	DropDescription.type = type;
	wcscpy_s(DropDescription.szMessage, MAX_PATH, lpszMessage);
	wcscpy_s(DropDescription.szInsert, MAX_PATH, lpszInsert);

	SetDropDescription(DropDescription);
}

DROPEFFECT CBackstageDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD /*dwKeyState*/, CPoint point)
{
	// Drop target helper
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->DragEnter(pWnd->GetSafeHwnd(), pDataObject->m_lpDataObject, &point, DROPEFFECT_NONE);

	return DROPEFFECT_NONE;
}

DROPEFFECT CBackstageDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* /*pDataObject*/, DWORD /*dwKeyState*/, CPoint point)
{
	// Drop target helper
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->DragOver(&point, DROPEFFECT_NONE);

	return DROPEFFECT_NONE;
}

void CBackstageDropTarget::OnDragLeave(CWnd* pWnd)
{
	// Reset drop description
	SetDropDescription();

	// Drop target helper
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->DragLeave();

	COleDropTarget::OnDragLeave(pWnd);
}

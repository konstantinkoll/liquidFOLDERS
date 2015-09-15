
// LFDropSource.cpp: Implementierung der Klasse LFDropSource
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFDropSource
//

LFDropSource::LFDropSource()
{
	m_lRefCount = 1;
	m_LastEffect = DROPEFFECT_NONE;
}

DWORD LFDropSource::GetLastEffect()
{
	return m_LastEffect;
}

STDMETHODIMP LFDropSource::QueryInterface(REFIID iid, void** ppvObject)
{
	if ((iid==IID_IDropSource) || (iid==IID_IUnknown))
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE LFDropSource::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE LFDropSource::Release()
{
	LONG Count = InterlockedDecrement(&m_lRefCount);
	if (!Count)
	{
		delete this;
		return 0;
	}

	return Count;
}

STDMETHODIMP LFDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if (fEscapePressed)
		return DRAGDROP_S_CANCEL;

	if ((grfKeyState & MK_LBUTTON)==0)
		return DRAGDROP_S_DROP;

	return S_OK;
}

STDMETHODIMP LFDropSource::GiveFeedback(DWORD dwEffect)
{
	m_LastEffect = dwEffect;

	return DRAGDROP_S_USEDEFAULTCURSORS;
}

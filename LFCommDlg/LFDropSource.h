
// LFDropSource.h: Schnittstelle der Klasse LFDropSource
//

#pragma once


// LFDropSource
//

class AFX_EXT_CLASS LFDropSource : public IDropSource
{
public:
	LFDropSource();

	BEGIN_INTERFACE

	// IUnknown members
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release());

	// IDropSource members
	STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
	STDMETHOD(GiveFeedback)(DWORD dwEffect);

	END_INTERFACE

	DWORD GetLastEffect();

protected:
	LONG m_lRefCount;
	DWORD m_LastEffect;
};

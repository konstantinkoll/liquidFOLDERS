
// CBackstageDropTarget.h: Schnittstelle der Klasse CBackstageDropTarget
//

#pragma once


// CBackstageDropTarget
//

class CBackstageDropTarget : public COleDropTarget
{
public:
	CBackstageDropTarget();
	virtual ~CBackstageDropTarget();

	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);

protected:
	static DROPIMAGETYPE DropEffectToDropImage(DROPEFFECT DropEffect);
	void SetDropDescription(const DROPDESCRIPTION& DropDescription);
	void SetDropDescription(DROPIMAGETYPE type=DROPIMAGE_INVALID, LPCWSTR lpszMessage=L"", LPCWSTR lpszInsert=L"");
	void SetDropDescription(DROPEFFECT DropEffect, LPCWSTR lpszMessage, LPCWSTR lpszInsert);

	IDropTargetHelper* m_pDropTargetHelper;
};

inline void CBackstageDropTarget::SetDropDescription(DROPEFFECT DropEffect, LPCWSTR lpszMessage, LPCWSTR lpszInsert)
{
	SetDropDescription(DropEffectToDropImage(DropEffect), lpszMessage, lpszInsert);
}

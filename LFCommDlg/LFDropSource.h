
// LFDropSource.h: Schnittstelle der Klasse LFDropSource
//

#pragma once
#include "LFDataSource.h"


// LFDropSource
//

class LFDropSource : public COleDropSource
{
public:
	LFDropSource(LFDataSource* pDataSource);

	virtual SCODE QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState);
	virtual SCODE GiveFeedback(DROPEFFECT DropEffect);

protected:
	BOOL GetGlobalData(LPCWSTR lpszFormat, FORMATETC& FormatEtc, STGMEDIUM& StgMedium) const;
	DWORD GetGlobalDataDWord(LPCTSTR lpszFormat) const;
	BOOL SetDragImageCursor(DROPEFFECT DropEffect) const;

	LPDATAOBJECT p_DataObject;

private:
	BOOL m_SetCursor;
};

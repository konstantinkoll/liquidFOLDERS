
// CInspectorIconCtrl.h: Schnittstelle der Klasse CInspectorIconCtrl
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// CInspectorIconCtrl
//

#define StatusUnused            0
#define StatusUsed              1
#define StatusMultiple          2


class CInspectorIconCtrl : public CWnd
{
public:
	CInspectorIconCtrl();
	~CInspectorIconCtrl();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetStatus(UINT _status, HICON _icon=NULL, CString _description=_T(""));
	INT GetPreferredHeight(INT cx);

protected:
	UINT m_Status;
	HICON m_Icon;
	INT m_IconSize;
	CString m_Description;
	CGdiPlusBitmapResource* m_Empty;
	CGdiPlusBitmapResource* m_Multiple;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	DECLARE_MESSAGE_MAP()

private:
	CString m_Description_Unused;
};


// CInspectorIconCtrl.h: Schnittstelle der Klasse CInspectorIconCtrl
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// CInspectorIconCtrl
//

#define IconEmpty         0
#define IconMultiple      1
#define IconCore          2
#define IconExtension     3
#define IconPreview       4

class CInspectorIconCtrl : public CWnd
{
public:
	CInspectorIconCtrl();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetEmpty();
	void SetMultiple(CString Description=_T(""));
	void SetCoreIcon(INT IconID, CString Description=_T(""));
	void SetFormatIcon(CHAR* FileFormat, CString Description=_T(""));
	//void SetPreview(, CString Description=_T(""));		TODO
	INT GetPreferredHeight();

protected:
	CGdiPlusBitmapResource m_Empty;
	CGdiPlusBitmapResource m_Multiple;
	CString m_strUnused;
	CString m_strDescription;
	UINT m_Status;
	INT m_IconID;
	CHAR m_FileFormat[LFExtSize];

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};

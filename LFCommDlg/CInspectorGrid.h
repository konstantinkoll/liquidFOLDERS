
// CInspectorGrid: Schnittstelle der Klasse CInspectorGrid
//

#pragma once
#include "liquidFOLDERS.h"


// CInspectorGrid
//

class AFX_EXT_CLASS CInspectorGrid : public CMFCPropertyGridCtrl
{
friend class CAttributeProperty;

public:
	CInspectorGrid();

	CFont* GetBoldFnt();
	CFont* GetItalicFnt();
	int GetLeftMargin();

protected:
	CFont m_fontItalic;

	afx_msg LRESULT OnSetFont(WPARAM, LPARAM);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	void CreateItalicFont();
};

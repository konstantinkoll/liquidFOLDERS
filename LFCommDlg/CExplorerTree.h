
// CExplorerTree: Schnittstelle der Klasse CExplorerTree
//

#pragma once


// CExplorerTree
//

class AFX_EXT_CLASS CExplorerTree : public CMFCShellTreeCtrl
{
public:
	CExplorerTree();

	BOOL CExplorerTree::Create(CWnd* pParentWnd, UINT nID);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
};

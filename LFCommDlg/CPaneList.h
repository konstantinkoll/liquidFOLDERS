
// CPaneList: Schnittstelle der Klasse CPaneList
//

#pragma once
#include "liquidFOLDERS.h"


// CPaneList
//

class AFX_EXT_CLASS CPaneList : public CListCtrl
{
public:
	CPaneList();

	virtual BOOL SetWindowPos(const CWnd* pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags);

	void SetTileSize(int cx=-1);
	void SetContextMenu(UINT _MenuResID);
	void EnableGroupView(BOOL fEnable);

protected:
	void DrawItem(int nID, CDC* pDC, CMFCVisualManager* dm);

	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	DECLARE_MESSAGE_MAP()

private:
	OSVERSIONINFO osInfo;
	UINT MenuResID;
	int LastWidth;
	int Editing;
};

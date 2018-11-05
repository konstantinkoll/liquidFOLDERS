
// CAbstractFileView: Schnittstelle der Klasse CAbstractFileView
//

#pragma once
#include "CFrontstageItemView.h"
#include "LFCore.h"


// CAbstractFileView
//

#define WM_RENAMEITEM     WM_USER+6

class CAbstractFileView : public CFrontstageItemView
{
public:
	CAbstractFileView(UINT Flags=FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM, SIZE_T szData=sizeof(ItemData), const CSize& szItemInflate=CSize(0, 0));

	virtual void EditLabel(INT Index);

	BOOL IsEditing() const;
	void UpdateSearchResult(LFSearchResult* pCookedFiles);

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void SetSearchResult(LFSearchResult* pCookedFiles);
	virtual void ShowTooltip(const CPoint& point);
	virtual INT GetItemCategory(INT Index) const;
	virtual LFFont* GetLabelFont() const;
	virtual RECT GetLabelRect(INT Index) const;

	void FinishUpdate(BOOL InternalCall=FALSE);
	void AddHeaderColumns(BOOL Shadow=FALSE);
	void DestroyEdit(BOOL Accept=FALSE);

	afx_msg void OnDestroy();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);

	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnDestroyEdit();
	DECLARE_MESSAGE_MAP()

	LFSearchResult* p_CookedFiles;

private:
	UINT m_FlagsMask;
	CEdit* m_pWndEdit;
	WCHAR m_TypingBuffer[256];
	DWORD m_TypingTicks;
};

inline void CAbstractFileView::AddHeaderColumns(BOOL Shadow)
{
	for (UINT a=0; a<LFAttributeCount; a++)
		VERIFY(AddHeaderColumn(Shadow));
}

inline BOOL CAbstractFileView::IsEditing() const
{
	return m_pWndEdit!=NULL;
}

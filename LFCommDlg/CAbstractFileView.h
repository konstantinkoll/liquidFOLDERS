
// CAbstractFileView: Schnittstelle der Klasse CAbstractFileView
//

#pragma once
#include "CFrontstageItemView.h"
#include "LFCore.h"


// CAbstractFileView
//

#define WM_RENAMEITEM     WM_USER+7

class CAbstractFileView : public CFrontstageItemView
{
public:
	CAbstractFileView(UINT Flags=FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM, SIZE_T szData=sizeof(ItemData), const CSize& szItemInflate=CSize(0, 0));

	void UpdateSearchResult(LFSearchResult* pCookedFiles);

protected:
	virtual INT GetItemCategory(INT Index) const;
	virtual void SetSearchResult(LFSearchResult* pCookedFiles);
	virtual void ShowTooltip(const CPoint& point);
	virtual COLORREF GetItemTextColor(INT Index, BOOL Themed) const;
	virtual BOOL AllowItemEditLabel(INT Index) const;
	virtual CEdit* CreateLabelEditControl();
	virtual void EndLabelEdit(INT Index, CString& Value);
	virtual void DestroyEdit(BOOL Accept=FALSE);

	void AddHeaderColumns(BOOL Shadow=FALSE);
	void FinishUpdate(BOOL InternalCall=FALSE);

	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	DECLARE_MESSAGE_MAP()

	LFSearchResult* p_CookedFiles;
	BOOL m_ShowCompColor;

private:
	UINT m_FlagsMask;
	WCHAR m_TypingBuffer[256];
	DWORD m_TypingTicks;
};

inline void CAbstractFileView::AddHeaderColumns(BOOL Shadow)
{
	for (UINT a=0; a<LFAttributeCount; a++)
		VERIFY(AddHeaderColumn(Shadow));
}

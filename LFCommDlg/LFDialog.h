
// LFDialog.h: Schnittstelle der Klasse LFDialog
//

#pragma once
#include "CBackstageWnd.h"
#include "CCategory.h"
#include "CDesktopDimmer.h"
#include "CHoverButton.h"
#include "LFDynArray.h"


// LFDialog
//

struct DialogControl
{
	CWnd* pChildWnd;
	RECT rectClient;
};

class LFDialog : public CBackstageWnd
{
public:
	LFDialog(UINT nIDTemplate, CWnd* pParentWnd=NULL, BOOL WantsBitmap=FALSE, BOOL UAC=FALSE);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create();
	INT_PTR DoModal();
	void EndDialog(INT nResult);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual void PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);
	virtual BOOL InitDialog();

	void MapDialogRect(LPRECT lpRect) const;
	void NextDlgCtrl() const;
	void PrevDlgCtrl() const;
	void GotoDlgCtrl(CWnd* pChildWnd);
	void SetDefID(UINT nID);
	DWORD GetDefID() const;
	CWnd* GetBottomWnd() const;
	void SetBottomLeftControl(CWnd* pChildWnd);
	void SetBottomLeftControl(UINT nID);
	void AddBottomRightControl(CWnd* pChildWnd);
	void AddBottomRightControl(UINT nID);

	static BOOL CompareClassName(LPCTSTR lpszClassName1, LPCTSTR lpszClassName2);
	static BOOL CompareClassName(HWND hWnd, LPCTSTR lpszClassName);

	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	DECLARE_MESSAGE_MAP()

	LPCTSTR m_lpszTemplateName;
	CWnd* p_ParentWnd;
	CCategory m_wndCategory[12];
	BOOL m_UAC;

private:
	static BOOL IsPushbutton(CWnd* pWnd);

	HWND hWndTop;
	HICON hIconShield;
	INT m_ShieldSize;
	INT m_UACHeight;
	LFDynArray<CHoverButton*, 8, 8> m_Buttons;
	LFDynArray<DialogControl, 2, 2> m_BottomRightControls;
	DialogControl m_BottomLeftControl;
	CPoint m_LastSize;
	CDesktopDimmer m_wndDesktopDimmer;
};

inline void LFDialog::MapDialogRect(LPRECT lpRect) const
{
	ASSERT(::IsWindow(m_hWnd));

	::MapDialogRect(m_hWnd, lpRect);
}

inline void LFDialog::NextDlgCtrl() const
{
	ASSERT(::IsWindow(m_hWnd));

	::SendMessage(m_hWnd, WM_NEXTDLGCTL, 0, 0);
}

inline void LFDialog::PrevDlgCtrl() const
{
	ASSERT(::IsWindow(m_hWnd));

	::SendMessage(m_hWnd, WM_NEXTDLGCTL, 1, 0);
}

inline void LFDialog::GotoDlgCtrl(CWnd* pChildWnd)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(pChildWnd);

	::SendMessage(m_hWnd, WM_NEXTDLGCTL, (WPARAM)pChildWnd->m_hWnd, 1L);
}

inline void LFDialog::SetDefID(UINT nID)
{
	ASSERT(::IsWindow(m_hWnd));

	::SendMessage(m_hWnd, DM_SETDEFID, nID, 0);
}

inline DWORD LFDialog::GetDefID() const
{
	ASSERT(::IsWindow(m_hWnd));

	return DWORD(::SendMessage(m_hWnd, DM_GETDEFID, 0, 0));
}

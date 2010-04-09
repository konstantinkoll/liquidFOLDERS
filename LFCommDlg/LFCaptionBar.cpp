
#pragma once
#include "stdafx.h"
#include "LFCaptionBar.h"
#include "afxvisualmanager.h"
#include "afxtoolbar.h"
#include "afxtrackmouse.h"
#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"
#include "afxmdichildwndex.h"
#include "afxtooltipmanager.h"
#include "afxtooltipctrl.h"
#include "afxribbonres.h"

#define nMessageBarMargin 4


LFCaptionBar::LFCaptionBar()
{
	m_pToolTip = NULL;

	m_clrBarText = (COLORREF)-1;
	m_clrBarBackground = (COLORREF)-1;
	m_clrBarBorder = (COLORREF)-1;

	m_nBorderSize = 4;
	m_nMargin = 4;

	m_nDefaultHeight = -1;
	m_nCurrentHeight = 0;

	m_bIsCloseBtnPressed = FALSE;
	m_bIsCloseBtnHighlighted= FALSE;
	m_bCloseTracked = FALSE;

	m_rectClose.SetRectEmpty();
	m_rectInfo.SetRectEmpty();
}

LFCaptionBar::~LFCaptionBar()
{
}

BOOL LFCaptionBar::Create(DWORD dwStyle, CWnd* pParentWnd, UINT uID, int nHeight)
{
	ENSURE(AfxIsExtendedFrameClass(pParentWnd) || pParentWnd->IsKindOf(RUNTIME_CLASS(CDialog)));

	SetPaneStyle(CBRS_ALIGN_TOP);
	m_nDefaultHeight = max(nHeight, 24);
	m_dwStyle |= CBRS_HIDE_INPLACE;

	if (!CPane::Create(NULL, dwStyle, CRect(0, 0, 0, 0), pParentWnd, uID, 0))
		return FALSE;

	if (pParentWnd->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		((CFrameWndEx*) pParentWnd)->AddPane(this);
	}
	else if (pParentWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		((CMDIFrameWndEx*) pParentWnd)->AddPane(this);
	}
	else if (pParentWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		((CMDIChildWndEx*) pParentWnd)->AddPane(this);
	}
	else
	{
		ASSERT(FALSE);
	}

	m_nBorderSize = 0;

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFCaptionBar, CPane)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_REGISTERED_MESSAGE(AFX_WM_UPDATETOOLTIPS, OnUpdateToolTips)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnNeedTipText)
END_MESSAGE_MAP()

void LFCaptionBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}

int LFCaptionBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPane::OnCreate(lpCreateStruct)==-1)
		return -1;

	CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_CAPTIONBAR);

	if (m_pToolTip->GetSafeHwnd())
	{
		CRect rectDummy(0, 0, 0, 0);
		m_pToolTip->SetMaxTipWidth(640);
		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, 1);
	}

	SetWindowText(_T("Workflow Bar"));
	return 0;
}

void LFCaptionBar::OnSize(UINT nType, int cx, int cy)
{
	CPane::OnSize(nType, cx, cy);
	RecalcLayout();
	InvalidateRect(NULL);
}

void LFCaptionBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	lpncsp->rgrc[0].bottom -= m_nBorderSize;
	lpncsp->rgrc[0].top += m_nBorderSize;
}

void LFCaptionBar::OnPaint()
{
	CPaintDC dcPaint(this);
	CMemDC memDC(dcPaint, this);
	CDC& dc = memDC.GetDC();

	CRect rectClient;
	GetClientRect(rectClient);

	OnDrawBackground(&dc, rectClient);

	int nOldBkMode = dc.SetBkMode(TRANSPARENT);
	
	if (!m_rectClose.IsRectEmpty())
	{
		COLORREF clrText = CMFCVisualManager::GetInstance()->OnFillCaptionBarButton(&dc, (CMFCCaptionBar*)this, m_rectClose, m_bIsCloseBtnPressed, m_bIsCloseBtnHighlighted, FALSE, FALSE, TRUE);

		CMenuImages::IMAGE_STATE imageState;

		if (GetRValue(clrText) > 192 && GetGValue(clrText) > 192 && GetBValue(clrText) > 192)
		{
			imageState = CMenuImages::ImageWhite;
		}
		else
		{
			imageState = CMenuImages::ImageBlack;
		}

		CMenuImages::Draw(&dc, CMenuImages::IdClose, m_rectClose, imageState);

		CMFCVisualManager::GetInstance()->OnDrawCaptionBarButtonBorder(&dc, (CMFCCaptionBar*)this, m_rectClose, m_bIsCloseBtnPressed, m_bIsCloseBtnHighlighted, FALSE, FALSE, TRUE);
	}

	dc.SetBkMode(nOldBkMode);
}

void LFCaptionBar::OnNcPaint()
{
	CWindowDC dcWin(this);

	CRect rectClient;
	GetClientRect(rectClient);

	CRect rectWindow;
	GetWindowRect(rectWindow);

	CRect rectBorder = rectWindow;

	ScreenToClient(rectWindow);

	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dcWin.ExcludeClipRect(rectClient);

	rectBorder.OffsetRect(-rectBorder.left, -rectBorder.top);

	int nTop = rectBorder.top;
	rectBorder.top = rectBorder.bottom-m_nBorderSize;
	OnDrawBorder(&dcWin, rectBorder);

	rectBorder.top = nTop;
	rectBorder.bottom = rectBorder.top+m_nBorderSize;

	OnDrawBorder(&dcWin, rectBorder);
	dcWin.SelectClipRgn(NULL);
}

BOOL LFCaptionBar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void LFCaptionBar::OnDrawBackground(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	CMFCVisualManager::GetInstance()->OnFillBarBackground(pDC, this, rect, rect);
	CMFCVisualManager::GetInstance()->OnDrawCaptionBarInfoArea(pDC, (CMFCCaptionBar*)this, m_rectInfo);
}

void LFCaptionBar::OnDrawBorder(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	rect.InflateRect(2, 0);
	CMFCVisualManager::GetInstance()->OnDrawCaptionBarBorder(pDC, (CMFCCaptionBar*)this, rect, m_clrBarBorder, TRUE);
}

CSize LFCaptionBar::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	RecalcLayout();
	return CSize(32767, m_nCurrentHeight);
}

void LFCaptionBar::RecalcLayout()
{
	m_nCurrentHeight = m_nDefaultHeight+2*nMessageBarMargin;

	CRect rectClient;
	GetClientRect(rectClient);
	if (rectClient.IsRectEmpty())
		return;

	CSize sizeMenuImage = CMenuImages::Size();
	sizeMenuImage.cx += 2*nMessageBarMargin;
	sizeMenuImage.cy += 2*nMessageBarMargin;

	m_rectClose = CRect(CPoint(rectClient.right-sizeMenuImage.cx-nMessageBarMargin, rectClient.top+nMessageBarMargin), sizeMenuImage);

	rectClient.DeflateRect(nMessageBarMargin, nMessageBarMargin);
	rectClient.right -= m_rectClose.Width();
	m_rectInfo = rectClient;

	UpdateTooltips();
}

void LFCaptionBar::AdjustLayout()
{
	if (!GetSafeHwnd())
		return;

	CFrameWnd* pParent = GetParentFrame();
	if (pParent)
		if (pParent->GetSafeHwnd())
			pParent->RecalcLayout();

	RecalcLayout();
}

void LFCaptionBar::OnDestroy()
{
	CTooltipManager::DeleteToolTip(m_pToolTip);
	CPane::OnDestroy();
}

void LFCaptionBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CPane::OnLButtonDown(nFlags, point);

	if (m_bIsCloseBtnHighlighted)
	{
		m_bIsCloseBtnPressed = TRUE;
		InvalidateRect(m_rectClose);
		UpdateWindow();
	}
}

void LFCaptionBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	CPane::OnLButtonUp(nFlags, point);

	if (m_bIsCloseBtnPressed)
	{
		m_bIsCloseBtnPressed = FALSE;
		m_bIsCloseBtnHighlighted = FALSE;

		InvalidateRect(m_rectClose);
		UpdateWindow();

		ShowPane(FALSE, FALSE, FALSE);
	}
}

void LFCaptionBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CPane::OnMouseMove(nFlags, point);

	BOOL bTrack = FALSE;

	if (!m_rectClose.IsRectEmpty())
	{
		BOOL bIsBtnHighlighted = m_rectClose.PtInRect(point);

		if (m_bIsCloseBtnHighlighted!=bIsBtnHighlighted)
		{
			m_bIsCloseBtnHighlighted = bIsBtnHighlighted;
			m_bIsCloseBtnPressed = (nFlags & MK_LBUTTON) && m_bIsCloseBtnHighlighted;

			InvalidateRect(m_rectClose);
			UpdateWindow();

			bTrack = bIsBtnHighlighted;
		}
	}

	if (!m_bTracked)
	{
		m_bTracked = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = GetSafeHwnd();
		tme.dwHoverTime = HOVER_DEFAULT;
		::AFXTrackMouse(&tme);
	}
}

afx_msg LRESULT LFCaptionBar::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (m_bIsCloseBtnPressed || m_bIsCloseBtnHighlighted)
	{
		m_bIsCloseBtnPressed = FALSE;
		m_bIsCloseBtnHighlighted = FALSE;

		InvalidateRect(m_rectClose);
		UpdateWindow();
	}

	return 0;
}

void LFCaptionBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (!CMFCToolBar::IsCustomizeMode())
	{
		ASSERT_VALID(GetOwner());

		ClientToScreen(&point);
		OnPaneContextMenu(GetOwner(), point);
		return;
	}

	CPane::OnRButtonUp(nFlags, point);
}

BOOL LFCaptionBar::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;
	ENSURE(pNMH);

	if ((!m_pToolTip->GetSafeHwnd()) || (pNMH->hwndFrom!=m_pToolTip->GetSafeHwnd()))
		return FALSE;

	if (CMFCPopupMenu::GetActiveMenu())
		return FALSE;

	LPNMTTDISPINFO pTTDispInfo = (LPNMTTDISPINFO)pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND)==0);

	ENSURE(strTipText.LoadString(IDS_AFXBARRES_CLOSEBAR));
	if (strTipText.IsEmpty())
		return TRUE;

	pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR)strTipText);
	return TRUE;
}

BOOL LFCaptionBar::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		if (m_pToolTip->GetSafeHwnd())
			m_pToolTip->RelayEvent(pMsg);
	}

	return CPane::PreTranslateMessage(pMsg);
}

LRESULT LFCaptionBar::OnUpdateToolTips(WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT)wp;

	if (nTypes & AFX_TOOLTIP_TYPE_CAPTIONBAR)
	{
		CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_CAPTIONBAR);

		CRect rectDummy(0, 0, 0, 0);
		m_pToolTip->SetMaxTipWidth(640);
		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, 1);
	}

	return 0;
}

void LFCaptionBar::UpdateTooltips()
{
	if (m_pToolTip->GetSafeHwnd())
		m_pToolTip->SetToolRect(this, 1, m_rectClose);
}

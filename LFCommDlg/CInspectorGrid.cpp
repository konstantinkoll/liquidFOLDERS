
// CInspectorGrid.cpp: Implementierung der Klasse CInspectorGrid
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CInspectorGrid

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CInspectorGrid::CInspectorGrid()
	: CMFCPropertyGridCtrl()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = LFCommDlgDLL.hModule;

	if (!(::GetClassInfo(hInst, L"CInspectorGrid", &wndcls)))
	{
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = (HBRUSH)(COLOR_3DFACE);
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = L"CInspectorGrid";

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	m_bVSDotNetLook = TRUE;
	m_nLeftColumnWidth = 150;
}

CFont* CInspectorGrid::GetBoldFnt()
{
	return &m_fontBold;
}

CFont* CInspectorGrid::GetItalicFnt()
{
	return &m_fontItalic;
}

int CInspectorGrid::GetLeftMargin()
{
	return m_nEditLeftMargin;
}

void CInspectorGrid::CreateItalicFont()
{
	if (m_fontItalic.GetSafeHandle())
		m_fontItalic.DeleteObject();

	CFont* pFont = CFont::FromHandle(m_hFont ? m_hFont : (HFONT)::GetStockObject(DEFAULT_GUI_FONT));
	ASSERT_VALID(pFont);

	LOGFONT lf;
	ZeroMemory(&lf, sizeof(LOGFONT));
	pFont->GetLogFont(&lf);
	lf.lfItalic = true;
	m_fontItalic.CreateFontIndirect(&lf);
}


BEGIN_MESSAGE_MAP(CInspectorGrid, CMFCPropertyGridCtrl)
	ON_WM_NCPAINT()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CInspectorGrid::OnNcPaint()
{
	DrawControlBorder(this);
}

LRESULT CInspectorGrid::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	LRESULT res = CMFCPropertyGridCtrl::OnSetFont(wParam, lParam);

	CreateItalicFont();

	return res;
}

void CInspectorGrid::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
}

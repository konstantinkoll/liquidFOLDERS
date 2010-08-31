
// CExplorerList.cpp: Implementierung der Klasse CExplorerList
//

#include "stdafx.h"
#include "CExplorerList.h"


// CExplorerList
//

CExplorerList::CExplorerList()
	: CListCtrl()
{
	p_App = (LFApplication*)AfxGetApp();
	p_FooterHandler = NULL;
	hTheme = NULL;
}

void CExplorerList::EnableTheming()
{
	if ((p_App->m_ThemeLibLoaded) && (p_App->OSVersion>=OS_Vista))
	{
		p_App->zSetWindowTheme(GetSafeHwnd(), L"explorer", NULL);
		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}
}


BEGIN_MESSAGE_MAP(CExplorerList, CListCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
END_MESSAGE_MAP()

int CExplorerList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	EnableTheming();

	if (p_App->OSVersion>=OS_Vista)
		SendMessage(0x10BD, (WPARAM)&IID_IListViewFooter, (LPARAM)&p_FooterHandler);

	return 0;
}

void CExplorerList::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);

	CListCtrl::OnDestroy();
}

LRESULT CExplorerList::OnThemeChanged()
{
	if ((p_App->m_ThemeLibLoaded) && (p_App->OSVersion>=OS_Vista))
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(m_hWnd, VSCLASS_LISTVIEW);
	}

	return TRUE;
}

BOOL CExplorerList::SupportsFooter()
{
	return (p_FooterHandler!=NULL);
}

void CExplorerList::ShowFooter(IListViewFooterCallback* pCallbackObject)
{
	if (p_FooterHandler)
		p_FooterHandler->Show(pCallbackObject);
}

void CExplorerList::RemoveFooter()
{
	if (p_FooterHandler)
	{
		BOOL Visible;
		p_FooterHandler->IsVisible(&Visible);

		if (Visible)
			p_FooterHandler->RemoveAllButtons();
	}
}

void CExplorerList::SetFooterText(LPCWSTR pText)
{
	if (p_FooterHandler)
		p_FooterHandler->SetIntroText(pText);
}

void CExplorerList::InsertFooterButton(int insertAt, LPCWSTR pText, LPCWSTR pUnknown, UINT iconIndex, LONG lParam)
{
	if (p_FooterHandler)
		p_FooterHandler->InsertButton(insertAt, pText, pUnknown, iconIndex, lParam);
}

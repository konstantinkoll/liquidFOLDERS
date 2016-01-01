
// CBackstageShadow.h: Schnittstelle der Klasse CBackstageShadow
//

#pragma once


// CBackstageShadow
//

class CBackstageShadow
{
public:
	CBackstageShadow();
	~CBackstageShadow();

	BOOL Create();
	void Update(CWnd* pBackstageWnd, CRect rectWindow);
	void Update(CWnd* pBackstageWnd);

protected:
	void Update(UINT nID, CDC& dc, POINT ptSrc, SIZE szWindow, CWnd* pBackstageWnd, const CRect& rectWindow);

	INT m_Width;
	INT m_Height;
	CWnd m_wndShadow[4];
	POINT m_wndTopLeft[4];
};

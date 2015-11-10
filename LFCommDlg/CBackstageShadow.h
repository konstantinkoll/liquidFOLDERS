
// CBackstageShadow.h: Schnittstelle der Klasse CBackstageShadow
//

#pragma once


// CBackstageShadow
//

class CBackstageShadow : public CWnd
{
public:
	CBackstageShadow();

	BOOL Create();
	void Update(CWnd* pBackstageWnd, CRect rectWindow);
	void Update(CWnd* pBackstageWnd);

protected:
	INT m_Width;
	INT m_Height;
};

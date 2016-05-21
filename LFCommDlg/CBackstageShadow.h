
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
	static BYTE CalcOpacity(UINT Proximity);
	static BYTE CalcOpacity(DOUBLE Proximity);

	INT m_Width;
	INT m_Height;
	CWnd m_wndShadow[4];
	POINT m_wndTopLeft[4];

private:
	static void HorizontalLine(const BITMAP& Bitmap, UINT Row, UINT Width, BYTE Opacity);
	static void VerticalLine(const BITMAP& Bitmap, UINT Column, UINT Height, BYTE Opacity);
	static void CornersTop(const BITMAP& Bitmap, UINT Width);
	static void CornersBottom(const BITMAP& Bitmap, UINT Width, UINT Height);

	static BYTE* m_pShadowCorner;
};

inline BYTE CBackstageShadow::CalcOpacity(UINT Proximity)
{
	return (BYTE)((Proximity+4)*(Proximity+4)*(Proximity+5)>>6);
}

inline BYTE CBackstageShadow::CalcOpacity(DOUBLE Proximity)
{
	return (Proximity<0.0) ? 0 : (BYTE)((Proximity+4.0)*(Proximity+4.0)*(Proximity+5.0)/64.0);
}

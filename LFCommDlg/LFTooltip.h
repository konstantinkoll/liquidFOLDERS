
// LFTooltip.h: Schnittstelle der Klasse LFTooltip
//

#pragma once


// LFTooltip
//

class LFTooltip : public CWnd
{
public:
	LFTooltip();

	BOOL Create();

	void ShowTooltip(const CPoint& point, const CString& strCaption, const CString& strText, HICON hIcon=NULL, HBITMAP hBitmap=NULL);
	void HideTooltip();

	static void AppendAttribute(CString& Str, const CString& Name, const CString& Value);
	static void AppendAttribute(CString& Str, UINT ResID, const CString& Value);
	static void AppendAttribute(CString& Str, UINT ResID, LPCSTR pValue);
	static void AppendAttribute(LPWSTR pStr, SIZE_T cCount, const CString& Name, const CString& Value);
	static void AppendAttribute(LPWSTR pStr, SIZE_T cCount, UINT ResID, const CString& Value);
	static void AppendAttribute(LPWSTR pStr, SIZE_T cCount, UINT ResID, const LPCSTR pValue);

protected:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	CRect m_ContentRect;
};

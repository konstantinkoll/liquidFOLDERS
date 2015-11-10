
// LFFont.h: Schnittstelle der Klasse LFFont
//

#pragma once


// LFFont
//

class LFFont : public CFont
{
public:
	LFFont();

	BOOL CreateFont(INT nHeight, BYTE nQuality=CLEARTYPE_QUALITY, INT nWeight=FW_NORMAL, BYTE bItalic=0, LPCTSTR lpszFacename=_T("Arial"));
	BOOL CreateFontIndirect(const LOGFONT* lpLogFont);
	INT GetFontHeight() const;
	CSize GetTextExtent(LPCTSTR lpszString) const;
	CSize GetTextExtent(const CString& str) const;

protected:
	void CalcFontHeight();

	INT m_FontHeight;

private:
#pragma warning(push)
#pragma warning(disable: 4516)
	CFont::CreateFontIndirect;
	CFont::CreateFont;
	CFont::CreatePointFont;
	CFont::CreatePointFontIndirect;
#pragma warning(pop)
};

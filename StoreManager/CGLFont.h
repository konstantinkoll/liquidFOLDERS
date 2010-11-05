
class CGLFont
{
public:
	CGLFont();
	~CGLFont();

	BOOL Create(CString face, UINT size, BOOL bold, BOOL italic);
	BOOL Create(CFont* font);
	UINT Render(WCHAR* pStr, INT x, INT y, INT cCount=-1);
	UINT GetTextWidth(WCHAR* pStr, INT cCount=-1);
	UINT GetTextHeight(WCHAR* pStr);

protected:
	BOOL Initialize(HFONT hFont);
	UINT RenderChar(UCHAR ch, INT x, INT y, UINT* pHeight);

private:
	float TexCoords[256-32][4];
	UINT m_Spacing;
	UINT m_LineHeight;
	UINT m_TexSize;
	UINT m_TexID;

	enum PaintResult
	{
		Fail,
		MoreData,
		Success
	};
	PaintResult PaintAlphabet(HDC hDC, BOOL bMeasureOnly=FALSE);
};

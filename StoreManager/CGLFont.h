
class CGLFont
{
public:
	CGLFont();
	~CGLFont();

	BOOL Create(CString face, UINT size, BOOL bold, BOOL italic);
	BOOL Create(CFont* font);
	UINT Render(wchar_t* pStr, float x, float y, int cCount=-1);
	UINT GetTextWidth(wchar_t* pStr, int cCount=-1);
	UINT GetTextHeight(wchar_t* pStr);

protected:
	BOOL Initialize(HFONT hFont);
	float RenderChar(UCHAR ch, float x, float y, float* pHeight);

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

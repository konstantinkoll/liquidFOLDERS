
// CCaptionBar.h: Schnittstelle der Klasse CCaptionBar
//


// CCapationBar
//

class AFX_EXT_CLASS CCaptionBar : public CMFCCaptionBar
{
public:
	CCaptionBar();
	virtual ~CCaptionBar();

	virtual BOOL LoadState(LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState(LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

	void SetIcon(LPCWSTR Icon, BarElementAlignment iconAlignment);
};


// CCaptionBar.h: Schnittstelle der Klasse CCaptionBar
//


// CCapationBar
//

class CCaptionBar : public CMFCCaptionBar
{
public:
	CCaptionBar();
	~CCaptionBar();

	virtual BOOL LoadState(LPCTSTR lpszProfileName = NULL, INT nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState(LPCTSTR lpszProfileName = NULL, INT nIndex = -1, UINT uiID = (UINT) -1);

	void SetIcon(LPCWSTR Icon, BarElementAlignment iconAlignment);
};

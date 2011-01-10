
// CPIDLSelector.h: Schnittstelle der Klasse CPIDLSelector
//

#include "LFCommDlg.h"


// CPIDLDropdownWindow
//

class CPIDLDropdownWindow : public CDropdownWindow
{
public:
	void PopulateList();

protected:
	BOOL AddPIDL(LPITEMIDLIST pidl, UINT Category, BOOL FreeOnFail=TRUE);
	void AddKnownFolder(REFKNOWNFOLDERID rfid, UINT Category);
	void AddPath(WCHAR* Path, UINT Category);
	void AddCSIDL(INT ID, UINT Category);
	void AddChildren(WCHAR* Path, UINT Category);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChooseFolder();
	DECLARE_MESSAGE_MAP()
};


// CPIDLSelector
//

class CPIDLSelector : public CDropdownSelector
{
public:
	CPIDLSelector();
	~CPIDLSelector();

	virtual void CreateDropdownWindow();
	virtual void SetEmpty(BOOL Repaint=TRUE);
	virtual void GetTooltipData(HICON& hIcon, CSize& size, CString& caption, CString& hint);

	void SetItem(LPITEMIDLIST _pidl, BOOL Repaint=TRUE, UINT NotifyCode=NM_SELCHANGED);
	void SetItem(IShellFolder* pDesktop, WCHAR* Path, BOOL Repaint=TRUE, UINT NotifyCode=NM_SELCHANGED);

	LPITEMIDLIST pidl;

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnSetItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShellChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	ULONG m_ulSHChangeNotifyRegister;
};


// CPIDLSelector.h: Schnittstelle der Klasse CPIDLSelector
//

#include "LFCommDlg.h"


// CPIDLDropdownWindow
//

class CPIDLDropdownWindow : public CDropdownWindow
{
public:
	CPIDLDropdownWindow();

	void PopulateList();

protected:
	BOOL AddPIDL(LPITEMIDLIST pidl, UINT Category, BOOL FreeOnFail=TRUE);
	void AddKnownFolder(REFKNOWNFOLDERID rfid, UINT Category);
	void AddPath(wchar_t* Path, UINT Category);
	void AddCSIDL(int ID, UINT Category);
	void AddChildren(wchar_t* Path, UINT Category);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
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

	void SetItem(LPITEMIDLIST _pidl, BOOL Repaint=TRUE);

	LPITEMIDLIST pidl;

protected:
	afx_msg LRESULT OnSetItem(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};


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
	CImageList il;

	void AddPIDL(LPITEMIDLIST pidl, UINT Category);
	void AddKnownFolder(REFKNOWNFOLDERID rfid, UINT Category);
	void AddPath(wchar_t* Path, UINT Category);
	void AddCSIDL(int ID, UINT Category);
	void AddChildren(wchar_t* Path, UINT Category);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnChooseFolder();
	DECLARE_MESSAGE_MAP()
};


// CPIDLSelector
//

class CPIDLSelector : public CDropdownSelector
{
public:
	CPIDLSelector();

	virtual void CreateDropdownWindow();
};

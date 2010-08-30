
// CPIDLSelector.h: Schnittstelle der Klasse CPIDLSelector
//

#include "LFCommDlg.h"


// CPIDLDropdownWindow
//

class CPIDLDropdownWindow : public CDropdownWindow
{
public:
	CPIDLDropdownWindow();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
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

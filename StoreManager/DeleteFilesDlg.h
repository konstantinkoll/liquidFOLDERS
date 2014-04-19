
// DeleteFilesDlg.h: Schnittstelle der Klasse DeleteFilesDlg
//

#pragma once
#include "LFCommDlg.h"


// DeleteFilesDlg
//

class DeleteFilesDlg : public LFDialog
{
public:
	DeleteFilesDlg(CWnd* pParentWnd);

	virtual void DoDataExchange(CDataExchange* pDX);

	BOOL m_Delete;

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CFont BoldFont;
};

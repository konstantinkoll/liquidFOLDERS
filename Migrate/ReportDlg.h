
// ReportDlg.h: Schnittstelle der Klasse ReportDlg
//

#pragma once
#include "LFCommDlg.h"
#include "Migrate.h"
#include "CMigrationList.h"


// ReportDlg
//

typedef DynArray<ML_Entry*> ReportList;

class ReportDlg : public CDialog
{
public:
	ReportDlg(CWnd* pParent, ReportList* Successful, ReportList* WithErrors);

	ReportList* m_Lists[2];

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CImageListTransparent m_Icons;
};

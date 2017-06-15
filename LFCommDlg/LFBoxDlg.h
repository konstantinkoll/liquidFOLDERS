
// LFBoxDlg.h: Schnittstelle der Klasse LFBoxDlg
//

#pragma once
#include "LFDialog.h"
#include "Box.h"


// LFBoxDlg
//

class LFBoxDlg : public LFDialog
{
public:
	LFBoxDlg(const Box& Box, CWnd* pParentWnd=NULL);

	WCHAR m_FolderPath[MAX_PATH];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

private:
	CItemPanel m_wndPanel;
};

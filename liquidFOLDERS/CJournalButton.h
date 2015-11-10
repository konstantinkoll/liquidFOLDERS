
// CJournalButton.h: Schnittstelle der Klasse CJournalButton
//

#pragma once
#include "LFCommDlg.h"


// CJournalUI
//

class CJournalUI : public CCmdUI
{
public:
	CJournalUI();

	virtual void Enable(BOOL bOn);

	BOOL m_Enabled;
};


// CJournalButton
//

class CJournalButton : public CBackstageBar
{
public:
	CJournalButton();

protected:
	virtual void OnClickButton(INT Index) const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnIdleUpdateCmdUI();
	DECLARE_MESSAGE_MAP()
};

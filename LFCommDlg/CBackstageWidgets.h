
// CBackstageWidgets: Schnittstelle der Klasse CBackstageWidgets
//

#pragma once
#include "CBackstageBar.h"


// CBackstageWidgets
//

class CBackstageBar;

class CBackstageWidgets : public CBackstageBar
{
public:
	CBackstageWidgets();

	void AddWidgetSize(LPSIZE lpSize) const;

protected:
	virtual void DrawItem(CDC& dc, CRect& rectItem, UINT Index, UINT State, BOOL Themed) const;
	virtual void OnClickButton(INT Index) const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
};

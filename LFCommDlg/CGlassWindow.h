
// CGlassWindow: Schnittstelle der Klasse CGlassWindow
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFApplication.h"


// CGlassWindow
//

class AFX_EXT_CLASS CGlassWindow : public CWnd
{
public:
	CGlassWindow();
	~CGlassWindow();

	virtual void UseGlassBackground();

	BOOL m_IsAeroWindow;
	HTHEME hTheme;

protected:
	LFApplication* p_App;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnCompositionChanged();
	DECLARE_MESSAGE_MAP()
};

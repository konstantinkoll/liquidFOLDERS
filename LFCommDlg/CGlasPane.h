
// CGlasPane: Schnittstelle der Klasse CGlasPane
//

#pragma once


// CGlasPane
//

class AFX_EXT_CLASS CGlasPane : public CWnd
{
public:
	CGlasPane();

	BOOL Create(CWnd* pParentWnd, UINT nID);

protected:
	DECLARE_MESSAGE_MAP()
};

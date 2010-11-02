
// CDriveMenu.h: Schnittstelle der Klasse CDriveMenu
//

#pragma once
#include <ezshellextensions.h>


// CDriveMenu
//

#define VERB_CREATENEWSTOREDRIVE     "newstoredrive"

class CDriveMenu : public CContextMenuExtension
{
public:
	DECLARE_DYNCREATE(CDriveMenu)
	DECLARE_OLECREATE_EX(CDriveMenu)

	CDriveMenu();

	virtual void GetExtensionTargetInfo(CExtensionTargetInfo& info);
	virtual BOOL OnInitialize(LPDATAOBJECT dataObject);
	virtual void OnGetMenuItems(CGetMenuitemsEventArgs& e);
	virtual BOOL OnExecuteMenuItem(CExecuteItemEventArgs& e);

protected:
	wchar_t Drive;
};

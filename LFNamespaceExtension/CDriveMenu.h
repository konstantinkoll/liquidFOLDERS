
#pragma once
#include <ezshellextensions.h>

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
	char Drive;
};

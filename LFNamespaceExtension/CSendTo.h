
#pragma once
#include <ezshellextensions.h>

class CSendTo : public CSendToExtension
{
public:
	DECLARE_DYNCREATE(CSendTo)
	DECLARE_OLECREATE_EX(CSendTo)

	CSendTo();

	virtual void GetExtensionTargetInfo(CSendToExtensionTargetInfo& info);
	virtual UINT OnDragDrop(CDragDropEventArgs& e);
};

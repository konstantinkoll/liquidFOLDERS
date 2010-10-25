
// LFSendTo.h: Schnittstelle der Klasse LFSendTo
//

#pragma once
#include <ezshellextensions.h>


// LFSendTo
//

class LFSendTo : public CSendToExtension
{
public:
	DECLARE_DYNCREATE(LFSendTo)
	DECLARE_OLECREATE_EX(LFSendTo)

	LFSendTo();

	virtual void GetExtensionTargetInfo(CSendToExtensionTargetInfo& info);
	virtual UINT OnDragDrop(CDragDropEventArgs& e);
};

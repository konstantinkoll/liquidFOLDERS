
// LFSendTo.cpp: Implementierung der Klasse LFSendTo
//

#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "LFSendTo.h"


IMPLEMENT_DYNCREATE(LFSendTo, CSendToExtension)
IMPLEMENT_OLECREATE_EX(LFSendTo, "LFNamespaceExtension.SendTo",
	0x3F2D914D, 0xFE57, 0x414F, 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4)


//The classfactory is nested in your class and has a name formed
//by concatenating the class name with "Factory".
BOOL LFSendTo::LFSendToFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
	{
		BOOL ret = AfxOleRegisterServerClass(m_clsid, m_lpszProgID, m_lpszProgID, m_lpszProgID, OAT_DISPATCH_OBJECT);
		
		// Register the shell extension
		CSendToExtension::RegisterExtension(RUNTIME_CLASS(LFSendTo));
		return ret;
	}
	else
	{
		// Unregister the shell extension
		CSendToExtension::UnregisterExtension(RUNTIME_CLASS(LFSendTo));
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
	}
}


// LFSendTo
//

LFSendTo::LFSendTo()
{
}

void LFSendTo::GetExtensionTargetInfo(CSendToExtensionTargetInfo& info)
{
	WCHAR Name[256];
	LFGetDefaultStoreName(Name, 256);

	info.description = Name;
	info.iconFile = theApp.m_PathApplication;
	info.iconIndex = 0;
}

UINT LFSendTo::OnDragDrop(CDragDropEventArgs& /*e*/)
{
	return theApp.ImportFiles("", GetDataObject(), FALSE);
}

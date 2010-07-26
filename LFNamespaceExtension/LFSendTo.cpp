
#include "stdafx.h"
#include "LFCore.h"
#include "LFNamespaceExtension.h"
#include "LFSendTo.h"
#include "..\\LFCore\\resource.h"


IMPLEMENT_DYNCREATE(LFSendTo, CSendToExtension)

// The GUID and ProgID of the shell extension
IMPLEMENT_OLECREATE_EX(LFSendTo, _T("LFNamespaceExtension.SendTo"),
	0x3f2d914d, 0xfe57, 0x414f, 0x9f, 0x88, 0xa3, 0x77, 0xc7, 0x84, 0x1d, 0xa4)


// This function is called when you register the shell extension dll file
// using the regsvr32.exe or similar utility

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


// Class LFSendTo
//

LFSendTo::LFSendTo()
{
}

void LFSendTo::GetExtensionTargetInfo(CSendToExtensionTargetInfo& info)
{
	char Name[256];
	LFGetDefaultStoreName(Name, 256);

	info.description = Name;
	info.iconFile = theApp.m_IconFile;
	info.iconIndex = IDI_STORE_Default-1;
}

UINT LFSendTo::OnDragDrop(CDragDropEventArgs& /*e*/)
{
	if (!LFDefaultStoreAvailable())
	{
		LFErrorBox(LFNoDefaultStore);
		return DROPEFFECT_NONE;
	}

	CStringArray* files = GetFiles();

	// TODO
	MessageBox(NULL, _T("Not implemented yet"), _T("Send to"), 0);

	return DROPEFFECT_COPY;
}

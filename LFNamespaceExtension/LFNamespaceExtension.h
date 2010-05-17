
#pragma once
#include "resource.h"
#include "liquidFOLDERS.h"
#include <objbase.h>
#include <shlguid.h>

#define CLFNamespaceExtensionVersion   1

static const GUID GUID_FMTID_PROPERTY =
	{ 0xf29f85e0, 0x4ff9, 0x1068, { 0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9 } };

class CLFNamespaceExtensionApp : public CWinApp
{
public:
	CLFNamespaceExtensionApp();
	~CLFNamespaceExtensionApp();

	virtual BOOL InitInstance();

	TCHAR m_IconFile[MAX_PATH];
	LFAttributeDescriptor* m_Attributes[LFAttributeCount];
	LFDomainDescriptor* m_Domains[LFDomainCount];
	LFItemCategoryDescriptor* m_ItemCategories[LFItemCategoryCount];
	wchar_t* m_AttrCategoryNames[LFAttrCategoryCount+1];
};

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
extern CLFNamespaceExtensionApp theApp;
extern OSVERSIONINFO osInfo;

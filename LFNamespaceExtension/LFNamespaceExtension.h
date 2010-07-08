
#pragma once
#include "resource.h"
#include "liquidFOLDERS.h"
#include <objbase.h>
#include <shlguid.h>

#define LFNamespaceExtensionVersion   1

class LFNamespaceExtensionApp : public CWinApp
{
public:
	LFNamespaceExtensionApp();
	~LFNamespaceExtensionApp();

	virtual BOOL InitInstance();

	BOOL HideFileExt();

	TCHAR m_IconFile[MAX_PATH];
	LFAttributeDescriptor* m_Attributes[LFAttributeCount];
	LFDomainDescriptor* m_Domains[LFDomainCount];
	LFItemCategoryDescriptor* m_ItemCategories[LFItemCategoryCount];
	wchar_t* m_AttrCategoryNames[LFAttrCategoryCount+1];
};

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
extern LFNamespaceExtensionApp theApp;
extern OSVERSIONINFO osInfo;

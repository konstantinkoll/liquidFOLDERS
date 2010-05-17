
#pragma once
#include "resource.h"
#include "liquidFOLDERS.h"
#include <objbase.h>
#include <shlguid.h>

#define CLFNamespaceExtensionVersion   1

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


#pragma once
#include "liquidFOLDERS.h"
#include <eznamespaceextensions.h>


class CFileItem : public CNSEItem
{
public:
	DECLARE_DYNCREATE(CFileItem)

	CString StoreID;
	LFCoreAttributes Attrs;

	CFileItem();
	CFileItem(LPCTSTR _StoreID, LFCoreAttributes* _Attrs);

	virtual ~CFileItem();

	virtual NSEItemAttributes GetAttributes(NSEItemAttributes requested);
	virtual void Serialize(CArchive& ar);
	virtual void GetDisplayName(CString& displayName);
	virtual void GetDisplayNameEx(CString& displayName, DisplayNameFlags flags);
	virtual void GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e);
	virtual void GetInfoTip(CString& infotip);
	virtual int GetXPTaskPaneColumnIndices(UINT* indices);
	int GetTileViewColumnIndices(UINT* indices);
	int GetPreviewDetailsColumnIndices(UINT* indices);
	virtual int GetContentViewColumnIndices(UINT* indices);
	BOOL GetColumnValueEx(VARIANT* value,CShellColumn& column);
	virtual BOOL IsValid();
	virtual int CompareTo(CNSEItem* otherItem, CShellColumn& column);
	virtual BOOL GetFileDescriptor(FILEDESCRIPTOR* fd);
	virtual LPSTREAM GetStream();

	// TODO
	virtual BOOL OnChangeName(CChangeNameEventArgs& e);
};

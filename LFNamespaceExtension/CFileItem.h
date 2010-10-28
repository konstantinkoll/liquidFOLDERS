
// CFileItem.h: Schnittstelle der Klasse CFileItem
//

#pragma once
#include "liquidFOLDERS.h"
#include <eznamespaceextensions.h>


// CFileItem
//

class CFileItem : public CNSEItem
{
public:
	DECLARE_DYNCREATE(CFileItem)

	// IPersist
	CFileItem();
	CFileItem(LPCTSTR _StoreID, LFCoreAttributes* _Attrs);

	// PIDL handling
	virtual void Serialize(CArchive& ar);

	// IEnumIDList
	virtual BOOL IsValid();

	// IMoniker
	virtual void GetDisplayName(CString& displayName);
	virtual void GetDisplayNameEx(CString& displayName, DisplayNameFlags flags);

	// IExtractIcon
	virtual void GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e);

	// IQueryInfo
	virtual void GetInfoTip(CString& infotip);

	// IShellFolder2
	BOOL GetColumnValueEx(VARIANT* value,CShellColumn& column);

	// IShellFolder
	virtual BOOL OnChangeName(CChangeNameEventArgs& e);

	// IShellItem
	virtual NSEItemAttributes GetAttributes(NSEItemAttributes requested);
	virtual int CompareTo(CNSEItem* otherItem, CShellColumn& column);

	// IDropSource
	virtual BOOL GetFileDescriptor(FILEDESCRIPTOR* fd);
	virtual LPSTREAM GetStream();

	// Exposed property handlers
	virtual int GetXPTaskPaneColumnIndices(UINT* indices);
	int GetTileViewColumnIndices(UINT* indices);
	int GetPreviewDetailsColumnIndices(UINT* indices);
	virtual int GetContentViewColumnIndices(UINT* indices);

	// Other
	BOOL SetShellLink(IShellLink* psl);

	CString StoreID;
	LFCoreAttributes Attrs;
};

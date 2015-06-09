
// CFileItem.h: Schnittstelle der Klasse CFileItem
//

#pragma once
#include "LF.h"
#include <eznamespaceextensions.h>


// CFileItem
//

class CFileItem : public CNSEItem
{
public:
	DECLARE_DYNCREATE(CFileItem)

	// IPersist
	CFileItem();
	CFileItem(CHAR* _StoreID, LFCoreAttributes* _Attrs);
	CFileItem(LFItemDescriptor* _Item);
	~CFileItem();

	// PIDL handling
	virtual void Serialize(CArchive& ar);

	// IMoniker
	virtual void GetDisplayName(CString& displayName);
	virtual void GetDisplayNameEx(CString& displayName, DisplayNameFlags flags);

	// IExtractIcon
	virtual void GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e);

	// IExtractImage
	virtual HBITMAP GetThumbnail(CGetThumbnailEventArgs& e);
	virtual CachingPolicy GetThumbnailCachingPolicy();
	virtual CTime GetThumbnailDateTimeStamp();

	// IQueryInfo
	virtual void GetInfoTip(CString& infotip);

	// IShellFolder2
	BOOL GetColumnValueEx(VARIANT* value,CShellColumn& column);

	// IShellFolder
	virtual BOOL OnChangeName(CChangeNameEventArgs& e);

	// IShellItem
	virtual NSEItemAttributes GetAttributes(NSEItemAttributes requested);
	virtual INT CompareTo(CNSEItem* otherItem, CShellColumn& column);

	// IDropSource
	virtual BOOL GetFileDescriptor(FILEDESCRIPTOR* fd);
	virtual LPSTREAM GetStream();

	// Exposed property handlers
	virtual INT GetXPTaskPaneColumnIndices(UINT* indices);
	INT GetTileViewColumnIndices(UINT* indices);
	INT GetPreviewDetailsColumnIndices(UINT* indices);
	virtual INT GetContentViewColumnIndices(UINT* indices);

	// Other
	BOOL SetShellLink(IShellLink* pShellLink);

	LFItemDescriptor* Item;
};

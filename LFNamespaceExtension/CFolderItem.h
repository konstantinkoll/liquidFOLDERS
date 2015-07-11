
// CFolderItem.h: Schnittstelle der Klasse CFolderItem
//

#pragma once
#include "LFCore.h"
#include <eznamespaceextensions.h>


BOOL RunPath(HWND hWnd, CString path, CString parameter=_T(""));


// CFolderItem
//

#define LEVELROOT               0
#define LEVELSTORES             1
#define LEVELATTRIBUTE          2
#define LEVELATTRVALUE          3

#define NSEIA_CFOLDERITEM       NSEIA_FileSysAncestor | NSEIA_Browsable | NSEIA_Folder | NSEIA_CanLink | NSEIA_DropTarget

#define VERB_ADDSTORE           "addstore"
#define VERB_COPY               "copy"
#define VERB_CREATESHORTCUT     "link"
#define VERB_MAKEDEFAULTSTORE   "defaultstore"
#define VERB_OPEN               "open"
#define VERB_OPENNEWWINDOW      "opennewwindow"
#define VERB_EXPLORE            "explore"
#define VERB_OPENLIQUIDFOLDERS   "openstoremanager"
#define VERB_OPENFILEDROP       "filedrop"
#define VERB_OPENWITH           "openwith"
#define VERB_OPENBROWSER        "browse"
#define VERB_RENAME             "rename"
#define VERB_DELETE             "delete"
#define VERB_PROPERTIES         "properties"
#define VERB_IMPORTFOLDER       "importfolder"
#define VERB_MIGRATIONWIZARD    "migrationwizard"
#define VERB_STOREMANAGER       "storemanager"
#define VERB_FILEDROP           "filedrop"
#define VERB_ABOUT              "about"

struct FolderSerialization
{
	UCHAR Level;
	UINT Icon;
	UINT Type;
	UINT CategoryID;
	WCHAR DisplayName[256];
	WCHAR Description[256];
	WCHAR Comments[256];
	CHAR StoreID[LFKeySize];
	CHAR FileID[LFKeySize];
	UINT Count;
	INT64 Size;
	FILETIME CreationTime;
	FILETIME FileTime;
	UCHAR Compare;
	LFVariantData Value;
};

class CFolderItem : public CNSEFolder
{
public:
	DECLARE_DYNCREATE(CFolderItem)
	DECLARE_OLECREATE_EX(CFolderItem)

	// IPersistFolder
	CFolderItem();
	CFolderItem(FolderSerialization& _Attrs);
	CFolderItem(UCHAR Level, LFItemDescriptor* i);

	virtual void GetCLSID(LPCLSID pCLSID);

	// Registration
	virtual void GetExtensionTargetInfo(CExtensionTargetInfo& info);

	// PIDL handling
	virtual void Serialize(CArchive& ar);
	virtual CNSEItem* DeserializeChild(CArchive& ar);

	// IEnumIDList
	virtual void ConvertSearchResult(CGetChildrenEventArgs& e, LFSearchResult* Result);
	virtual BOOL GetChildren(CGetChildrenEventArgs& e);

	// IMoniker
	virtual void GetDisplayName(CString& displayName);
	virtual void GetDisplayNameEx(CString& displayName, DisplayNameFlags flags);
	virtual CNSEItem* GetChildFromDisplayName(CGetChildFromDisplayNameEventArgs& e);

	// IExtractIcon
	virtual void GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e);
	virtual void GetOverlayIcon(CGetOverlayIconEventArgs& e);

	// IQueryInfo
	virtual void GetInfoTip(CString& infotip);

	// IContextMenu
	virtual void GetMenuItems(CGetMenuitemsEventArgs& e);
	virtual BOOL OnExecuteMenuItem(CExecuteMenuitemsEventArgs& e);

	// IShellBrowser
	virtual void OnMergeFrameMenu(CMergeFrameMenuEventArgs& e);
	virtual void OnExecuteFrameCommand(CExecuteFrameCommandEventArgs& e);

	// IExplorerCommandProvider
	virtual void GetToolbarCommands(CPtrList& commands);

	// ICategoryProvider
	virtual CCategorizer* GetCategorizer(CShellColumn &column);

	// IColumnProvider
	virtual BOOL GetColumn(CShellColumn& column, INT index);

	// IShellFolder2
	virtual BOOL GetColumnValueEx(VARIANT* value, CShellColumn& column);

	// IShellFolder
	virtual NSEItemAttributes GetAttributes(NSEItemAttributes requested);
	virtual INT CompareTo(CNSEItem* otherItem, CShellColumn& column);
	virtual BOOL OnOpen(CExecuteMenuitemsEventArgs& e);
	virtual BOOL OnDelete(CExecuteMenuitemsEventArgs& e);
	virtual BOOL OnChangeName(CChangeNameEventArgs& e);

	// IDropSource
	virtual void InitDataObject(CInitDataObjectEventArgs& e);
	virtual BOOL GetFileDescriptor(FILEDESCRIPTOR* fd);

	// IDropTarget
	virtual void DragEnter(CNSEDragEventArgs& e);
	virtual void DragOver(CNSEDragEventArgs& e);
	virtual void DragDrop(CNSEDragEventArgs& e);

	// Exposed property handlers
	virtual INT GetXPTaskPaneColumnIndices(UINT* indices);
	virtual INT GetTileViewColumnIndices(UINT* indices);
	virtual INT GetPreviewDetailsColumnIndices(UINT* indices);
	virtual INT GetContentViewColumnIndices(UINT* indices);

	// Other
	BOOL SetShellLink(IShellLink* pShellLink);

	FolderSerialization Attrs;

protected:
	void CreateShortcut(CNSEItem* Item);
	BOOL RunStoreCommand(CExecuteMenuitemsEventArgs& e, CString Path, CString Parameter);
	BOOL OnProperties(CExecuteMenuitemsEventArgs& e);
	BOOL OnExplorer(CExecuteMenuitemsEventArgs& e);
	BOOL OnOpenWith(CExecuteMenuitemsEventArgs& e);
};


// CFolderItem.h: Schnittstelle der Klasse CFolderItem
//

#pragma once
#include "LFCore.h"
#include <eznamespaceextensions.h>


BOOL RunPath(HWND hWnd, CString path, CString parameter=_T(""));


// CFolderItem
//

#define LevelRoot               0
#define LevelStores             1
#define LevelStoreHome          2
#define LevelAttribute          3
#define LevelAttrValue          4
#define LevelFile               5

#define NSEIA_CFOLDERITEM       NSEIA_CanLink | NSEIA_FileSysAncestor | NSEIA_Browsable | NSEIA_Folder

#define VERB_CREATELINK         "link"
#define VERB_CREATENEWSTORE     "newstore"
#define VERB_MAKEDEFAULTSTORE   "defaultstore"
#define VERB_MAKEHYBRIDSTORE    "hybridstore"
#define VERB_OPEN               "open"
#define VERB_EXPLORE            "explore"
#define VERB_OPENSTOREMANAGER   "openstoremanager"
#define VERB_OPENNEWWINDOW      "opennewwindow"
#define VERB_OPENWITH           "openwith"
#define VERB_RENAME             "rename"
#define VERB_DELETE             "delete"
#define VERB_PROPERTIES         "properties"
#define VERB_IMPORTFOLDER       "importfolder"
#define VERB_STOREMANAGER       "storemanager"
#define VERB_FILEDROP           "filedrop"
#define VERB_MIGRATE            "migrate"
#define VERB_ABOUT              "about"

struct FolderSerialization
{
	UINT Level;
	UINT Icon;
	UINT Type;
	UINT CategoryID;
	CString DisplayName;
	CString Description;
	CString Comment;
	CString StoreID;
	CString FileID;
	UINT DomainID;
	UCHAR Compare;
	LFVariantData Value;
	FILETIME CreationTime;
	FILETIME FileTime;
	UINT Count;
	INT64 Size;
	CString Format;
};

class CFolderItem : public CNSEFolder
{
public:
	DECLARE_DYNCREATE(CFolderItem)
	DECLARE_OLECREATE_EX(CFolderItem)

	// IPersistFolder
	CFolderItem();
	CFolderItem(FolderSerialization& _data);

	virtual void GetCLSID(LPCLSID pCLSID);

	// Registration
	virtual void GetExtensionTargetInfo(CExtensionTargetInfo& info);

	// PIDL handling
	virtual void Serialize(CArchive& ar);
	virtual CNSEItem* DeserializeChild(CArchive& ar);

	// IEnumIDList
	virtual BOOL GetChildren(CGetChildrenEventArgs& e);
	virtual BOOL IsValid();

	// IMoniker
	virtual void GetDisplayName(CString& displayName);
	virtual void GetDisplayNameEx(CString& displayName, DisplayNameFlags flags);

	// IExtractIcon
	virtual void GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e);

	// IQueryInfo
	virtual void GetInfoTip(CString& infotip);

	// IContextMenu
	virtual void GetMenuItems(CGetMenuitemsEventArgs& e);
	virtual BOOL OnExecuteMenuItem(CExecuteMenuitemsEventArgs& e);

	// IShellBrowser
	virtual void GetToolbarButtons(CPtrList& commands);
	virtual void OnMergeFrameMenu(CMergeFrameMenuEventArgs& e);
	virtual void OnExecuteFrameCommand(CExecuteFrameCommandEventArgs& e);

	// IExplorerCommandProvider
	virtual void GetToolbarCommands(CPtrList& commands);

	// ICategoryProvider
	virtual CCategorizer* GetCategorizer(CShellColumn &column);

	// IColumnProvider
	virtual BOOL GetColumn(CShellColumn& column, int index);

	// IShellFolder2
	virtual BOOL GetColumnValueEx(VARIANT* value, CShellColumn& column);

	// IShellFolder
	virtual NSEItemAttributes GetAttributes(NSEItemAttributes requested);
	virtual int CompareTo(CNSEItem* otherItem, CShellColumn& column);
	virtual BOOL OnOpen(CExecuteMenuitemsEventArgs& e);
	virtual BOOL OnDelete(CExecuteMenuitemsEventArgs& e);
	virtual BOOL OnChangeName(CChangeNameEventArgs& e);

	// IDropSource
	virtual void InitDataObject(CInitDataObjectEventArgs& e);
	virtual BOOL GetFileDescriptor(FILEDESCRIPTOR* fd);
	virtual void OnExternalDrop(CNSEDragEventArgs& e);

	// IDropTarget
	virtual void DragEnter(CNSEDragEventArgs& e);
	virtual void DragOver(CNSEDragEventArgs& e);
	virtual void DragDrop(CNSEDragEventArgs& e);

	// Exposed property handlers
	virtual int GetXPTaskPaneColumnIndices(UINT* indices);
	virtual int GetTileViewColumnIndices(UINT* indices);
	virtual int GetPreviewDetailsColumnIndices(UINT* indices);
	virtual int GetContentViewColumnIndices(UINT* indices);
	virtual FolderThemes GetFolderTheme();

	// Other
	BOOL SetShellLink(IShellLink* psl);

	FolderSerialization data;

protected:
	void CreateShortcut(CNSEItem* Item);
	BOOL OnImportFolder(CExecuteMenuitemsEventArgs& e);
	BOOL OnProperties(CExecuteMenuitemsEventArgs& e);
	BOOL OnExplorer(CExecuteMenuitemsEventArgs& e);
	BOOL OnOpenWith(CExecuteMenuitemsEventArgs& e);
};

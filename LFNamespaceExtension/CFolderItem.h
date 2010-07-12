
#pragma once
#include "LFCore.h"

#define LevelRoot        0
#define LevelStores      1
#define LevelStoreHome   2
#define LevelAttribute   3
#define LevelAttrValue   4
#define LevelFile        5

#define NSEIA_CFOLDERITEM   NSEIA_CanLink | NSEIA_DropTarget | NSEIA_FileSysAncestor | NSEIA_Browsable | NSEIA_Folder

#define VERB_CREATELINK         "link"
#define VERB_CREATENEWSTORE     "newstore"
#define VERB_DELETE             "delete"
#define VERB_MAKEDEFAULTSTORE   "defaultstore"
#define VERB_MAKEHYBRIDSTORE    "hybridstore"
#define VERB_OPEN               "open"
#define VERB_RENAME             "rename"
#define VERB_STOREMANAGER       "storemanager"
#define VERB_MIGRATE            "migrate"

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

	FolderSerialization data;

	CFolderItem();
	CFolderItem(FolderSerialization &_data);

	virtual void GetCLSID(LPCLSID pCLSID);
	virtual void GetExtensionTargetInfo(CExtensionTargetInfo& info);
	virtual NSEItemAttributes GetAttributes(NSEItemAttributes requested);
	virtual void Serialize(CArchive& ar);
	virtual CNSEItem* DeserializeChild(CArchive& ar);
	virtual BOOL GetChildren(CGetChildrenEventArgs& e);
	virtual void GetDisplayName(CString& displayName);
	virtual void GetDisplayNameEx(CString& displayName, DisplayNameFlags flags);
	virtual CNSEItem* GetChildFromDisplayName(CGetChildFromDisplayNameEventArgs& e);
	virtual void GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e);
	virtual void GetInfoTip(CString& infotip);
	virtual int GetXPTaskPaneColumnIndices(UINT* indices);
	virtual int GetTileViewColumnIndices(UINT* indices);
	virtual int GetPreviewDetailsColumnIndices(UINT* indices);
	virtual int GetContentViewColumnIndices(UINT* indices);
	virtual CCategorizer* GetCategorizer(CShellColumn &column);
	virtual FolderThemes GetFolderTheme();
	virtual BOOL GetColumn(CShellColumn& column, int index);
	virtual BOOL GetColumnValueEx(VARIANT* value, CShellColumn& column);
	virtual BOOL IsValid();
	virtual void GetMenuItems(CGetMenuitemsEventArgs& e);
	virtual BOOL OnExecuteMenuItem(CExecuteMenuitemsEventArgs& e);
	virtual void OnExecuteFrameCommand(CExecuteFrameCommandEventArgs& e);
	virtual int CompareTo(CNSEItem* otherItem, CShellColumn& column);
	virtual BOOL GetFileDescriptor(FILEDESCRIPTOR* fd);
	virtual void GetToolbarButtons(CPtrList& commands);
	virtual void GetToolbarCommands(CPtrList& commands);
	virtual BOOL OnChangeName(CChangeNameEventArgs& e);
	virtual BOOL OnDelete(CExecuteMenuitemsEventArgs& e);
	virtual BOOL OnOpen(CExecuteMenuitemsEventArgs& e);

	BOOL OnCreateNewStore();
	BOOL OnStoreManager(HWND hWnd=NULL);
	BOOL OnMigrate(HWND hWnd=NULL);
	void OnCreateShortcut(CNSEItem* Item, const CString& LinkFilename, const CString& Description, UINT Icon);
	void UpdateItems();

	// TODO
	virtual void InitDataObject(CInitDataObjectEventArgs& e);
	virtual void OnExternalDrop(CNSEDragEventArgs& e);
	virtual void DragOver(CNSEDragEventArgs& e);
	virtual void DragEnter(CNSEDragEventArgs& e);
	virtual void DragDrop(CNSEDragEventArgs& e);
};

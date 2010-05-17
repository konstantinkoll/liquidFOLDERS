
#pragma once
#include "LFCore.h"

#define LevelRoot        0
#define LevelStores      1
#define LevelStoreHome   2
#define LevelDomain      3
#define LevelAttribute   4
#define LevelAttrValue   5

#define NSEIA_CFOLDERITEM   NSEIA_CanLink | NSEIA_DropTarget | NSEIA_FileSysAncestor | NSEIA_Browsable | NSEIA_Folder

#define VERB_CREATENEWSTORE     "newstore"
#define VERB_MAKEDEFAULTSTORE   "defaultstore"
#define VERB_MAKEHYBRIDSTORE    "hybridstore"
#define VERB_RENAME             "rename"
#define VERB_DELETE             "delete"
#define VERB_CREATELINK         "link"
#define VERB_CREATELINKDESKTOP  "linkdesktop"

struct FolderSerialization
{
	UINT Level;
	UINT Icon;
	UINT Type;
	UINT CategoryID;
	CString DisplayName;
	CString Hint;
	CString Comment;
	CString FileID;
	CString StoreID;
	UINT DomainID;
	UINT AttributeID;
	CString AttributeValue;
	FILETIME CreationTime;
	FILETIME FileTime;
};

class CFolderItem : public CNSEFolder
{
public:
	DECLARE_DYNCREATE(CFolderItem)
	DECLARE_OLECREATE_EX(CFolderItem)

	FolderSerialization data;

	CFolderItem();
	CFolderItem(FolderSerialization _data);

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
	int GetTileViewColumnIndices(UINT* indices);
	int GetPreviewDetailsColumnIndices(UINT* indices);
	virtual CCategorizer* GetCategorizer(CShellColumn &column);
	virtual FolderThemes GetFolderTheme();
	virtual BOOL GetColumn(CShellColumn& column,int index);
	virtual BOOL GetColumnValueEx(VARIANT* value, CShellColumn& column);
	virtual BOOL IsValid();
	virtual void GetMenuItems(CGetMenuitemsEventArgs& e);
	virtual BOOL OnExecuteMenuItem(CExecuteMenuitemsEventArgs& e);
	virtual int CompareTo(CNSEItem* otherItem, CShellColumn& column);
	virtual BOOL GetFileDescriptor(FILEDESCRIPTOR* fd);
	virtual BOOL OnChangeName(CChangeNameEventArgs& e);
	virtual BOOL OnDelete(CExecuteMenuitemsEventArgs& e);
	void CreateShortcut(CNSEItem* Item, const CString& LinkFilename, const CString& Description, UINT Icon);
	void UpdateItems(BOOL add);

	// TODO
	virtual void InitDataObject(CInitDataObjectEventArgs& e);
	virtual void OnExternalDrop(CNSEDragEventArgs& e);
	virtual void DragOver(CNSEDragEventArgs& e);
	virtual void DragEnter(CNSEDragEventArgs& e);
	virtual void DragDrop(CNSEDragEventArgs& e);
	virtual BOOL OnOpen(CExecuteMenuitemsEventArgs& e);
};

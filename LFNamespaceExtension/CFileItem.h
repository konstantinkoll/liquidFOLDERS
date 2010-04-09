
#if !defined(AFX_LFNamespaceExtensionItem_H__7622C9A3_7250_4A9F_98AB_ED90FAF02D6B__INCLUDED_)
#define AFX_LFNamespaceExtensionItem_H__7622C9A3_7250_4A9F_98AB_ED90FAF02D6B__INCLUDED_

#pragma once

class CFileItem : public CNSEItem
{
public:

	DECLARE_DYNCREATE(CFileItem)

	// name of the file
	CString name;
	// Full path of the file
	CString fullPath;
	
	CFileItem();
	CFileItem(LPCTSTR parentFolder,LPCTSTR name);
	CFileItem(LPCTSTR s);

	virtual ~CFileItem();

	virtual void GetOverlayIcon(CGetOverlayIconEventArgs& e);
	virtual LPSTREAM GetStream();
	virtual BOOL GetFileDescriptor(FILEDESCRIPTOR* fd);
	virtual void GetDisplayNameEx(CString& displayName,DisplayNameFlags flags);
	virtual HBITMAP GetThumbnail(CGetThumbnailEventArgs& e);
	virtual void GetDisplayName(CString& displayName);
	virtual void Serialize(CArchive& ar);
	virtual int CompareTo(CNSEItem* otherItem, CShellColumn& column);
	virtual BOOL OnChangeName(CChangeNameEventArgs& e);
	virtual int GetXPTaskPaneColumnIndices(UINT* indices);
	virtual NSEItemAttributes GetAttributes(NSEItemAttributes requested);
	//virtual BOOL GetColumnValue(CString& value,CShellColumn& column);
	virtual void GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e);
	virtual void GetInfoTip(CString& infotip);
	virtual BOOL IsValid();

	HKEY GetKey();
	void GetTypeString(CString& typeStr);

	BOOL GetColumnValueEx(VARIANT* value,CShellColumn& column);
	int GetTileViewColumnIndices(UINT* indices);
	int GetPreviewDetailsColumnIndices(UINT* indices);

};


#endif

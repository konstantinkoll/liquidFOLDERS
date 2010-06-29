
#pragma once

class CFileItem : public CNSEItem
{
public:
	DECLARE_DYNCREATE(CFileItem)

	CString StoreID;
	LFCoreAttributes Attrs;

	CFileItem();
	CFileItem(LPCTSTR _StoreID, LFCoreAttributes* _Attrs);

	virtual ~CFileItem();

	virtual void GetDisplayName(CString& displayName);
	virtual void GetDisplayNameEx(CString& displayName, DisplayNameFlags flags);
	virtual void Serialize(CArchive& ar);

	virtual void GetOverlayIcon(CGetOverlayIconEventArgs& e);
	virtual LPSTREAM GetStream();
	virtual BOOL GetFileDescriptor(FILEDESCRIPTOR* fd);
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

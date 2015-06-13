
// Commands.h: Schnittstelle der Klassen für Toolbar-Befehle
//

#pragma once
#include "LF.h"
#include <eznamespaceextensions.h>


// CmdCreateNewStore
//

class CmdCreateNewStore : public CExplorerCommand
{
public:
	CmdCreateNewStore();

	virtual CString GetCaption(CPtrList* nseItems);
	virtual CString GetToolTip(CPtrList* nseItems);
	virtual ExplorerCommandState GetState(CPtrList* nseItems);
	virtual BOOL Invoke(CPtrList* nseItems);
	virtual CString GetIcon(CPtrList* nseItems);
};


// CmdProperties
//

class CmdProperties : public CExplorerCommand
{
public:
	CmdProperties();

	virtual CString GetCaption(CPtrList* nseItems);
	virtual CString GetToolTip(CPtrList* nseItems);
	virtual ExplorerCommandState GetState(CPtrList* nseItems);
	virtual BOOL Invoke(CPtrList* nseItems);
	virtual CString GetIcon(CPtrList* nseItems);
};


// CmdFileDrop
//

class CmdFileDrop : public CExplorerCommand
{
public:
	CmdFileDrop(CHAR* StoreID);

	virtual CString GetCaption(CPtrList* nseItems);
	virtual CString GetToolTip(CPtrList* nseItems);
	virtual ExplorerCommandState GetState(CPtrList* nseItems);
	virtual BOOL Invoke(CPtrList* nseItems);
	virtual CString GetIcon(CPtrList* nseItems);

protected:
	CHAR m_StoreID[LFKeySize];
};


// CmdImportFolder
//

class CmdImportFolder : public CExplorerCommand
{
public:
	CmdImportFolder(CHAR* StoreID);

	virtual CString GetCaption(CPtrList* nseItems);
	virtual CString GetToolTip(CPtrList* nseItems);
	virtual ExplorerCommandState GetState(CPtrList* nseItems);
	virtual BOOL Invoke(CPtrList* nseItems);
	virtual CString GetIcon(CPtrList* nseItems);

protected:
	CHAR m_StoreID[LFKeySize];
};

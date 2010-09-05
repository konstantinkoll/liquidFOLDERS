
#pragma once
#include "LFCore.h"


// CmdImportFolder
//

class CmdImportFolder : public CExplorerCommand
{
public:
	CmdImportFolder();

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


// CmdStoreManager
//

class CmdStoreManager : public CExplorerCommand
{
public:
	CmdStoreManager();

	virtual CString GetCaption(CPtrList* nseItems);
	virtual CString GetToolTip(CPtrList* nseItems);
	virtual ExplorerCommandState GetState(CPtrList* nseItems);
	virtual BOOL Invoke(CPtrList* nseItems);
	virtual CString GetIcon(CPtrList* nseItems);
};


// CmdMigrate
//


class CmdMigrate : public CExplorerCommand
{
public:
	CmdMigrate();

	virtual CString GetCaption(CPtrList* nseItems);
	virtual CString GetToolTip(CPtrList* nseItems);
	virtual ExplorerCommandState GetState(CPtrList* nseItems);
	virtual BOOL Invoke(CPtrList* nseItems);
	virtual CString GetIcon(CPtrList* nseItems);
};

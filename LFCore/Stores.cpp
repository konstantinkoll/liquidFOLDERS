#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "ShellProperties.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>
#include <io.h>
#include <iostream>
#include <malloc.h>
#include <objbase.h>
#include <shellapi.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;
extern LFMessageIDs LFMessages;

static char KeyChars[38] = { LFKeyChars };


// Verzeichnis-Funktionen
//

DWORD CreateDir(LPCSTR lpPath)
{
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	PACL pAcl = NULL;
	DWORD cbAcl = 0;
	DWORD dwNeeded = 0;
	DWORD dwError = 0;
	HANDLE hToken;
	PTOKEN_USER ptu = NULL;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		return GetLastError();

	GetTokenInformation(hToken, TokenUser, NULL, 0, &dwNeeded);
	if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	ptu = (TOKEN_USER*)malloc(dwNeeded);
	if (!GetTokenInformation(hToken, TokenUser, ptu, dwNeeded, &dwNeeded))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	cbAcl = sizeof(ACL)+((sizeof(ACCESS_ALLOWED_ACE)-sizeof(DWORD))+GetLengthSid(ptu->User.Sid));
	pAcl = (ACL*)malloc(cbAcl);

	if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	if (!AddAccessAllowedAce(pAcl, ACL_REVISION,GENERIC_ALL | STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL, ptu->User.Sid))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE);
	SetSecurityDescriptorOwner(&sd, ptu->User.Sid, FALSE);
	SetSecurityDescriptorGroup(&sd, NULL, FALSE); 
	SetSecurityDescriptorSacl(&sd, FALSE, NULL, FALSE);

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = TRUE;

	CreateDirectoryA(lpPath, &sa);
	dwError = GetLastError();

Cleanup:
	if (ptu)
		free(ptu);
	if (pAcl)
		free(pAcl);

	CloseHandle(hToken);
	return dwError;
}

void RemoveDir(LPCSTR lpPath)
{
	// Dateien l�schen
	char DirSpec[MAX_PATH];
	strcpy_s(DirSpec, MAX_PATH, lpPath);
	strcat_s(DirSpec, "*");

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA(DirSpec, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
FileFound:
		if ((FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL))==0)
		{
			char fn[MAX_PATH];
			strcpy_s(fn, MAX_PATH, lpPath);
			strcat_s(fn, MAX_PATH, FindFileData.cFileName);
			DeleteFileA(fn);
		}

		if (FindNextFileA(hFind, &FindFileData)!=0)
			goto FileFound;

		FindClose(hFind);
	}

	// Verzeichnis l�schen
	RemoveDirectoryA(lpPath);
}

unsigned int CopyDir(LPCSTR lpPathSrc, LPCSTR lpPathDst)
{
	char DirSpec[MAX_PATH];
	strcpy_s(DirSpec, MAX_PATH, lpPathSrc);
	strcat_s(DirSpec, MAX_PATH, "*");

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA(DirSpec, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
FileFound:
		if ((FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL))==0)
		{
			char fns[MAX_PATH];
			strcpy_s(fns, MAX_PATH, lpPathSrc);
			strcat_s(fns, MAX_PATH, FindFileData.cFileName);

			char fnd[MAX_PATH];
			strcpy_s(fnd, MAX_PATH, lpPathDst);
			strcat_s(fnd, MAX_PATH, FindFileData.cFileName);

			if (!CopyFileA(fns, fnd, FALSE))
			{
				FindClose(hFind);
				return LFCannotCopyIndex;
			}
		}

		if (FindNextFileA(hFind, &FindFileData)!=0)
			goto FileFound;

		FindClose(hFind);
	}

	return LFOk;
}

bool DirFreeSpace(LPCSTR lpPathSrc, unsigned int Required)
{
	ULARGE_INTEGER FreeBytesAvailable;
	if (!GetDiskFreeSpaceExA(lpPathSrc, &FreeBytesAvailable, NULL, NULL))
		return false;

	return FreeBytesAvailable.QuadPart>=Required;
}

bool DirWriteable(LPCSTR lpPath)
{
	char Filename[MAX_PATH];
	strcpy_s(Filename, MAX_PATH, lpPath);
	strcat_s(Filename, MAX_PATH, "LF_TEST.BIN");

	HANDLE h = CreateFileA(Filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h==INVALID_HANDLE_VALUE)
		return false;

	CloseHandle(h);
	return DeleteFileA(Filename)==TRUE;
}

unsigned int ValidateStoreDirectories(LFStoreDescriptor* s)
{
	// Store phys. anlegen
	if (s->DatPath[0]!='\0')
	{
		DWORD res = CreateDir(s->DatPath);
			if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
				return LFIllegalPhysicalPath;
	}

	if (s->DatPath[0]!='\0')
	{
		DWORD res = CreateDir(s->IdxPathMain);
			if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
				return LFIllegalPhysicalPath;

		// Store auf externen Medien verstecken
		if ((s->StoreMode!=LFStoreModeInternal) && (res==ERROR_SUCCESS))
			SetFileAttributesA(s->DatPath, FILE_ATTRIBUTE_HIDDEN);
	}

	// Hilfsindex anlegen
	if (s->StoreMode==LFStoreModeHybrid)
	{
		char tmpStr[MAX_PATH];
		GetAutoPath(s, tmpStr);
		DWORD res = CreateDir(tmpStr);
		if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;
		res = CreateDir(s->IdxPathAux);
		if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;
	}

	return LFOk;
}

void CreateStoreIndex(char* _Path, char* _StoreID, char* _DatPath, unsigned int &res)
{
	if (_Path[0]=='\0')
		return;

	CIndex idx(_Path, _StoreID, _DatPath);
	if (!idx.Create())
		res = LFIndexNotCreated;
}

void GetFileLocation(char* _Path, char* _FileID, char* _FileFormat, char* dst, size_t cCount)
{
	char buf[3] = " \\";
	buf[0] = _FileID[0];

	strcpy_s(dst, cCount, _Path);
	strcat_s(dst, cCount, buf);
	strcat_s(dst, cCount, &_FileID[1]);

	if (_FileFormat[0]!='\0')
	{
		strcat_s(dst, cCount, ".");
		strcat_s(dst, cCount, _FileFormat);
	}
}

bool FileExists(char* path)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA(path, &FindFileData);

	bool res = (hFind!=INVALID_HANDLE_VALUE);
	if (res)
		FindClose(hFind);

	return res;
}

unsigned int PrepareImport(LFStoreDescriptor* slot, LFItemDescriptor* i, char* Dst, size_t cCount)
{
	SetAttribute(i, LFAttrStoreID, slot->StoreID);

	SYSTEMTIME st;
	GetSystemTime(&st);
	srand(st.wMilliseconds*rand());

	char Path[MAX_PATH];

	do
	{
		for (unsigned int a=0; a<LFKeyLength; a++)
		{
			int r = rand()%sizeof(KeyChars);
			i->CoreAttributes.FileID[a] = KeyChars[r];
		}

		i->CoreAttributes.FileID[LFKeyLength] = 0;
		GetFileLocation(slot->DatPath, i->CoreAttributes.FileID, "*", Path, MAX_PATH);
	}
	while (FileExists(Path));

	if (Dst)
	{
		GetFileLocation(slot->DatPath, i->CoreAttributes.FileID, i->CoreAttributes.FileFormat, Path, MAX_PATH);
		strcpy_s(Dst, cCount, Path);
	}

	char* LastBackslash = strrchr(Path, '\\');
	if (LastBackslash)
		*LastBackslash = '\0';

	DWORD res = CreateDir(Path);
	return ((res==ERROR_SUCCESS) || (res==ERROR_ALREADY_EXISTS)) ? LFOk : LFIllegalPhysicalPath;
}

void SendLFNotifyMessage(unsigned int Msg, unsigned int Flags, HWND hWndSource)
{
	SendNotifyMessage(HWND_BROADCAST, Msg, (WPARAM)Flags, (LPARAM)hWndSource);
}

void SendShellNotifyMessage(unsigned int Msg, char* StoreID)
{
	ULONG chEaten;
	ULONG dwAttributes;
	LPITEMIDLIST pidl = NULL;
	IShellFolder* pDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&pDesktop)))
		return;

	wchar_t Key[LFKeySize+1];
	if (StoreID)
	{
		wcscpy_s(Key, LFKeySize+1, L"\\");
		MultiByteToWideChar(CP_ACP, 0, StoreID, (int)(strlen(StoreID)+1), &Key[1], LFKeySize);
	}
	else
	{
		Key[0] = L'\0';
	}

	wchar_t Path[MAX_PATH];
	wcscpy_s(Path, MAX_PATH, L"::{3F2D914F-FE57-414F-9F88-A377C7841DA4}");
	wcscat_s(Path, MAX_PATH, Key);
	if (SUCCEEDED(pDesktop->ParseDisplayName(NULL, NULL, Path, &chEaten, &pidl, &dwAttributes)))
		SHChangeNotify(Msg, SHCNF_FLUSH, pidl, (Msg==SHCNE_RENAMEFOLDER) ? pidl : NULL);

	wcscpy_s(Path, MAX_PATH, L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}");
	wcscat_s(Path, MAX_PATH, Key);
	if (SUCCEEDED(pDesktop->ParseDisplayName(NULL, NULL, Path, &chEaten, &pidl, &dwAttributes)))
		SHChangeNotify(Msg, SHCNF_FLUSH, pidl, (Msg==SHCNE_RENAMEFOLDER) ? pidl : NULL);

	pDesktop->Release();
}


// Stores
//

LFCore_API unsigned int LFGetFileLocation(char* StoreID, LFCoreAttributes* ca, char* dst, size_t cCount)
{
	if (StoreID=='\0')
		return LFIllegalKey;

	if (ca->Flags & LFFlagLink)
	{
		*dst = '\0';
		return LFOk;
	}

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(StoreID);

	if (slot)
		if (IsStoreMounted(slot))
		{
			GetFileLocation(slot->DatPath, ca->FileID, ca->FileFormat, dst, cCount);

			if (FileExists(dst))
			{
				ca->Flags &= ~LFFlagMissing;
				res = LFOk;
			}
			else
			{
				ca->Flags |= LFFlagMissing;
				res = LFNoFileBody;
			}
		}
		else
		{
			res = LFStoreNotMounted;
		}

	ReleaseMutex(Mutex_Stores);
	return res;
}

LFCore_API unsigned int LFGetFileLocation(LFItemDescriptor* i, char* dst, size_t cCount)
{
	if ((i->Type & LFTypeMask)!=LFTypeFile)
		return LFIllegalKey;

	unsigned int flags = i->CoreAttributes.Flags;
	unsigned int res = LFGetFileLocation(i->StoreID, &i->CoreAttributes, dst, cCount);

	if (flags!=i->CoreAttributes.Flags)
	{
		// Update index
		CIndex* idx1;
		CIndex* idx2;
		HANDLE StoreLock = NULL;
		if (OpenStore(i->StoreID, true, idx1, idx2, NULL, &StoreLock)==LFOk)
		{
			if (idx1)
			{
				idx1->Update(i, false);
				delete idx1;
			}
			if (idx2)
			{
				idx2->Update(i, false);
				delete idx2;
			}
			ReleaseMutexForStore(StoreLock);
		}
	}

	return res;
}

LFCore_API unsigned int LFGetStoreSettings(char* key, LFStoreDescriptor* s)
{
	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(key);
	if (slot)
		*s = *slot;

	ReleaseMutex(Mutex_Stores);
	return (slot ? LFOk : LFIllegalKey);
}

LFCore_API unsigned int LFGetStoreSettings(GUID guid, LFStoreDescriptor* s)
{
	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(guid);
	if (slot)
		*s = *slot;

	ReleaseMutex(Mutex_Stores);
	return (slot ? LFOk : LFIllegalKey);
}

LFCore_API unsigned int LFCreateStore(LFStoreDescriptor* s, bool MakeDefault, HWND hWndSource)
{
	// GUID generieren
	CoCreateGuid(&s->guid);

	// Pfad erg�nzen
	AppendGUID(s, s->DatPath);

	unsigned int res = ValidateStoreSettings(s);
	if (res!=LFOk)
		return res;

	// Ggf. Name setzen
	if (!s->StoreName[0])
		LoadStringW(LFCoreModuleHandle, IDS_StoreDefaultName, s->StoreName, 256);

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	// CreateTime und MaintenanceTime setzen
	GetSystemTimeAsFileTime(&s->CreationTime);
	s->MaintenanceTime = s->CreationTime;
	s->NeedsCheck = false;

	// Key generieren
	CreateStoreKey(s->StoreID);

	// Index-Version
	s->IndexVersion = CurIdxVersion;

	// Store speichern
	res = UpdateStore(s, MakeDefault);

	if (res==LFOk)
	{
		HANDLE StoreLock;
		if (GetMutexForStore(s, &StoreLock))
		{
			ReleaseMutex(Mutex_Stores);

			res = ValidateStoreDirectories(s);
			if (res==LFOk)
			{
				CreateStoreIndex(s->IdxPathMain, s->StoreID, s->DatPath, res);
				CreateStoreIndex(s->IdxPathAux, s->StoreID, s->DatPath, res);
			}

			ReleaseMutex(StoreLock);
			SendLFNotifyMessage(LFMessages.StoresChanged, s->StoreMode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores , hWndSource);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
		}
		else
		{
			res = LFMutexError;
		}
	}
	else
	{
		ReleaseMutex(Mutex_Stores);
	}

	return res;
}

LFCore_API unsigned int LFMakeDefaultStore(char* key, HWND hWndSource, bool InternalCall)
{
	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!InternalCall)
		if (!GetMutex(Mutex_Stores))
			return LFMutexError;

	unsigned int res = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(key);

	if (slot)
		if (slot->StoreMode!=LFStoreModeInternal)
		{
			res = LFIllegalStoreDescriptor;
		}
		else
		{
			res = LFRegistryError;

			HKEY k;
			if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &k)==ERROR_SUCCESS)
			{
				if (RegSetValueExA(k, "DefaultStore", 0, REG_SZ, (BYTE*)key, (DWORD)strlen(key))==ERROR_SUCCESS)
				{
					strcpy_s(DefaultStore, LFKeySize, key);
					res = LFOk;
				}
				RegCloseKey(k);
			}
		}

	if (!InternalCall)
	{
		ReleaseMutex(Mutex_Stores);

		if (res==LFOk)
		{
			SendLFNotifyMessage(LFMessages.DefaultStoreChanged, LFMSGF_IntStores, hWndSource);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
			SendShellNotifyMessage(SHCNE_UPDATEITEM);
		}
	}

	return res;
}

LFCore_API unsigned int LFMakeHybridStore(char* key, HWND hWndSource)
{
	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(key, &StoreLock);
	if (slot)
	{
		if ((slot->StoreMode!=LFStoreModeExternal) || (!IsStoreMounted(slot)))
		{
			ReleaseMutexForStore(StoreLock);
			ReleaseMutex(Mutex_Stores);
			return LFIllegalStoreDescriptor;
		}

		slot->StoreMode = LFStoreModeHybrid;
		res = ValidateStoreSettings(slot);
		if (res!=LFOk)
		{
			ReleaseMutexForStore(StoreLock);
			ReleaseMutex(Mutex_Stores);
			return res;
		}

		#define CheckAbort if (res!=LFOk) { ReleaseMutexForStore(StoreLock); return res; }

		res = UpdateStore(slot);
		ReleaseMutex(Mutex_Stores);
		CheckAbort

		res = ValidateStoreDirectories(slot);
		CheckAbort

		res = CopyDir(slot->IdxPathMain, slot->IdxPathAux);
		CheckAbort
	}

	ReleaseMutexForStore(StoreLock);

	if (res==LFOk)
	{
		SendLFNotifyMessage(LFMessages.StoreAttributesChanged, LFMSGF_ExtHybStores, hWndSource);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
		SendShellNotifyMessage(SHCNE_UPDATEITEM);
	}

	return res;
}

LFCore_API unsigned int LFSetStoreAttributes(char* key, wchar_t* name, wchar_t* comment, HWND hWndSource, bool InternalCall)
{
	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;
	if ((!name) && (!comment))
		return LFOk;
	if (name)
		if (wcscmp(name, L"")==0)
			return LFIllegalValue;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(key);
	if (slot)
	{
		if (name)
			wcscpy_s(slot->StoreName, 256, name);
		if (comment)
			wcscpy_s(slot->Comment, 256, comment);
		res = UpdateStore(slot);
	}

	unsigned int Mode = slot->StoreMode;
	ReleaseMutex(Mutex_Stores);

	if (res==LFOk)
	{
		if (name)
			SendShellNotifyMessage(SHCNE_RENAMEFOLDER, key);
		if (comment)
			SendShellNotifyMessage(SHCNE_UPDATEITEM);

		if (!InternalCall)
		{
			SendLFNotifyMessage(LFMessages.StoreAttributesChanged, Mode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores, hWndSource);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
		}
	}

	return res;
}

LFCore_API unsigned int LFDeleteStore(char* key, HWND hWndSource)
{
	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(key, &StoreLock);
	if ((slot) && (StoreLock))
	{
		if (slot->DatPath[0]!='\0')
		{
			char Path[MAX_PATH];
			strcpy_s(Path, MAX_PATH, slot->DatPath);
			strcat_s(Path, MAX_PATH, "*");

			if (FileExists(Path))
				if (!DirWriteable(slot->DatPath))
				{
					ReleaseMutex(Mutex_Stores);
					ReleaseMutexForStore(StoreLock);
					return LFDriveWriteProtected;
				}
		}

		LFStoreDescriptor victim = *slot;
		res = DeleteStore(slot);
		ReleaseMutex(Mutex_Stores);
		ReleaseMutexForStore(StoreLock);

		if (res==LFOk)
		{
			SendLFNotifyMessage(LFMessages.StoresChanged, victim.StoreMode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores, hWndSource);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);

			if (victim.IdxPathAux[0]!='\0')
			{
				char path[MAX_PATH];
				GetAutoPath(&victim, path);

				RemoveDir(victim.IdxPathAux);
				RemoveDir(path);
			}

			if (victim.IdxPathMain[0]!='\0')
				RemoveDir(victim.IdxPathMain);

			if (victim.DatPath[0]!='\0')
			{
				for (unsigned a=0; a<sizeof(KeyChars); a++)
				{
					char subpath[3];
					subpath[0] = KeyChars[a];
					subpath[1] = '\\';
					subpath[2] = '\0';

					char path[MAX_PATH];
					strcpy_s(path, MAX_PATH, victim.DatPath);
					strcat_s(path, MAX_PATH, subpath);

					RemoveDir(path);
				}

				RemoveDir(victim.DatPath);
			}
		}
	}
	else
	{
		if (slot)
			res = LFMutexError;

		ReleaseMutex(Mutex_Stores);
	}

	return res;
}

LFCore_API bool LFAskDeleteStore(LFItemDescriptor* s, HWND hWnd)
{
	wchar_t caption[256];
	wchar_t tmp[256];
	LoadString(LFCoreModuleHandle, IDS_DeleteStoreCaption, tmp, 256);
	swprintf_s(caption, 256 , tmp, s->CoreAttributes.FileName);

	wchar_t msg[256];
	LoadString(LFCoreModuleHandle, IDS_DeleteStoreMessage, msg, 256);

	return MessageBox(hWnd, msg, caption, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING)==IDYES;
}

LFCore_API bool LFAskDeleteStore(LFStoreDescriptor* s, HWND hWnd)
{
	wchar_t caption[256];
	wchar_t tmp[256];
	LoadString(LFCoreModuleHandle, IDS_DeleteStoreCaption, tmp, 256);
	swprintf_s(caption, 256 , tmp, s->StoreName);

	wchar_t msg[256];
	LoadString(LFCoreModuleHandle, IDS_DeleteStoreMessage, msg, 256);

	return MessageBox(hWnd, msg, caption, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING)==IDYES;
}

unsigned int RunMaintenance(LFStoreDescriptor* s, bool scheduled)
{
	// Verzeichnisse pr�fen
	unsigned int res = ValidateStoreDirectories(s);
	if (res!=LFOk)
		return res;

	// Sind die Verzeichnisse beschreibbar?
	if (s->IdxPathMain[0]!='\0')
		if (!DirWriteable(s->IdxPathMain))
			return LFDriveWriteProtected;
	if (s->IdxPathAux[0]!='\0')
		if (!DirWriteable(s->IdxPathAux))
			return LFDriveWriteProtected;

	// Index pr�fen
	CIndex* idx = new CIndex((s->StoreMode!=LFStoreModeHybrid) ? s->IdxPathMain : s->IdxPathAux, s->StoreID, s->DatPath);
	switch (idx->Check(scheduled))
	{
	case IndexNoAccess:
		delete idx;
		return LFIndexAccessError;
	case IndexCannotCreate:
		delete idx;
		return LFIndexCreateError;
	case IndexNotEnoughFreeDiscSpace:
		delete idx;
		return LFNotEnoughFreeDiscSpace;
	case IndexCompleteReindexRequired:
		// TODO
	case IndexError:
		delete idx;
		return LFIndexRepairError;
	case IndexFullyRepaired:
		s->IndexVersion = CurIdxVersion;

		res = UpdateStore(s);
		if (res!=LFOk)
		{
			delete idx;
			return res;
		}
	}

	delete idx;

	// Index duplizieren
	if ((s->StoreMode==LFStoreModeHybrid) && IsStoreMounted(s))
	{
		res = CopyDir(s->IdxPathAux, s->IdxPathMain);
		if (res!=LFOk)
			return res;
	}

	GetSystemTimeAsFileTime(&s->MaintenanceTime);
	s->NeedsCheck = false;

	return LFOk;
}

LFCore_API unsigned int LFStoreMaintenance(char* key)
{
	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(key, &StoreLock);
	ReleaseMutex(Mutex_Stores);

	if (!slot)
		return LFIllegalKey;
	if (!StoreLock)
		return LFMutexError;

	unsigned int res = RunMaintenance(slot, true);
	ReleaseMutexForStore(StoreLock);

	return res;
}

LFCore_API LFMaintenanceList* LFStoreMaintenance()
{
	LFMaintenanceList* ml = LFAllocMaintenanceList();

	if (!GetMutex(Mutex_Stores))
	{
		ml->m_LastError = LFMutexError;
		return ml;
	}

	char* keys;
	unsigned int count = FindStores(&keys);
	ReleaseMutex(Mutex_Stores);

	if (count)
	{
		char* ptr = keys;
		for (unsigned int a=0; a<count; a++)
		{
			if (!GetMutex(Mutex_Stores))
			{
				free(keys);
				ml->m_LastError = LFMutexError;
				return ml;
			}

			HANDLE StoreLock = NULL;
			LFStoreDescriptor* slot = FindStore(ptr, &StoreLock);
			ReleaseMutex(Mutex_Stores);

			if ((!slot) || (!StoreLock))
				continue;

			ml->AddStore(LFStoreMaintenance(ptr), slot->StoreName, ptr, slot->StoreMode==LFStoreModeInternal ? IDI_STORE_Internal : slot->StoreMode==LFStoreModeRemote ? IDI_STORE_Server : IDI_STORE_Bag);
			ReleaseMutexForStore(StoreLock);

			ptr += LFKeySize;
		}

		free(keys);
	}

	return ml;
}

unsigned int OpenStore(LFStoreDescriptor* s, bool WriteAccess, CIndex* &Index1, CIndex* &Index2)
{
	Index1 = Index2 = NULL;

	if (WriteAccess && !IsStoreMounted(s))
		return LFStoreNotMounted;

	// Einfache Wartungsarbeiten
	if (s->NeedsCheck)
	{
		unsigned int res = RunMaintenance(s, false);
		if ((res!=LFOk) && (res!=LFDriveWriteProtected))
			return res;
	}

	// Index �ffnen
	if (WriteAccess)
	{
		Index1 = new CIndex(s->IdxPathMain, s->StoreID, s->DatPath);
		if (s->StoreMode==LFStoreModeHybrid)
			Index2 = new CIndex(s->IdxPathAux, s->StoreID, s->DatPath);
	}
	else
	{
		Index1 = new CIndex(s->StoreMode==LFStoreModeHybrid ? s->IdxPathAux : s->IdxPathMain, s->StoreID, s->DatPath);
	}

	return LFOk;
}

unsigned int OpenStore(char* key, bool WriteAccess, CIndex* &Index1, CIndex* &Index2, LFStoreDescriptor** s, HANDLE* lock)
{
	Index1 = Index2 = NULL;

	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(key, lock);
	ReleaseMutex(Mutex_Stores);

	if (!slot)
		return LFIllegalKey;
	if (!*lock)
		return LFMutexError;

	if (s)
		*s = slot;

	unsigned int res = OpenStore(slot, WriteAccess, Index1, Index2);
	if (res!=LFOk)
		ReleaseMutexForStore(*lock);

	return res;
}

LFCore_API unsigned int LFImportFiles(char* key, LFFileImportList* il, LFItemDescriptor* it, bool recursive, bool move)
{
	assert(il);

	// Store finden
	char store[LFKeySize] = "";
	if (key)
		strcpy_s(store, LFKeySize, key);

	if (store[0]=='\0')
		if (GetMutex(Mutex_Stores))
		{
			strcpy_s(store, LFKeySize, DefaultStore);
			ReleaseMutex(Mutex_Stores);
		}

	if (store[0]=='\0')
	{
		il->m_LastError = LFNoDefaultStore;
		return LFNoDefaultStore;
	}

	// Importliste vorbereiten
	il->Resolve(recursive);

	// Import
	CIndex* idx1;
	CIndex* idx2;
	LFStoreDescriptor* slot;
	HANDLE StoreLock = NULL;
	unsigned int res = OpenStore(&store[0], true, idx1, idx2, &slot, &StoreLock);
	if (res==LFOk)
	{
		for (unsigned int a=0; a<il->m_ItemCount; a++)
			if (il->m_Items[a])
			{
				LFItemDescriptor* i = LFAllocItemDescriptor(it);
				i->CoreAttributes.Flags = LFFlagNew;
				SetNameExtAddFromFile(i, il->m_Items[a]);
				SetAttributesFromFile(i, il->m_Items[a]);

				char CopyToA[MAX_PATH];
				res = PrepareImport(slot, i, CopyToA, MAX_PATH);
				if (res!=LFOk)
				{
					LFFreeItemDescriptor(i);
					break;
				}

				wchar_t CopyToW[MAX_PATH];
				size_t sz = strlen(CopyToA)+1;
				MultiByteToWideChar(CP_ACP, 0, CopyToA, (int)sz, &CopyToW[0], (int)sz);

				BOOL shres = move ? MoveFile(il->m_Items[a], CopyToW) : CopyFile(il->m_Items[a], CopyToW, FALSE);
				if (!shres)
				{
					LFFreeItemDescriptor(i);
					res = LFIllegalPhysicalPath;
					break;
				}

				il->m_FileCount++;
				il->m_FileSize += i->CoreAttributes.FileSize;

				if (idx1)
					idx1->AddItem(i);
				if (idx2)
					idx2->AddItem(i);
				LFFreeItemDescriptor(i);
			}

		if (idx1)
			delete idx1;
		if (idx2)
			delete idx2;
		ReleaseMutexForStore(StoreLock);
	}

	il->m_LastError = res;
	return res;
}

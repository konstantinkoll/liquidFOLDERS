#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "Stores.h"
#include "StoreCache.h"
#include <io.h>
#include <malloc.h>
#include <shellapi.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>


// Der Inhalt dieses Segments wird über alle Instanzen von LFCore geteilt.
// Der Zugriff muss daher über Mutex-Objekte serialisiert/synchronisiert werden.
// Alle Variablen im Segment müssen initalisiert werden !

#pragma data_seg("common_storecache")

bool Initialized = false;
char DefaultStore[LFKeySize] = { 0 };

unsigned int StoreCount = 0;
LFStoreDescriptor StoreCache[MaxStores] = { 0 };

#pragma data_seg()
#pragma comment(linker, "/SECTION:common_storecache,RWS")


extern HANDLE Mutex_Stores;
extern HMODULE LFCoreModuleHandle;
extern LFMessageIDs LFMessages;
extern unsigned int DriveTypes[26];

#define AppPath "\\liquidFOLDERS\\"

bool IsStoreMounted(LFStoreDescriptor* s)
{
	if (!s)
		return false;

	return (strcmp(s->DatPath, "")!=0);
}

void AppendGUID(LFStoreDescriptor* s, char* p)
{
	if (s)
	{
		OLECHAR szGUID[MAX_PATH];
		int ccount = StringFromGUID2(s->guid, szGUID, MAX_PATH);

		if (ccount)
		{
			char szcGUID[MAX_PATH];
			wcstombs_s(NULL, szcGUID, MAX_PATH, szGUID, ccount);
			strcat_s(p, MAX_PATH, szcGUID);
			strcat_s(p, MAX_PATH, "\\");
		}
	}
}

void GetAutoPath(LFStoreDescriptor* s, char* p)
{
	SHGetSpecialFolderPathA(NULL, p, CSIDL_APPDATA, TRUE);
	strcat_s(p, MAX_PATH, AppPath);
	AppendGUID(s, p);
}

bool FolderExists(char* path)
{
	if (_access(path, 0)==0)
	{
		struct stat status;
		stat(path, &status);
		return ((status.st_mode & S_IFDIR)!=0);
	}

	return false;
}

unsigned int GetKeyFileFromStoreDescriptor(LFStoreDescriptor* s, char* f)
{
	if (!s)
		return LFIllegalStoreDescriptor;
	if ((s->StoreMode!=LFStoreModeHybrid) && (s->StoreMode!=LFStoreModeExternal))
		return LFIllegalStoreDescriptor;

	if (!IsStoreMounted(s))
		return LFStoreNotMounted;

	OLECHAR szGUID[MAX_PATH];
	int ccount = StringFromGUID2(s->guid, szGUID, MAX_PATH);
	if (ccount==0)
		return LFIllegalStoreDescriptor;

	char szcGUID[MAX_PATH];
	wcstombs_s(NULL, szcGUID, MAX_PATH, szGUID, ccount);

	strncpy_s(f, MAX_PATH, s->DatPath, 3);		// .store-Datei immer im Hauptverzeichnis
	strcat_s(f, MAX_PATH, szcGUID);
	strcat_s(f, MAX_PATH, ".store");

	return LFOk;
}

bool LoadStoreSettingsFromRegistry(char* key, LFStoreDescriptor* s)
{
	if (!key)
		return false;
	if (key[0]=='\0')
		return false;

	bool res = false;
	ZeroMemory(s, sizeof(LFStoreDescriptor));
	strcpy_s(s->StoreID, LFKeySize, key);

	char regkey[256];
	strcpy_s(regkey, 256, LFStoresHive);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, s->StoreID);

	HKEY k;
	if (RegOpenKeyA(HKEY_CURRENT_USER, regkey, &k)==ERROR_SUCCESS)
	{
		res = true;

		DWORD sz = sizeof(s->StoreName);
		if (RegQueryValueExW(k, L"Name", 0, NULL, (BYTE*)&s->StoreName, &sz)!=ERROR_SUCCESS)
			res = false;

		sz = sizeof(s->Comment);
		RegQueryValueExW(k, L"Comment", 0, NULL, (BYTE*)&s->Comment, &sz);

		sz = sizeof(s->StoreMode);
		if (RegQueryValueExA(k, "Mode", 0, NULL, (BYTE*)&s->StoreMode, &sz)!=ERROR_SUCCESS)
			res = false;

		if (s->StoreMode==LFStoreModeHybrid)
		{
			sz = sizeof(s->LastSeen);
			RegQueryValueExW(k, L"LastSeen", 0, NULL, (BYTE*)&s->LastSeen, &sz);
		}

		sz = sizeof(s->guid);
		if (RegQueryValueExA(k, "GUID", 0, NULL, (BYTE*)&s->guid, &sz)!=ERROR_SUCCESS)
			res = false;

		sz = sizeof(s->CreationTime);
		RegQueryValueExA(k, "CreationTime", 0, NULL, (BYTE*)&s->CreationTime, &sz);

		sz = sizeof(s->FileTime);
		RegQueryValueExA(k, "FileTime", 0, NULL, (BYTE*)&s->FileTime, &sz);

		sz = sizeof(s->MaintenanceTime);
		RegQueryValueExA(k, "MaintenanceTime", 0, NULL, (BYTE*)&s->MaintenanceTime, &sz);

		sz = sizeof(s->IndexVersion);
		if (RegQueryValueExA(k, "IndexVersion", 0, NULL, (BYTE*)&s->IndexVersion, &sz)!=ERROR_SUCCESS)
			res = false;

		sz = sizeof(s->AutoLocation);
		if (RegQueryValueExA(k, "AutoLocation", 0, NULL, (BYTE*)&s->AutoLocation, &sz)!=ERROR_SUCCESS)
			res = false;

		if (s->StoreMode==LFStoreModeInternal)
		{
			sz = sizeof(s->DatPath);
			if (RegQueryValueExA(k, "Path", 0, NULL, (BYTE*)s->DatPath, &sz)!=ERROR_SUCCESS)
				if (!s->AutoLocation)
					res = false;
		}

		RegCloseKey(k);
	}

	return res;
}

bool LoadStoreSettingsFromFile(char* filename, LFStoreDescriptor* s)
{
	if (!filename)
		return false;
	if (filename[0]=='\0')
		return false;

	HANDLE hFile = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return false;

	DWORD wmRead;
	bool res = (ReadFile(hFile, s, sizeof(LFStoreDescriptor), &wmRead, NULL)==TRUE);
	res &= (wmRead==sizeof(LFStoreDescriptor));
	CloseHandle(hFile);

	return res;
}

unsigned int SaveStoreSettingsToRegistry(LFStoreDescriptor* s)
{
	if (!s)
		return LFIllegalStoreDescriptor;
	if ((s->StoreMode!=LFStoreModeInternal) && (s->StoreMode!=LFStoreModeHybrid))
		return LFIllegalStoreDescriptor;

	// Registry-Zugriff
	unsigned int res = LFRegistryError;

	char regkey[256];
	strcpy_s(regkey, 256, LFStoresHive);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, s->StoreID);

	HKEY k;
	if (RegCreateKeyA(HKEY_CURRENT_USER, regkey, &k)==ERROR_SUCCESS)
	{
		res = LFOk;
		if (RegSetValueExW(k, L"Name", 0, REG_SZ, (BYTE*)s->StoreName, (DWORD)wcslen(s->StoreName)*sizeof(wchar_t))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if (RegSetValueExW(k, L"Comment", 0, REG_SZ, (BYTE*)s->Comment, (DWORD)wcslen(s->Comment)*sizeof(wchar_t))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if (RegSetValueExA(k, "Mode", 0, REG_DWORD, (BYTE*)&s->StoreMode, sizeof(unsigned int))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if (s->StoreMode==LFStoreModeHybrid)
			if (RegSetValueExW(k, L"LastSeen", 0, REG_SZ, (BYTE*)s->LastSeen, (DWORD)wcslen(s->LastSeen)*sizeof(wchar_t))!=ERROR_SUCCESS)
				res = LFRegistryError;
		if (RegSetValueExA(k, "GUID", 0, REG_BINARY, (BYTE*)&s->guid, sizeof(GUID))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if (RegSetValueExA(k, "CreationTime", 0, REG_BINARY, (BYTE*)&s->CreationTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if (RegSetValueExA(k, "FileTime", 0, REG_BINARY, (BYTE*)&s->FileTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if (RegSetValueExA(k, "MaintenanceTime", 0, REG_BINARY, (BYTE*)&s->MaintenanceTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if (RegSetValueExA(k, "AutoLocation", 0, REG_DWORD, (BYTE*)&s->AutoLocation, sizeof(unsigned int))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if (RegSetValueExA(k, "IndexVersion", 0, REG_DWORD, (BYTE*)&s->IndexVersion, sizeof(unsigned int))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if ((s->StoreMode==LFStoreModeInternal) && (!s->AutoLocation))
			if (RegSetValueExA(k, "Path", 0, REG_SZ, (BYTE*)s->DatPath, (DWORD)strlen(s->DatPath))!=ERROR_SUCCESS)
				res = LFRegistryError;
		RegCloseKey(k);
	}

	return res;
}

unsigned int SaveStoreSettingsToFile(LFStoreDescriptor* s)
{
	if (!s)
		return LFIllegalStoreDescriptor;
	if ((s->StoreMode!=LFStoreModeHybrid) && (s->StoreMode!=LFStoreModeExternal))
		return LFIllegalStoreDescriptor;

	char filename[MAX_PATH];
	unsigned int res = GetKeyFileFromStoreDescriptor(s, filename);
	if (res!=LFOk)
		return res;

	HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_WRITE_THROUGH, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return LFDriveNotReady;

	DWORD wmWritten;
	res = WriteFile(hFile, s, sizeof(LFStoreDescriptor), &wmWritten, NULL) ? LFOk : LFDriveNotReady;
	if (wmWritten!=sizeof(LFStoreDescriptor))
		res = LFDriveNotReady;
	CloseHandle(hFile);

	return res;
}

unsigned int DeleteStoreSettingsFromRegistry(LFStoreDescriptor* s)
{
	if (!s)
		return LFIllegalStoreDescriptor;
	if ((s->StoreMode!=LFStoreModeInternal) && (s->StoreMode!=LFStoreModeHybrid))
		return LFIllegalStoreDescriptor;

	char regkey[256];
	strcpy_s(regkey, 256, LFStoresHive);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, s->StoreID);

	LRESULT lres = RegDeleteKeyA(HKEY_CURRENT_USER, regkey);
	switch (lres)
	{
	case ERROR_FILE_NOT_FOUND:
		return LFIllegalKey;
		break;
	case ERROR_SUCCESS:
		return LFOk;
		break;
	default:
		return LFRegistryError;
	}
}

unsigned int DeleteStoreSettingsFromFile(LFStoreDescriptor* s)
{
	if (!s)
		return LFIllegalStoreDescriptor;
	if ((s->StoreMode!=LFStoreModeHybrid) && (s->StoreMode!=LFStoreModeExternal))
		return LFIllegalStoreDescriptor;

	char filename[MAX_PATH];
	unsigned int res = GetKeyFileFromStoreDescriptor(s, filename);
	if (res!=LFOk)
		return res;

	return DeleteFileA(filename) ? LFOk : LFDriveNotReady;
}

unsigned int ValidateStoreSettings(LFStoreDescriptor* s)
{
	if (!s)
		return LFIllegalStoreDescriptor;
	if ((s->StoreMode<LFStoreModeInternal) || (s->StoreMode>LFStoreModeExternal))
		return LFIllegalStoreDescriptor;

	// Bei Hybrid-Stores das gemountete Volume eintragen
	if ((s->StoreMode==LFStoreModeHybrid) || (s->StoreMode==LFStoreModeExternal))
		if (IsStoreMounted(s))
		{
			wchar_t szDriveRoot[] = L" :\\";
			szDriveRoot[0] = s->DatPath[0];

			SHFILEINFO sfi;
			if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ATTRIBUTES))
				wcscpy_s(s->LastSeen, 256, sfi.szDisplayName);
		}

	// Datenpfad überprüfen (Indexpfade werden nicht gespeichert, sondern dynamisch vergeben)
	if ((s->StoreMode==LFStoreModeInternal) && (s->AutoLocation))
		GetAutoPath(s, s->DatPath);

	// Hauptindex immer als Unterverzeichnis des Stores
	if ((s->StoreMode!=LFStoreModeHybrid) || (IsStoreMounted(s)))
	{
		strcpy_s(s->IdxPathMain, MAX_PATH, s->DatPath);
		strcat_s(s->IdxPathMain, MAX_PATH, "INDEX\\");
	}
	else
	{
		strcpy_s(s->IdxPathMain, MAX_PATH, "");
	}

	// Für Hybrid-Stores lokalen Hilfsindex eintragen
	if (s->StoreMode==LFStoreModeHybrid)
	{
		GetAutoPath(s, s->IdxPathAux);
		strcat_s(s->IdxPathAux, MAX_PATH, "INDEX\\");
	}
	else
	{
		s->IdxPathAux[0] = '\0';
	}

	return LFOk;
}


void ChooseNewDefaultStore()
{
	for (unsigned int a=0; a<StoreCount; a++)
		if ((StoreCache[a].StoreMode==LFStoreModeInternal) && (strcmp(DefaultStore, StoreCache[a].StoreID)!=0))
		{
			LFMakeDefaultStore(StoreCache[a].StoreID, NULL, true);
			return;
		}

	// Alten Default Store löschen
	strcpy_s(DefaultStore, LFKeySize, "");

	HKEY hive;
	if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &hive)==ERROR_SUCCESS)
	{
		RegDeleteValueA(hive, "DefaultStore");
		RegCloseKey(hive);
	}
}


void LoadRegistry()
{
	bool DefaultStoreOk = false;

	// Stores aus der Registry laden
	HKEY hive;
	if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &hive)!=ERROR_SUCCESS)
		return;

	DWORD Subkeys;
	if (RegQueryInfoKey(hive, NULL, 0, NULL, &Subkeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL)!=ERROR_SUCCESS)
		Subkeys = 0;

	for (DWORD a=0; a<Subkeys; a++)
	{
		// Noch Platz im Cache?
		if (StoreCount>=MaxStores)
			break;

		// Nächsten Key finden
		char key[256];
		DWORD l = 255;
		if (RegEnumKeyA(hive, a, key, l)!=ERROR_SUCCESS)
		{
			RegCloseKey(hive);
			return;
		}

		// Store laden
		if (LoadStoreSettingsFromRegistry(key, &StoreCache[StoreCount]))
			if (ValidateStoreSettings(&StoreCache[StoreCount])==LFOk)
			{
				StoreCache[StoreCount].NeedsCheck = true;
				DefaultStoreOk |= (strcmp(DefaultStore, StoreCache[StoreCount].StoreID)==0);
				StoreCount++;
			}
	}

	RegCloseKey(hive);

	// Ggf. neuen DefaultStore wählen
	if ((!DefaultStoreOk) && (StoreCount))
		ChooseNewDefaultStore();
}

void MountExternal()
{
	DWORD DrivesOnSystem = LFGetLogicalDrives(LFGLD_External);
	char szDriveRoot[] = " :\\";

	for (char cDrive='A'; cDrive<='Z'; cDrive++, DrivesOnSystem>>=1)
	{
		if (!(DrivesOnSystem & 1))
			continue;

		szDriveRoot[0] = cDrive;
		SHFILEINFOA sfi;
		if (SHGetFileInfoA(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES))
			if (sfi.dwAttributes)
				LFMountDrive(cDrive, true);
	}
}

void InitStoreCache()
{
	if (GetMutex(Mutex_Stores))
	{
		if (!Initialized)
		{
			// Default-Store laden
			HKEY k;
			if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &k)==ERROR_SUCCESS)
			{
				DWORD type;
				DWORD sz = LFKeySize;
				RegQueryValueExA(k, "DefaultStore", NULL, &type, (BYTE*)DefaultStore, &sz);
				RegCloseKey(k);
			}

			// Anwendungsordner anlegen
			char tmpStr[MAX_PATH];
			SHGetSpecialFolderPathA(NULL, tmpStr, CSIDL_APPDATA, TRUE);
			strcat_s(tmpStr, MAX_PATH, AppPath);
			CreateDir(tmpStr);

			// Stores aus der Registry
			StoreCount = 0;
			LoadRegistry();

			Initialized = true;
		}

		ReleaseMutex(Mutex_Stores);

		// Externe Laufwerke mounten
		MountExternal();
	}
}


void CreateStoreKey(char* key)
{
	bool unique;
	char chars[38] = { LFKeyChars };

	SYSTEMTIME st;
	GetSystemTime(&st);
	srand(st.wMilliseconds*rand());

	do
	{
		for (unsigned int a=0; a<LFKeyLength; a++)
		{
			int r = rand()%38;
			key[a] = chars[r];
		}
		key[LFKeyLength] = 0;

		unique = true;
		for (unsigned int a=0; a<StoreCount; a++)
			if (strcmp(StoreCache[a].StoreID, key)==0)
			{
				unique = false;
				break;
			}
	}
	while (!unique);
}


void AddStoresToSearchResult(LFSearchResult* res, LFFilter* filter)
{
	for (unsigned int a=0; a<StoreCount; a++)
	{
		if (filter)
			if ((filter->Options.OnlyInternalStores) && (StoreCache[a].StoreMode!=LFStoreModeInternal))
			{
				res->m_HidingItems = true;
				continue;
			}

		res->AddStoreDescriptor(&StoreCache[a], filter);
	}
}

LFStoreDescriptor* FindStore(char* key, HANDLE* lock)
{
	for (unsigned int a=0; a<StoreCount; a++)
		if (strcmp(StoreCache[a].StoreID, key)==0)
		{
			if (lock)
				GetMutexForStore(&StoreCache[a], lock);
			return &StoreCache[a];
		}

	return NULL;
}

LFStoreDescriptor* FindStore(GUID guid, HANDLE* lock)
{
	for (unsigned int a=0; a<StoreCount; a++)
		if (StoreCache[a].guid==guid)
		{
			if (lock)
				GetMutexForStore(&StoreCache[a], lock);
			return &StoreCache[a];
		}

	return NULL;
}

unsigned int FindStores(char** keys)
{
	if (StoreCount)
	{
		*keys = (char*)malloc(LFKeySize*StoreCount);
		char* ptr = *keys;

		for (unsigned int a=0; a<StoreCount; a++)
		{
			strcpy_s(ptr, LFKeySize, StoreCache[a].StoreID);
			ptr += LFKeySize;
		}
	}

	return StoreCount;
}

unsigned int UpdateStore(LFStoreDescriptor* s, bool MakeDefault)
{
	// FileTime setzen
	GetSystemTimeAsFileTime(&s->FileTime);

	// Cache aktualisieren
	LFStoreDescriptor* slot = FindStore(s->StoreID);
	if (!slot)
	{
		if (StoreCount==MaxStores)
			return LFTooManyStores;

		slot = &StoreCache[StoreCount];
		StoreCache[StoreCount++] = *s;
	}
	else
	{
		if (slot!=s)
			*slot = *s;
	}

	unsigned int res = LFOk;
	if ((s->StoreMode==LFStoreModeInternal) || (s->StoreMode==LFStoreModeHybrid))
		res = SaveStoreSettingsToRegistry(s);
	if ((s->StoreMode==LFStoreModeHybrid) || (s->StoreMode==LFStoreModeExternal))
		if ((res==LFOk) && (IsStoreMounted(s)))
			res = SaveStoreSettingsToFile(s);

	// Ggf. Store zum Default Store machen
	if (res==LFOk)
		if ((s->StoreMode==LFStoreModeInternal) && ((MakeDefault) || (DefaultStore[0]=='\0')))
			res = LFMakeDefaultStore(s->StoreID, NULL, true);

	return res;
}

unsigned int DeleteStore(LFStoreDescriptor* s)
{
	LFStoreDescriptor victim = *s;

	// Aus dem Cache entfernen
	for (unsigned int a=0; a<StoreCount; a++)
		if (strcmp(StoreCache[a].StoreID, s->StoreID)==0)
		{
			if (a<StoreCount-1)
			{
				HANDLE MoveLock;
				if (!GetMutexForStore(&StoreCache[StoreCount-1], &MoveLock))
					return LFMutexError;
				StoreCache[a] = StoreCache[--StoreCount];
				ReleaseMutexForStore(MoveLock);
			}
			else
			{
				StoreCount--;
			}
			break;
		}

	// Ggf. ersten Store als neuen Default Store
	if (strcmp(victim.StoreID, DefaultStore)==0)
		ChooseNewDefaultStore();

	// Einstellungen
	unsigned int res = LFOk;
	if ((victim.StoreMode==LFStoreModeInternal) || (victim.StoreMode==LFStoreModeHybrid))
		res = DeleteStoreSettingsFromRegistry(&victim);
	if ((victim.StoreMode==LFStoreModeHybrid) || (victim.StoreMode==LFStoreModeExternal))
		if ((res==LFOk) && (IsStoreMounted(&victim)))
			res = DeleteStoreSettingsFromFile(&victim);

	return res;
}


LFCore_API bool LFDefaultStoreAvailable()
{
	bool res = false;

	if (GetMutex(Mutex_Stores))
	{
		res = (DefaultStore[0]!='\0');
		ReleaseMutex(Mutex_Stores);
	}

	return res;
}

LFCore_API char* LFGetDefaultStore()
{
	char* s = (char*)(malloc(LFKeySize));
	*s = '\0';

	if (GetMutex(Mutex_Stores))
	{
		strcpy_s(s, LFKeySize, DefaultStore);
		ReleaseMutex(Mutex_Stores);
	}

	return s;
}

LFCore_API void LFGetDefaultStoreName(char* name, size_t cCount)
{
	LoadStringA(LFCoreModuleHandle, IDS_DefaultStore, name, (int)cCount);
}

LFCore_API unsigned int LFGetStoreCount()
{
	unsigned int res = 0;

	if (GetMutex(Mutex_Stores))
	{
		res = StoreCount;
		ReleaseMutex(Mutex_Stores);
	}

	return res;
}


LFCore_API unsigned int LFMountDrive(char d, bool InternalCall)
{
	char mask[] = " :\\*.store";
	mask[0] = d;
	bool changeOccured = false;
	unsigned int res = LFOk;

	WIN32_FIND_DATAA ffd;
	HANDLE hFind = FindFirstFileA(mask, &ffd);

	if (hFind!=INVALID_HANDLE_VALUE)
		do
		{
			// Vollständigen Dateinamen zusammensetzen
			char f[MAX_PATH] = " :\\";
			f[0] = d;
			strcat_s(f, MAX_PATH, ffd.cFileName);

			LFStoreDescriptor* s = LFAllocStoreDescriptor();
			if (LoadStoreSettingsFromFile(f, s)==true)
			{
				if (!GetMutex(Mutex_Stores))
				{
					LFFreeStoreDescriptor(s);
					res = LFMutexError;
					continue;
				}

				// Lokal gültigen Schlüssel eintragen
				CreateStoreKey(s->StoreID);

				// Store mit der selben GUID suchen
				LFStoreDescriptor* slot = FindStore(s->guid);
				if (!slot)
				{
					// Nicht gefunden: der Store wird hier als externer Store behandelt
					s->StoreMode = LFStoreModeExternal;

					// Zum Cache hinzufügen
					if (StoreCount<MaxStores)
					{
						StoreCache[StoreCount] = *s;
						slot = &StoreCache[StoreCount++];		// Slot zeigt auf Eintrag
					}
					else
					{
						res = LFTooManyStores;
					}
				}
				else
				{
					// Wenn der Store kein Hybrid-Store ist, wird er doppelt gemountet. Überspringen!
					if (slot->StoreMode!=LFStoreModeHybrid)
					{
						slot = NULL;
					}
					else
					{
						// Name, Kommentar und Dateizeit aktualisieren
						wcscpy_s(slot->StoreName, 256, s->StoreName);
						wcscpy_s(slot->Comment, 256, s->Comment);
						slot->FileTime = s->FileTime;
					}
				}

				if (slot)
				{
					strcpy_s(slot->DatPath, MAX_PATH, s->DatPath);
					slot->DatPath[0] = d;
					slot->NeedsCheck = true;

					ValidateStoreSettings(slot);
					changeOccured = true;

					if (slot->StoreMode!=LFStoreModeHybrid)
						goto Finish1;

					// Hybrid-Stores in der Registry abspeichern, damit LastSeen aktualisiert wird
					SaveStoreSettingsToRegistry(slot);

					HANDLE StoreLock;
					if (!GetMutexForStore(slot, &StoreLock))
						goto Finish1;

					ReleaseMutex(Mutex_Stores);
					res = CopyDir(slot->IdxPathMain, slot->IdxPathAux);
					ReleaseMutexForStore(StoreLock);
					goto Finish2;
				}

Finish1:
				ReleaseMutex(Mutex_Stores);
Finish2:
				;
			}

			LFFreeStoreDescriptor(s);
		} while (FindNextFileA(hFind, &ffd));

	FindClose(hFind);

	if (!InternalCall)
	{
		SendLFNotifyMessage(changeOccured ? LFMessages.StoresChanged : LFMessages.DrivesChanged, changeOccured ? LFMSGF_ExtHybStores : 0, NULL);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}

	return res;
}

LFCore_API unsigned int LFUnmountDrive(char d, bool InternalCall)
{
	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	bool changeOccured = false;
	DriveTypes[d-'A'] = DRIVE_UNKNOWN;
	unsigned int res = LFOk;

	for (unsigned int a=0; a<StoreCount; a++)
		if (IsStoreMounted(&StoreCache[a]))
			if ((StoreCache[a].DatPath[0]==d) && (StoreCache[a].StoreMode!=LFStoreModeInternal))
			{
				HANDLE StoreLock;
				if (!GetMutexForStore(&StoreCache[a], &StoreLock))
				{
					res = LFMutexError;
					continue;
				}

				switch (StoreCache[a].StoreMode)
				{
				case LFStoreModeHybrid:
					strcpy_s(StoreCache[a].DatPath, MAX_PATH, "");
					strcpy_s(StoreCache[a].IdxPathMain, MAX_PATH, "");
					changeOccured = true;
					break;
				case LFStoreModeExternal:
					if (a<StoreCount-1)
					{
						HANDLE MoveLock;
						if (!GetMutexForStore(&StoreCache[StoreCount-1], &MoveLock))
						{
							ReleaseMutexForStore(StoreLock);
							res = LFMutexError;
							continue;
						}

						StoreCache[a--] = StoreCache[--StoreCount];
						ReleaseMutexForStore(MoveLock);
					}
					else
					{
						StoreCount--;
					}
					changeOccured = true;
					break;
				}

				ReleaseMutexForStore(StoreLock);
			}

	ReleaseMutex(Mutex_Stores);

	if (!InternalCall)
	{
		SendLFNotifyMessage(changeOccured ? LFMessages.StoresChanged : LFMessages.DrivesChanged, changeOccured ? LFMSGF_ExtHybStores : 0, NULL);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}

	return res;
}

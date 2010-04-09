#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFVariantData.h"
#include "Mutex.h"
#include "StoreCache.h"
#include <io.h>
#include <malloc.h>
#include <objbase.h>
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


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;
extern LFMessageIDs LFMessages;
extern unsigned int DriveTypes[26];


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
		int ccount = StringFromGUID2(s->GUID, szGUID, MAX_PATH);

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
	strcat_s(p, MAX_PATH, "\\liquidFOLDERS\\");
	AppendGUID(s, p);
}

BOOL FolderExists(char* path)
{
	if (_access(path, 0)==0)
	{
		struct stat status;
		stat(path, &status);
		return (status.st_mode & S_IFDIR);
	}

	return FALSE;
}

unsigned int GetKeyFileFromStoreDescriptor(LFStoreDescriptor* s, char* f)
{
	if (s==NULL)
		return LFIllegalStoreDescriptor;
	if ((s->StoreMode!=LFStoreModeHybrid) && (s->StoreMode!=LFStoreModeExternal))
		return LFIllegalStoreDescriptor;

	if (!IsStoreMounted(s))
		return LFStoreNotMounted;

	OLECHAR szGUID[MAX_PATH];
	int ccount = StringFromGUID2(s->GUID, szGUID, MAX_PATH);
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
	if (strcmp(key, "")==0)
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

		sz = sizeof(s->GUID);
		if (RegQueryValueExA(k, "GUID", 0, NULL, (BYTE*)&s->GUID, &sz)!=ERROR_SUCCESS)
			res = false;

		sz = sizeof(s->CreationTime);
		RegQueryValueExA(k, "CreationTime", 0, NULL, (BYTE*)&s->CreationTime, &sz);

		sz = sizeof(s->FileTime);
		RegQueryValueExA(k, "FileTime", 0, NULL, (BYTE*)&s->FileTime, &sz);

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
	if (strcmp(filename, "")==0)
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
	if (s==NULL)
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
		if (RegSetValueExA(k, "GUID", 0, REG_BINARY, (BYTE*)&s->GUID, sizeof(GUID))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if (RegSetValueExA(k, "CreationTime", 0, REG_BINARY, (BYTE*)&s->CreationTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if (RegSetValueExA(k, "FileTime", 0, REG_BINARY, (BYTE*)&s->FileTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;
		if (RegSetValueExA(k, "AutoLocation", 0, REG_DWORD, (BYTE*)&s->AutoLocation, sizeof(BOOL))!=ERROR_SUCCESS)
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
	if (s==NULL)
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
	if (s==NULL)
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
	if (s==NULL)
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
	if (s->StoreMode==LFStoreModeInternal)
	{
		if (s->AutoLocation)
		{
			GetAutoPath(s, s->DatPath);
		}
		else
		{
			//if (!FolderExists(&s->DatPath[0]))
			//	return LFIllegalPhysicalPath;
		}
	}
	else
	{
		//if (IsStoreMounted(s))
			//if (!FolderExists(&s->DatPath[0]))
			//	return LFIllegalPhysicalPath;
	}

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
	bool found = false;
	for (unsigned int a=0; a<StoreCount; a++)
		if ((StoreCache[a].StoreMode==LFStoreModeInternal) && (strcmp(DefaultStore, StoreCache[a].StoreID)!=0))
		{
			LFMakeDefaultStore(StoreCache[a].StoreID, NULL, TRUE);
			found = true;
			break;
		}

	if (!found)
	{
		// Alten Default Store löschen
		strcpy_s(DefaultStore, LFKeySize, "");

		HKEY hive;
		if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &hive)==ERROR_SUCCESS)
		{
			RegDeleteValueA(hive, "DefaultStore");
			RegCloseKey(hive);
		}
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
				LFMountDrive(cDrive);
	}
}

void InitStoreCache()
{
	if (Initialized)
		return;
	Initialized = true;

	// Default-Store laden
	HKEY k;
	if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &k)==ERROR_SUCCESS)
	{
		DWORD type;
		DWORD sz = LFKeySize;
		RegQueryValueExA(k, "DefaultStore", NULL, &type, (BYTE*)DefaultStore, &sz);
		RegCloseKey(k);
	}

	// Stores aus der Registry
	LoadRegistry();

	// Externe Laufwerke mounten
	MountExternal();
}


void CreateStoreKey(char* key)
{
	bool unique;
	char chars[38] = { LFKeyChars };
	srand(rand());

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

LFItemDescriptor* StoreDescriptor2ItemDescriptor(LFStoreDescriptor* s)
{
	LFFilter* nf = LFAllocFilter();
	nf->Mode = LFFilterModeStoreHome;
	nf->AllowSubfolders = true;
	strcpy_s(nf->StoreID, LFKeySize, s->StoreID);
	wcscpy_s(nf->Name, 256, s->StoreName);

	LFItemDescriptor* d = LFAllocItemDescriptor();
	BOOL IsMounted = IsStoreMounted(s);

	if (strcmp(s->StoreID, DefaultStore)==0)
	{
		d->IconID = IDI_STORE_Default;
		d->Type |= LFTypeDefaultStore;
		wchar_t ds[256];
		LoadStringW(LFCoreModuleHandle, IDS_DefaultStore, ds, 256);
		SetAttributeUnicodeString(d, LFAttrHint, ds);
	}
	else
	{
		d->IconID = (s->StoreMode==LFStoreModeInternal ? IDI_STORE_Empty : IDI_Bag);
		if ((s->StoreMode==LFStoreModeHybrid) || (s->StoreMode==LFStoreModeExternal))
			if (wcscmp(s->LastSeen, L"")!=0)
			{
				wchar_t ls[256];
				LoadStringW(LFCoreModuleHandle, IsMounted ? IDS_SeenOn :IDS_LastSeen, ls, 256);
				wchar_t hint[256];
				wsprintf(hint, ls, s->LastSeen);
				SetAttributeUnicodeString(d, LFAttrHint, hint);
			}
	}

	if (!IsMounted)
		d->Type |= LFTypeGhosted | LFTypeNotMounted;

	d->CategoryID = s->StoreMode;
	d->Type |= LFTypeStore;
	SetAttributeUnicodeString(d, LFAttrFileName, s->StoreName);
	SetAttributeUnicodeString(d, LFAttrComment, s->Comment);
	SetAttributeAnsiString(d, LFAttrStoreID, s->StoreID);
	SetAttributeAnsiString(d, LFAttrFileID, s->StoreID);
	SetAttributeTime(d, LFAttrCreationTime, s->CreationTime);
	SetAttributeTime(d, LFAttrFileTime, s->FileTime);
	d->NextFilter = nf;

	return d;
}

void AddStores(LFSearchResult* res)
{
	for (unsigned int a=0; a<StoreCount; a++)
		res->AddStoreDescriptor(&StoreCache[a]);
}

LFStoreDescriptor* FindStore(char* key, HANDLE* lock)
{
	for (unsigned int a=0; a<StoreCount; a++)
		if (strcmp(StoreCache[a].StoreID, key)==0)
		{
			if (lock)
				*lock = GetMutexForStore(&StoreCache[a]);
			return &StoreCache[a];
		}

	return NULL;
}

LFStoreDescriptor* FindStore(_GUID guid, HANDLE* lock)
{
	for (unsigned int a=0; a<StoreCount; a++)
		if (StoreCache[a].GUID == guid)
		{
			if (lock)
				*lock = GetMutexForStore(&StoreCache[a]);
			return &StoreCache[a];
		}

	return NULL;
}

unsigned int UpdateStore(LFStoreDescriptor* s, BOOL MakeDefault)
{
	// FileTime setzen
	SYSTEMTIME st;
	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &s->FileTime);

	// Cache aktualisieren
	LFStoreDescriptor* slot = FindStore(s->StoreID);
	if (slot==NULL)
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

	if (res!=LFOk)
		return res;

	// Ggf. Store zum Default Store machen
	if ((s->StoreMode==LFStoreModeInternal) && ((MakeDefault) || (strcmp(DefaultStore, "")==0)))
		res = LFMakeDefaultStore(s->StoreID, NULL, TRUE);

	return res;
}

unsigned int DeleteStore(LFStoreDescriptor* s)
{
	unsigned int res = LFOk;
	if ((s->StoreMode==LFStoreModeInternal) || (s->StoreMode==LFStoreModeHybrid))
		res = DeleteStoreSettingsFromRegistry(s);
	if ((s->StoreMode==LFStoreModeHybrid) || (s->StoreMode==LFStoreModeExternal))
		if ((res==LFOk) && (IsStoreMounted(s)))
			res = DeleteStoreSettingsFromFile(s);

	if (res!=LFOk)
		return res;

	// Ggf. ersten Store als neuen Default Store
	if (strcmp(s->StoreID, DefaultStore)==0)
		ChooseNewDefaultStore();

	// Aus dem Cache entfernen
	for (unsigned int a=0; a<StoreCount; a++)
		if (strcmp(StoreCache[a].StoreID, s->StoreID)==0)
		{
			StoreCache[a] = StoreCache[--StoreCount];
			break;
		}

	return res;
}


LFCore_API bool LFDefaultStoreAvailable()
{
	bool res = false;

	if (GetMutex(Mutex_Stores))
	{
		res = (strcmp(DefaultStore, "")!=0);
		ReleaseMutex(Mutex_Stores);
	}

	return res;
}

LFCore_API char* LFGetDefaultStore()
{
	char* s = static_cast<char*>(malloc(LFKeySize));
	strcpy_s(s, LFKeySize, "");

	if (GetMutex(Mutex_Stores))
	{
		strcpy_s(s, LFKeySize, DefaultStore);
		ReleaseMutex(Mutex_Stores);
	}

	return s;
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


LFCore_API void LFMountDrive(char d)
{
	char mask[] = " :\\*.store";
	mask[0] = d;
	bool changeOccured = false;

	WIN32_FIND_DATAA ffd;
	HANDLE hFind = FindFirstFileA(mask, &ffd);

	if (hFind!=INVALID_HANDLE_VALUE)
		do
		{
			if (!GetMutex(Mutex_Stores))
				continue;

			// Vollständigen Dateinamen zusammensetzen
			char f[MAX_PATH] = " :\\";
			f[0] = d;
			strcat_s(f, MAX_PATH, ffd.cFileName);

			LFStoreDescriptor* s = LFAllocStoreDescriptor();
			if (LoadStoreSettingsFromFile(f, s)==true)
			{
				// Lokal gültigen Schlüssel eintragen
				CreateStoreKey(s->StoreID);

				// Store mit der selben GUID suchen
				LFStoreDescriptor* slot = FindStore(s->GUID);
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
					strncpy_s(slot->DatPath, MAX_PATH, mask, 3);
					AppendGUID(slot, slot->DatPath);
					ValidateStoreSettings(slot);
					changeOccured = true;

					// Hybrid-Stores in der Registry abspeichern, damit LastSeen aktualisiert wird
					if (slot->StoreMode==LFStoreModeHybrid)
						SaveStoreSettingsToRegistry(slot);
				}
			}

			LFFreeStoreDescriptor(s);
			ReleaseMutex(Mutex_Stores);
		} while (FindNextFileA(hFind, &ffd));

	FindClose(hFind);

	if (changeOccured)
		SendNotifyMessage(HWND_BROADCAST, LFMessages.StoresChanged, LFMSGF_ExtHybStores, NULL);
}

LFCore_API void LFUnmountDrive(char d)
{
	if (!GetMutex(Mutex_Stores))
		return;

	bool changeOccured = false;
	DriveTypes[d-'A'] = DRIVE_UNKNOWN;

	for (unsigned int a=0; a<StoreCount; a++)
		if (IsStoreMounted(&StoreCache[a]))
			if ((StoreCache[a].DatPath[0]==d) && (StoreCache[a].StoreMode!=LFStoreModeInternal))
			{
				HANDLE StoreLock = GetMutexForStore(&StoreCache[a]);

				switch (StoreCache[a].StoreMode)
				{
				case LFStoreModeHybrid:
					strcpy_s(StoreCache[a].DatPath, MAX_PATH, "");
					strcpy_s(StoreCache[a].IdxPathMain, MAX_PATH, "");
					strcpy_s(StoreCache[a].IdxPathAux, MAX_PATH, "");
					changeOccured = true;
					break;
				case LFStoreModeExternal:
					StoreCache[a--] = StoreCache[--StoreCount];
					changeOccured = true;
					break;
				}

				ReleaseMutexForStore(StoreLock);
			}

	ReleaseMutex(Mutex_Stores);

	if (changeOccured)
		SendNotifyMessage(HWND_BROADCAST, LFMessages.StoresChanged, LFMSGF_ExtHybStores, NULL);
}

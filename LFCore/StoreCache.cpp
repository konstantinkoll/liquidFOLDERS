
#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>
#include <io.h>
#include <malloc.h>
#include <shellapi.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>


#pragma data_seg(".shared")

bool Initialized = false;

#pragma data_seg()


#pragma bss_seg(".stores")

char DefaultStore[LFKeySize];
unsigned int StoreCount;
LFStoreDescriptor StoreCache[MaxStores];

#pragma data_seg()
#pragma comment(linker, "/SECTION:.stores,RWS")


extern HANDLE Mutex_Stores;
extern HMODULE LFCoreModuleHandle;
extern LFMessageIDs LFMessages;
extern unsigned int VolumeTypes[26];


void AppendGUID(LFStoreDescriptor* s, wchar_t* p)
{
	if (s)
	{
		OLECHAR szGUID[MAX_PATH];
		int ccount = StringFromGUID2(s->guid, szGUID, MAX_PATH);

		if (ccount)
		{
			wcscat_s(p, MAX_PATH, szGUID);
			wcscat_s(p, MAX_PATH, L"\\");
		}
	}
}

void GetAutoPath(LFStoreDescriptor* s, wchar_t* p)
{
	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, L"Stores", p);
	wcscat_s(p, MAX_PATH, L"\\");
	AppendGUID(s, p);
}

unsigned int GetKeyFileFromStoreDescriptor(LFStoreDescriptor* s, wchar_t* f)
{
	if (!s)
		return LFIllegalStoreDescriptor;
	if ((s->StoreMode!=LFStoreModeHybrid) && (s->StoreMode!=LFStoreModeExternal))
		return LFIllegalStoreDescriptor;

	if (!LFIsStoreMounted(s))
		return LFStoreNotMounted;

	OLECHAR szGUID[MAX_PATH];
	int ccount = StringFromGUID2(s->guid, szGUID, MAX_PATH);
	if (!ccount)
		return LFIllegalStoreDescriptor;

	wcsncpy_s(f, MAX_PATH, s->DatPath, 3);		// .store-Datei immer im Hauptverzeichnis
	wcscat_s(f, MAX_PATH, szGUID);
	wcscat_s(f, MAX_PATH, L".store");

	return LFOk;
}

bool LoadStoreSettingsFromRegistry(char* key, LFStoreDescriptor* s)
{
	assert(s);

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
		if (RegQueryValueEx(k, L"Name", 0, NULL, (BYTE*)&s->StoreName, &sz)!=ERROR_SUCCESS)
			res = false;

		sz = sizeof(s->StoreComment);
		RegQueryValueEx(k, L"Comment", 0, NULL, (BYTE*)&s->StoreComment, &sz);

		sz = sizeof(s->StoreMode);
		if (RegQueryValueEx(k, L"Mode", 0, NULL, (BYTE*)&s->StoreMode, &sz)!=ERROR_SUCCESS)
			res = false;

		sz = sizeof(s->guid);
		if (RegQueryValueEx(k, L"GUID", 0, NULL, (BYTE*)&s->guid, &sz)!=ERROR_SUCCESS)
			res = false;

		sz = sizeof(s->CreationTime);
		RegQueryValueEx(k, L"CreationTime", 0, NULL, (BYTE*)&s->CreationTime, &sz);

		sz = sizeof(s->FileTime);
		RegQueryValueEx(k, L"FileTime", 0, NULL, (BYTE*)&s->FileTime, &sz);

		sz = sizeof(s->MaintenanceTime);
		RegQueryValueEx(k, L"MaintenanceTime", 0, NULL, (BYTE*)&s->MaintenanceTime, &sz);

		sz = sizeof(s->AutoLocation);
		if (RegQueryValueEx(k, L"AutoLocation", 0, NULL, (BYTE*)&s->AutoLocation, &sz)!=ERROR_SUCCESS)
			res = false;

		sz = sizeof(s->IndexVersion);
		if (RegQueryValueEx(k, L"IndexVersion", 0, NULL, (BYTE*)&s->IndexVersion, &sz)!=ERROR_SUCCESS)
			res = false;

		switch (s->StoreMode)
		{
		case LFStoreModeInternal:
			s->Source = LFTypeSourceInternal;

			sz = sizeof(s->DatPath);
			if (RegQueryValueEx(k, L"Path", 0, NULL, (BYTE*)s->DatPath, &sz)!=ERROR_SUCCESS)
				if (!s->AutoLocation)
					res = false;

			break;
		case LFStoreModeHybrid:
			sz = sizeof(s->LastSeen);
			RegQueryValueEx(k, L"LastSeen", 0, NULL, (BYTE*)&s->LastSeen, &sz);
		default:
			sz = sizeof(s->Source);
			if (RegQueryValueEx(k, L"Source", 0, NULL, (BYTE*)&s->Source, &sz)!=ERROR_SUCCESS)
				res = false;
		}

		RegCloseKey(k);
	}

	return res;
}

bool LoadStoreSettingsFromFile(wchar_t* filename, LFStoreDescriptor* s)
{
	assert(s);

	if (!filename)
		return false;
	if (filename[0]==L'\0')
		return false;

	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return false;

	ZeroMemory(s, sizeof(LFStoreDescriptor));

	DWORD wmRead;
	bool res = (ReadFile(hFile, s, sizeof(LFStoreDescriptor), &wmRead, NULL)==TRUE);
	res &= (wmRead>=3168);	// Size of v1 descriptor and minimum size
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

		if (RegSetValueEx(k, L"Name", 0, REG_SZ, (BYTE*)s->StoreName, (DWORD)wcslen(s->StoreName)*sizeof(wchar_t))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"Comment", 0, REG_SZ, (BYTE*)s->StoreComment, (DWORD)wcslen(s->StoreComment)*sizeof(wchar_t))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"Mode", 0, REG_DWORD, (BYTE*)&s->StoreMode, sizeof(unsigned int))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"GUID", 0, REG_BINARY, (BYTE*)&s->guid, sizeof(GUID))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"CreationTime", 0, REG_BINARY, (BYTE*)&s->CreationTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"FileTime", 0, REG_BINARY, (BYTE*)&s->FileTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"MaintenanceTime", 0, REG_BINARY, (BYTE*)&s->MaintenanceTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"AutoLocation", 0, REG_DWORD, (BYTE*)&s->AutoLocation, sizeof(unsigned int))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"IndexVersion", 0, REG_DWORD, (BYTE*)&s->IndexVersion, sizeof(unsigned int))!=ERROR_SUCCESS)
			res = LFRegistryError;

		switch (s->StoreMode)
		{
		case LFStoreModeInternal:
			if (!s->AutoLocation)
				if (RegSetValueEx(k, L"Path", 0, REG_SZ, (BYTE*)s->DatPath, (DWORD)wcslen(s->DatPath)*sizeof(wchar_t))!=ERROR_SUCCESS)
					res = LFRegistryError;
			break;
		case LFStoreModeHybrid:
			if (RegSetValueEx(k, L"LastSeen", 0, REG_SZ, (BYTE*)s->LastSeen, (DWORD)wcslen(s->LastSeen)*sizeof(wchar_t))!=ERROR_SUCCESS)
				res = LFRegistryError;
		default:
			if (RegSetValueEx(k, L"Source", 0, REG_DWORD, (BYTE*)&s->Source, sizeof(unsigned int))!=ERROR_SUCCESS)
				res = LFRegistryError;
		}

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

	wchar_t filename[MAX_PATH];
	unsigned int res = GetKeyFileFromStoreDescriptor(s, filename);
	if (res!=LFOk)
		return res;

	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_WRITE_THROUGH, NULL);
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

	wchar_t filename[MAX_PATH];
	unsigned int res = GetKeyFileFromStoreDescriptor(s, filename);
	if (res!=LFOk)
		return res;

	return DeleteFile(filename) ? LFOk : LFDriveNotReady;
}

unsigned int ValidateStoreSettings(LFStoreDescriptor* s, int Source)
{
	if (!s)
		return LFIllegalStoreDescriptor;
	if ((s->StoreMode<LFStoreModeInternal) || (s->StoreMode>LFStoreModeExternal))
		return LFIllegalStoreDescriptor;

	// Bei Hybrid-Stores das gemountete Volume eintragen
	if ((s->StoreMode==LFStoreModeHybrid) || (s->StoreMode==LFStoreModeExternal))
		if (LFIsStoreMounted(s))
		{
			wchar_t szDriveRoot[] = L" :\\";
			szDriveRoot[0] = s->DatPath[0];

			SHFILEINFO sfi;
			if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME))
				wcscpy_s(s->LastSeen, 256, sfi.szDisplayName);

			if (Source==-1)
				Source = LFGetSourceForDrive(s->DatPath[0] & 0xFF);

			assert((Source & LFTypeMaskSource)==Source);
			s->Source = Source;
		}

	// Datenpfad überprüfen (Indexpfade werden nicht gespeichert, sondern dynamisch vergeben)
	if ((s->StoreMode==LFStoreModeInternal) && (s->AutoLocation))
		GetAutoPath(s, s->DatPath);

	// Hauptindex immer als Unterverzeichnis des Stores
	if ((s->StoreMode!=LFStoreModeHybrid) || (LFIsStoreMounted(s)))
	{
		wcscpy_s(s->IdxPathMain, MAX_PATH, s->DatPath);
		wcscat_s(s->IdxPathMain, MAX_PATH, L"INDEX\\");
	}
	else
	{
		wcscpy_s(s->IdxPathMain, MAX_PATH, L"");
	}

	// Für Hybrid-Stores lokalen Hilfsindex eintragen
	if (s->StoreMode==LFStoreModeHybrid)
	{
		GetAutoPath(s, s->IdxPathAux);
		wcscat_s(s->IdxPathAux, MAX_PATH, L"INDEX\\");
	}
	else
	{
		s->IdxPathAux[0] = L'\0';
	}

	return LFOk;
}


void ChooseNewDefaultStore()
{
	int no = -1;

	for (unsigned int a=0; a<StoreCount; a++)
		if (strcmp(DefaultStore, StoreCache[a].StoreID)!=0)
			if ((no==-1) || (StoreCache[a].StoreMode==LFStoreModeInternal))
				no = a;

	if (no!=-1)
	{
		LFMakeDefaultStore(StoreCache[no].StoreID, NULL, true);
		return;
	}

	// Default Store löschen
	DefaultStore[0] = '\0';

	HKEY hive;
	if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &hive)==ERROR_SUCCESS)
	{
		RegDeleteValue(hive, L"DefaultStore");
		RegCloseKey(hive);
	}
}


__forceinline void LoadRegistry()
{
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
				StoreCount++;
			}
	}

	RegCloseKey(hive);
}

void MountExternal()
{
	DWORD DrivesOnSystem = LFGetLogicalDrives(LFGLD_External);
	wchar_t szDriveRoot[4] = L" :\\";

	for (char cDrive='A'; cDrive<='Z'; cDrive++, DrivesOnSystem>>=1)
	{
		if ((DrivesOnSystem & 1)==0)
			continue;

		szDriveRoot[0] = cDrive;
		SHFILEINFO sfi;
		if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES))
			if (sfi.dwAttributes)
				MountDrive(cDrive, true);
	}
}

void InitStoreCache()
{
	if (GetMutex(Mutex_Stores))
	{
		if (!Initialized)
		{
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
			StoreCount = 0;
			LoadRegistry();

			// Externe Laufwerke mounten
			MountExternal();

			// Standard-Store
			bool DefaultStoreOk = false;

			for (unsigned int a=0; a<StoreCount; a++)
				if (strcmp(DefaultStore, StoreCache[a].StoreID)==0)
				{
					DefaultStoreOk = true;
					break;
				}

			// Ggf. neuen Standard-Store wählen
			if ((!DefaultStoreOk) && (StoreCount))
				ChooseNewDefaultStore();
		}

		ReleaseMutex(Mutex_Stores);
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
		res->AddStoreDescriptor(&StoreCache[a], filter);
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

LFStoreDescriptor* FindStore(wchar_t* datpath, HANDLE* lock)
{
	for (unsigned int a=0; a<StoreCount; a++)
		if (wcscmp(StoreCache[a].DatPath, datpath)==0)
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

unsigned int UpdateStore(LFStoreDescriptor* s, bool UpdateTime, bool MakeDefault)
{
	// FileTime setzen
	if (UpdateTime)
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
		if ((res==LFOk) && (LFIsStoreMounted(s)))
			res = SaveStoreSettingsToFile(s);

	// Ggf. Store zum Default Store machen
	if (res==LFOk)
		if ((MakeDefault) || (DefaultStore[0]=='\0'))
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

	// Ggf. anderen Store als neuen Default Store
	if (strcmp(victim.StoreID, DefaultStore)==0)
		ChooseNewDefaultStore();

	// Einstellungen
	unsigned int res = LFOk;
	if ((victim.StoreMode==LFStoreModeInternal) || (victim.StoreMode==LFStoreModeHybrid))
		res = DeleteStoreSettingsFromRegistry(&victim);
	if ((victim.StoreMode==LFStoreModeHybrid) || (victim.StoreMode==LFStoreModeExternal))
		if ((res==LFOk) && (LFIsStoreMounted(&victim)))
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

LFCore_API void LFGetDefaultStoreName(wchar_t* name, size_t cCount)
{
	LoadString(LFCoreModuleHandle, IDS_DefaultStore, name, (int)cCount);
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

LFCore_API bool LFStoresOnVolume(char d)
{
	bool res = false;

	if (GetMutex(Mutex_Stores))
	{
		for (unsigned int a=0; a<StoreCount; a++)
			if (LFIsStoreMounted(&StoreCache[a]))
				if (StoreCache[a].DatPath[0]==d)
				{
					res = true;
					break;
				}

		ReleaseMutex(Mutex_Stores);
	}

	return res;
}

LFCore_API unsigned int LFGetStores(char** keys, unsigned int* count)
{
	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	*count = FindStores(keys);
	ReleaseMutex(Mutex_Stores);

	return LFOk;
}

unsigned int MountDrive(char d, bool InternalCall)
{
	wchar_t mask[] = L" :\\*.store";
	mask[0] = d;
	bool ChangeOccured = false;
	unsigned int Source = LFGetSourceForDrive(d);
	unsigned int res = LFOk;

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(mask, &ffd);

	if (hFind!=INVALID_HANDLE_VALUE)
		do
		{
			// Vollständigen Dateinamen zusammensetzen
			wchar_t f[MAX_PATH] = L" :\\";
			f[0] = d;
			wcscat_s(f, MAX_PATH, ffd.cFileName);

			LFStoreDescriptor s;
			if (LoadStoreSettingsFromFile(f, &s)==true)
			{
				// Korrekter Mode?
				if ((s.StoreMode!=LFStoreModeHybrid) && (s.StoreMode!=LFStoreModeExternal))
					continue;

				if (!GetMutex(Mutex_Stores))
				{
					res = LFMutexError;
					continue;
				}

				// Lokal gültigen Schlüssel eintragen
				CreateStoreKey(s.StoreID);

				// Store mit der selben GUID suchen
				LFStoreDescriptor* slot = FindStore(s.guid);
				if (!slot)
				{
					// Nicht gefunden: der Store wird hier als externer Store behandelt
					s.StoreMode = LFStoreModeExternal;

					// Zum Cache hinzufügen
					if (StoreCount<MaxStores)
					{
						StoreCache[StoreCount] = s;
						slot = &StoreCache[StoreCount++];		// Slot zeigt auf Eintrag
					}
					else
					{
						res = LFTooManyStores;
					}
				}
				else
				{
					// Wenn der Store kein Hybrid-Store ist, würde er doppelt gemountet. Überspringen!
					if (slot->StoreMode!=LFStoreModeHybrid)
					{
						slot = NULL;
					}
					else
					{
						// Name, Kommentar und Dateizeit aktualisieren
						wcscpy_s(slot->StoreName, 256, s.StoreName);
						wcscpy_s(slot->StoreComment, 256, s.StoreComment);
						slot->FileTime = s.FileTime;
					}
				}

				if (slot)
				{
					wcscpy_s(slot->DatPath, MAX_PATH, s.DatPath);
					slot->DatPath[0] = d;
					slot->NeedsCheck = true;

					ValidateStoreSettings(slot, Source);
					ChangeOccured = true;

					if (slot->StoreMode!=LFStoreModeHybrid)
						goto Finish;

					// Hybrid-Stores in der Registry abspeichern, damit LastSeen aktualisiert wird
					SaveStoreSettingsToRegistry(slot);

					HANDLE StoreLock;
					if (!GetMutexForStore(slot, &StoreLock))
						goto Finish;

					ReleaseMutex(Mutex_Stores);
					res = CopyDir(slot->IdxPathMain, slot->IdxPathAux);

					if (!InternalCall)
						SendShellNotifyMessage(SHCNE_UPDATEITEM, slot->StoreID);

					ReleaseMutexForStore(StoreLock);
					continue;
				}

Finish:
				ReleaseMutex(Mutex_Stores);
			}
		}
		while (FindNextFile(hFind, &ffd));

	FindClose(hFind);

	if (!InternalCall)
	{
		SendLFNotifyMessage(ChangeOccured ? LFMessages.StoresChanged : LFMessages.VolumesChanged, NULL);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}

	return res;
}

unsigned int UnmountDrive(char d, bool InternalCall)
{
	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	bool ChangeOccured = false;
	bool RemovedDefaultStore = false;
	VolumeTypes[d-'A'] = DRIVE_UNKNOWN;
	unsigned int res = LFOk;

	char NotifyIDs[MaxStores][LFKeySize];
	unsigned int NotifyCount = 0;

	for (unsigned int a=0; a<StoreCount; a++)
		if (LFIsStoreMounted(&StoreCache[a]))
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
					StoreCache[a].DatPath[0] = StoreCache[a].IdxPathMain[0] = '\0';
					strcpy_s(NotifyIDs[NotifyCount++], LFKeySize, StoreCache[a].StoreID);
					ChangeOccured = true;
					break;
				case LFStoreModeExternal:
					RemovedDefaultStore |= (strcmp(StoreCache[a].StoreID, DefaultStore)==0);
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
					ChangeOccured = true;
					break;
				}

				ReleaseMutexForStore(StoreLock);
			}

	// Ggf. anderen Store als neuen Default Store
	if (RemovedDefaultStore)
		ChooseNewDefaultStore();

	ReleaseMutex(Mutex_Stores);

	if (!InternalCall)
	{
		SendLFNotifyMessage(ChangeOccured ? LFMessages.StoresChanged : LFMessages.VolumesChanged, NULL);
		for (unsigned int a=0; a<NotifyCount; a++)
			SendShellNotifyMessage(SHCNE_UPDATEITEM, NotifyIDs[a]);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}

	return res;
}

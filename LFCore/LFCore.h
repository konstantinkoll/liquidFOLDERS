
#pragma once
#include "LF.h"

#ifdef LFCore_EXPORTS
#define LFCORE_API __declspec(dllexport)
#else
#define LFCORE_API __declspec(dllimport)
#endif


//
// Kernfunktionalit�t
//

#define LFGLV_Internal            1
#define LFGLV_External            2
#define LFGLV_Both                3
#define LFGLV_Network             4
#define LFGLV_IncludeFloppies     8
#define LFGLV_All                 15



// liquidFOLDERS initalisieren
LFCORE_API void __stdcall LFInitialize();



// Liefert einen String f�r Dateianzahl und -gr��e zur�ck
LFCORE_API void __stdcall LFCombineFileCountSize(unsigned int count, __int64 size, wchar_t* str, size_t cCount);



// Gibt true zur�ck, wenn diese Installation freigeschaltet ist
// Die gespeicherten Lizenzinformationen finden sich in License
LFCORE_API bool __stdcall LFIsLicensed(LFLicense* License=NULL, bool Reload=false);

// Gibt true zur�ck, wenn die Shareware-Version ausgelaufen ist,
// und keine ordnungsgem��e Lizenz vorliegt
LFCORE_API bool __stdcall LFIsSharewareExpired();



// Gibt true zur�ck, wenn der Explorer Dateiendungen verbirgt
LFCORE_API bool __stdcall LFHideFileExt();

// Gibt true zur�ck, wenn der Explorer leere Laufwerke verbirgt
LFCORE_API bool __stdcall LFHideVolumesWithNoMedia();



// Liefert den Source-Typ eines Laufwerks zur�ck
LFCORE_API unsigned int __stdcall LFGetSourceForVolume(char cVolume);

// Wie Win32-Funktion GetLogicalVolumes(), allerdings selektiv (s.o.)
LFCORE_API unsigned int __stdcall LFGetLogicalVolumes(unsigned int Mask=LFGLV_Both);

// Gibt einen Zeiger auf die IDs aller registrierten Nachrichten zur�ck
LFCORE_API LFMessageIDs* __stdcall LFGetMessageIDs();

// Erzeugt einen Link mit DropHandler zur Explorer-Erweiterung
// im SendTo-Ordner des Benutzers
// Wenn force==false wird der Link nur beim ersten Aufruf erzeugt
LFCORE_API void __stdcall LFCreateSendTo(bool force=false);

// Initalisiert eine LFProgress-Datenstruktur
LFCORE_API void __stdcall LFInitProgress(LFProgress* pProgress, HWND hWnd, unsigned int MajorCount=0);



// Informationen �ber ein Attribut zur�ckliefern
LFCORE_API void __stdcall LFGetAttributeInfo(LFAttributeDescriptor& attr, unsigned int ID);



// Informationen �ber ein Attribut zur�ckliefern
LFCORE_API void __stdcall LFGetContextInfo(LFContextDescriptor& ctx, unsigned int ID);



// Informationen �ber eine Kategorie zur�ckliefern
LFCORE_API void __stdcall LFGetItemCategoryInfo(LFItemCategoryDescriptor& cat, unsigned int ID);



// Neuen LFItemDescriptor erzeugen und zur�cksetzen
// Ggf. wird eine unabh�ngige Kopie von i erzeugt
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(LFItemDescriptor* i=NULL);

// Neuen LFItemDescriptor erzeugen und die Kern-Attribute belegen
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(LFCoreAttributes* attr);

// Neuen LFItemDescriptor erzeugen und den LFStoreDescriptor konvertieren
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(LFStoreDescriptor* s);

// Existierenden LFItemDescriptor freigeben
LFCORE_API void __stdcall LFFreeItemDescriptor(LFItemDescriptor* i);

// Attributwert holen
LFCORE_API void __stdcall LFGetAttributeVariantData(LFItemDescriptor* i, LFVariantData* v);

// Attributwert setzen
LFCORE_API void __stdcall LFSetAttributeVariantData(LFItemDescriptor* i, LFVariantData* v);



// Konvertiert einen FourCC in eine Zeichenkette
LFCORE_API void __stdcall LFFourCCToString(const unsigned int c, wchar_t* str, size_t cCount);

// Konvertiert eine 32-Bit-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFUINTToString(const unsigned int v, wchar_t* str, size_t cCount);

// Konvertiert eine 64-Bit-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFINT64ToString(const __int64 v, wchar_t* str, size_t cCount);

// Konvertiert einen Bruch in eine Zeichenkette
LFCORE_API void __stdcall LFFractionToString(const LFFraction frac, wchar_t* str, size_t cCount);

// Konvertiert eine Double-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFDoubleToString(const double d, wchar_t* str, size_t cCount);

// Konvertiert eine Koordinaten-Komponente in eine Zeichenkette
LFCORE_API void __stdcall LFGeoCoordinateToString(const double c, wchar_t* str, size_t cCount, bool IsLatitude, bool FillZero);

// Konvertiert eine Geo-Position in eine Zeichenkette
LFCORE_API void __stdcall LFGeoCoordinatesToString(const LFGeoCoordinates c, wchar_t* str, size_t cCount, bool FillZero);

// Konvertiert eine Zeit in eine Zeichenkette
LFCORE_API void __stdcall LFTimeToString(const FILETIME t, wchar_t* str, size_t cCount, unsigned int Mask=3);

// Konvertiert eine Zeitdauer in eine Zeichenkette
LFCORE_API void __stdcall LFDurationToString(unsigned int d, wchar_t* str, size_t cCount);

// Konvertiert eine Bitrate in eine Zeichenkette
LFCORE_API void __stdcall LFBitrateToString(const unsigned int r, wchar_t* str, size_t cCount);

// Konvertiert eine Megapixel-Angabe in eine Zeichenkette
LFCORE_API void __stdcall LFMegapixelToString(const double d, wchar_t* str, size_t cCount);

// Konvertiert ein Attribut in eine Zeichenkette
LFCORE_API void __stdcall LFAttributeToString(LFItemDescriptor* i, unsigned int attr, wchar_t* str, size_t cCount);

// Konvertiert eine LFVariantData-Struktur in eine Zeichenkette
LFCORE_API void __stdcall LFVariantDataToString(LFVariantData* v, wchar_t* str, size_t cCount);

// Erzeugt eine LFVariantData-Struktur aus einer Zeichenkette
LFCORE_API void __stdcall LFVariantDataFromString(LFVariantData* v, wchar_t* str);

// Erzeugt eine neutrale LFVariantData-Struktur (Null-Element)
// v->Attr muss gesetzt sein
LFCORE_API void __stdcall LFGetNullVariantData(LFVariantData* v);

// Pr�ft, ob eine LVVariantData-Struktur Null ist
LFCORE_API bool __stdcall LFIsNullVariantData(LFVariantData* v);

// Pr�ft, ob ein Dateiattribut gleich einer LFVariantData-Struktur ist
LFCORE_API bool __stdcall LFIsEqualToVariantData(LFItemDescriptor* i, LFVariantData* v);

// Vergleicht zwei Dateiattribute
LFCORE_API int __stdcall LFCompareVariantData(LFVariantData* v1, LFVariantData* v2);

// Entfernt doppelte Eint�ge in einem Unicode-Array
LFCORE_API void __stdcall LFSanitizeUnicodeArray(wchar_t* buf, size_t cCount);



// Neuen LFFilter erzeugen, ggf. als Kopie eines existierenden Filters
LFCORE_API LFFilter* __stdcall LFAllocFilter(LFFilter* f=NULL);

// Existierenden LFFilter freigeben
LFCORE_API void __stdcall LFFreeFilter(LFFilter* f);

// Neue LFFilterCondition erzeugen
LFCORE_API LFFilterCondition* __stdcall LFAllocFilterCondition();

// Existierende LFFilterCondition freigeben
LFCORE_API void __stdcall LFFreeFilterCondition(LFFilterCondition* c);



// Neues Suchergebnis mit Kontext ctx erzeugen
LFCORE_API LFSearchResult* __stdcall LFAllocSearchResult(int ctx, LFSearchResult* Result=NULL);

// Existierendes LFSearchResult freigeben
LFCORE_API void __stdcall LFFreeSearchResult(LFSearchResult* Result);

// LFItemDescriptor zum LFSearchResult hinzuf�gen
LFCORE_API bool __stdcall LFAddItemDescriptor(LFSearchResult* Result, LFItemDescriptor* i);

// LFItemDescriptor aus LFSearchResult entfernen
//
// !!ACHTUNG!!
// Die Sortierreihenfolge geht verloren!
LFCORE_API void __stdcall LFRemoveItemDescriptor(LFSearchResult* Result, unsigned int idx);

// Alle markierten LFItemDescriptor (DeleteFlag==true) aus LFSearchResult entfernen
//
// !!ACHTUNG!!
// Die Sortierreihenfolge geht verloren!
LFCORE_API void __stdcall LFRemoveFlaggedItemDescriptors(LFSearchResult* Result);

// Sortiert LFSearchResult
LFCORE_API void __stdcall LFSortSearchResult(LFSearchResult* Result, unsigned int attr, bool descending);

// Gruppiert LFSearchResult und liefert Kopie zur�ck
LFCORE_API LFSearchResult* __stdcall LFGroupSearchResult(LFSearchResult* Result, unsigned int attr, bool descending, bool groupone, LFFilter* f);



// Neue Dateiliste erzeugen
LFCORE_API LFFileIDList* __stdcall LFAllocFileIDList();
LFCORE_API LFFileIDList* __stdcall LFAllocFileIDList(HLIQUID hLiquid);

// Existierende LFFileIDList freigeben
LFCORE_API void __stdcall LFFreeFileIDList(LFFileIDList* il);

// String zur LFFileIDList hinzuf�gen
LFCORE_API bool __stdcall LFAddFileID(LFFileIDList* il, char* StoreID, char* FileID, void* UserData=NULL);

// Handle zu LIQUIDFILES-Struktur von Dateiliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateLiquidFiles(LFFileIDList* il);



// Neue Datei-Importliste erzeugen
LFCORE_API LFFileImportList* __stdcall LFAllocFileImportList();
LFCORE_API LFFileImportList* __stdcall LFAllocFileImportList(HDROP hDrop);

// Existierende LFFileImportList freigeben
LFCORE_API void __stdcall LFFreeFileImportList(LFFileImportList* il);

// String zur LFFileImportList hinzuf�gen
LFCORE_API bool __stdcall LFAddImportPath(LFFileImportList* il, wchar_t* path);



// Neue LFMaintenanceList erzeugen
LFCORE_API LFMaintenanceList* __stdcall LFAllocMaintenanceList();

// Existierende LFMaintenanceList freigeben
LFCORE_API void __stdcall LFFreeMaintenanceList(LFMaintenanceList* ml);



// Neue Transaktionsliste erzeugen
LFCORE_API LFTransactionList* __stdcall LFAllocTransactionList();

// Existierende LFTransactionList freigeben
LFCORE_API void __stdcall LFFreeTransactionList(LFTransactionList* tl);

// LFItemDescriptor zur LFTransactionList hinzuf�gen
LFCORE_API bool __stdcall LFAddItemDescriptor(LFTransactionList* tl, LFItemDescriptor* i, unsigned int UserData=0);

// PIDL von Transaktionsliste l�sen
LFCORE_API LPITEMIDLIST __stdcall LFDetachPIDL(LFTransactionList* tl, unsigned int idx);

// Handle zu DROPFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateDropFiles(LFTransactionList* tl);

// Handle zu LIQUIDFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateLiquidFiles(LFTransactionList* tl);



// Name einer Attribut-Kategorie in aktueller Sprache zur�ckliefern
LFCORE_API void __stdcall LFGetAttrCategoryName(wchar_t* pStr, unsigned int ID);

// Name einer Datenquelle in aktueller Sprache zur�ckliefern
LFCORE_API void __stdcall LFGetSourceName(wchar_t* pStr, unsigned int ID, bool qualified);

// Beschreibung eines Fehlers (LFError...) in aktueller Sprache zur�ckliefern
LFCORE_API void __stdcall LFGetErrorText(wchar_t* pStr, unsigned int ID);

// Anzeigen eines Fehlers (LFError...) in aktueller Sprache
LFCORE_API void __stdcall LFErrorBox(unsigned int ID, HWND hWnd=NULL);



// Suchabfrage durchf�hren
// - Ist f==NULL, so wird eine Liste aller Stores zur�ckgeliefert
LFCORE_API LFSearchResult* __stdcall LFQuery(LFFilter* f);

// Bestehendes Suchergebnis eingrenzen
// - f muss vom Typ LFFilterModeDirectoryTree oder LFFilterModeSearch sein
// - f muss ein Unterverzeichnis sein
// - first und last m�ssen einen g�ltigen Bereich umfassen
LFCORE_API LFSearchResult* __stdcall LFQuery(LFFilter* f, LFSearchResult* base, int first, int last);

// Statistik
// - Ist die StoreID leer, so wird die Statistik �ber alle Stores ermittelt
LFCORE_API LFStatistics* __stdcall LFQueryStatistics(char* StoreID);



//
// Stores
//

// Gibt den physischen Pfad einer Datei zur�ck
LFCORE_API unsigned int __stdcall LFGetFileLocation(LFItemDescriptor* i, wchar_t* dst, size_t cCount, bool CheckExists, bool RemoveNew, bool Extended=false);

// Gibt die Daten eines Stores zur�ck
LFCORE_API unsigned int __stdcall LFGetStoreSettings(char* StoreID, LFStoreDescriptor* s);
LFCORE_API unsigned int __stdcall LFGetStoreSettings(GUID guid, LFStoreDescriptor* s);

// Gibt die ID f�r das Icon eines Stores zur�ck
LFCORE_API unsigned int __stdcall LFGetStoreIcon(LFStoreDescriptor* s);

// Pr�ft, ob ein Store angeschlossen ist
LFCORE_API bool __stdcall LFIsStoreMounted(LFStoreDescriptor* s);

// Legt einen neuen Store an
// - Eingabeparameter interner Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - IndexMode: erforderlich
//   - AutoLocation: erforderlich
//   - DatPath: erforderlich, wenn AutoLocation==true
//   - Alle anderen Parameter werden ignoriert bzw. ausgef�llt
// - Eingabeparameter Hybrid-Store und externer Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - IndexMode: erforderlich
//   - DatPath: erforderlich
//   - Alle anderen Parameter werden ignoriert bzw. ausgef�llt
LFCORE_API unsigned int LFCreateStore(LFStoreDescriptor* s);

// Macht den internen Store zum Default Store
LFCORE_API unsigned int __stdcall LFMakeDefaultStore(char* StoreID);

// Macht den externen Store zum Hybrid-Store
LFCORE_API unsigned int __stdcall LFMakeStoreSearchable(char* StoreID, bool Searchable=true);

// Setzt Namen und Kommentar eines Stores
// Ist name oder comment NULL, so wird der jeweilige Wert nicht ver�ndert
LFCORE_API unsigned int __stdcall LFSetStoreAttributes(char* StoreID, wchar_t* name, wchar_t* comment, bool InternalCall=false);

// L�scht einen bestehenden Store
LFCORE_API unsigned int __stdcall LFDeleteStore(char* StoreID, LFProgress* pProgress=NULL);

// Startet geplante Wartungsarbeiten f�r alle Stores
LFCORE_API LFMaintenanceList* __stdcall LFStoreMaintenance(LFProgress* pProgress=NULL);

// Gibt an, ob ein Default Stores verf�gbar ist
LFCORE_API bool __stdcall LFDefaultStoreAvailable();

// Gibt die ID des aktuellen Default Stores zur�ck
LFCORE_API bool __stdcall LFGetDefaultStore(char* StoreID);

// Gibt den Standardnamen des Default Stores zur�ck
LFCORE_API void __stdcall LFGetDefaultStoreName(wchar_t* name, size_t cCount);

// Gibt die Anzahl aller Stores zur�ck
LFCORE_API unsigned int __stdcall LFGetStoreCount();

// Pr�ft, ob Stores auf dem angegebenen Laufwerk vorhanden sind
LFCORE_API bool __stdcall LFStoresOnVolume(char cVolume);

// Gibt die IDs aller Stores zur�ck
LFCORE_API unsigned int __stdcall LFGetStores(char** IDs, unsigned int* count);



//
// Filter
//

// Erzeugt eine neue Filterdatei in einem Store
LFCORE_API unsigned int __stdcall LFSaveFilter(char* key, LFFilter* f, wchar_t* name, wchar_t* comments=NULL, LFItemDescriptor** created=NULL);

// L�dt einen abgespeicherten Filter
LFCORE_API LFFilter* __stdcall LFLoadFilter(wchar_t* fn);
LFCORE_API LFFilter* __stdcall LFLoadFilter(LFItemDescriptor* i);



//
// Transaktionen
//

// Benennt die Datei um
LFCORE_API unsigned int __stdcall LFTransactionRename(char* StoreID, char* FileID, wchar_t* NewName);


// LFFileImportList
//

// Importiert Dateien in den Store
LFCORE_API void __stdcall LFTransactionImport(char* key, LFFileImportList* il, LFItemDescriptor* it, bool recursive, bool move, LFProgress* pProgress=NULL);


// LFTransactionList
//

// �ndert bei allen Eintr�gen in tl bis zu 3 Attributwerte
LFCORE_API void __stdcall LFTransactionUpdate(LFTransactionList* tl, LFVariantData* value1, LFVariantData* value2=NULL, LFVariantData* value3=NULL);

// Archiviert alle Dateien in tl
LFCORE_API void __stdcall LFTransactionArchive(LFTransactionList* tl);

// L�scht alle Dateien in tl
LFCORE_API void __stdcall LFTransactionDelete(LFTransactionList* tl, bool PutInTrash=true, LFProgress* pProgress=NULL);

// Stellt alle Dateien in tl wieder her, sofern nicht entg�ltig gel�scht
LFCORE_API void __stdcall LFTransactionRestore(LFTransactionList* tl, unsigned int Flags);

// Physische Orte aufl�sen
LFCORE_API void __stdcall LFTransactionResolvePhysicalLocations(LFTransactionList* tl, bool IncludePIDL=false);


// LFFileIDList
//

// Importiert Dateien in den Store
LFCORE_API void __stdcall LFTransactionImport(char* key, LFFileIDList* il, bool move, LFProgress* pProgress=NULL);

// L�scht alle Dateien in il
LFCORE_API void __stdcall LFTransactionDelete(LFFileIDList* il, bool PutInTrash=true, LFProgress* pProgress=NULL);

// F�gt die angegebenen Dateien zum Suchergebnis hinzu
LFCORE_API void __stdcall LFTransactionAddToSearchResult(LFFileIDList* il, LFSearchResult* sr);


//
// Geotagging
//

// Liefert die Anzahl der Territorien zur�ck
LFCORE_API unsigned int __stdcall LFIATAGetCountryCount();

// Liefert die Anzahl der Flugh�fen zur�ck
LFCORE_API unsigned int __stdcall LFIATAGetAirportCount();

// Liefert Zeiger auf Territorium zur�ck
LFCORE_API LFCountry* __stdcall LFIATAGetCountry(unsigned int ID);

// Setzt den Zeiger *pBuffer auf den n�chsten Flughafen
LFCORE_API int __stdcall LFIATAGetNextAirport(int last, LFAirport** pBuffer);

// Setzt den Zeiger *pBuffer auf den n�chsten Flughafen, der im Territorium CountryID liegt.
// *pBuffer kann in jedem Fall �berschrieben werden.
LFCORE_API int __stdcall LFIATAGetNextAirportByCountry(unsigned int CountryID, int last, LFAirport** pBuffer);

// Setzt den Zeiger *pBuffer auf den Flughafen mit dem �bergebenen Code.
// *pBuffer kann in jedem Fall �berschrieben werden.
LFCORE_API bool __stdcall LFIATAGetAirportByCode(char* Code, LFAirport** pBuffer);


//
// Shortcuts
//

// Fragt unter Windows XP, ob eine Verkn�pfung auf dem Desktop erstellt werden soll
// Bei h�heren Windows-Versionen wird immer true zur�ckgeliefert.
LFCORE_API bool __stdcall LFAskCreateShortcut(HWND hwnd);

// Speichert pShellLink auf dem Desktop ab
LFCORE_API void __stdcall LFCreateDesktopShortcut(IShellLink* pShellLink, wchar_t* LinkFilename);

// Liefert einen ShellLink f�r den angegebenen Store
LFCORE_API IShellLink* __stdcall LFGetShortcutForStore(LFItemDescriptor* i);
LFCORE_API IShellLink* __stdcall LFGetShortcutForStore(LFStoreDescriptor* s);
LFCORE_API IShellLink* __stdcall LFGetShortcutForStore(char* key);

// Erzeugt auf dem Desktop eine Verkn�pfung mit dem angegebenen Store
LFCORE_API void __stdcall LFCreateDesktopShortcutForStore(LFItemDescriptor* i);
LFCORE_API void __stdcall LFCreateDesktopShortcutForStore(LFStoreDescriptor* s);
LFCORE_API void __stdcall LFCreateDesktopShortcutForStore(char* key);


//
// Thumbnails
//

LFCORE_API HBITMAP __stdcall LFGetThumbnail(LFItemDescriptor* i);
LFCORE_API HBITMAP __stdcall LFGetThumbnail(LFItemDescriptor* i, SIZE sz);

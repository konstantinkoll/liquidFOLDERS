
#pragma once
#include "LF.h"

#ifdef LFCore_EXPORTS
#define LFCORE_API __declspec(dllexport)
#else
#define LFCORE_API __declspec(dllimport)
#endif


//
// Kernfunktionalität
//

#define LFGLV_Internal            1
#define LFGLV_External            2
#define LFGLV_Both                3
#define LFGLV_Network             4
#define LFGLV_IncludeFloppies     8
#define LFGLV_All                 15



// liquidFOLDERS initalisieren
LFCORE_API void __stdcall LFInitialize();



// Liefert einen String mit Dateianzahl und -größe zurück
LFCORE_API void __stdcall LFCombineFileCountSize(UINT Count, INT64 Size, WCHAR* pStr, size_t cCount);



// Gibt TRUE zurück, wenn der Explorer Dateiendungen verbirgt
LFCORE_API BOOL __stdcall LFHideFileExt();

// Gibt TRUE zurück, wenn der Explorer leere Laufwerke verbirgt
LFCORE_API BOOL __stdcall LFHideVolumesWithNoMedia();



// Gibt TRUE zurück, wenn diese Installation freigeschaltet ist
// Die gespeicherten Lizenzinformationen finden sich in License
LFCORE_API BOOL __stdcall LFIsLicensed(LFLicense* License=NULL, BOOL Reload=FALSE);

// Gibt TRUE zurück, wenn die Shareware-Version ausgelaufen ist,
// und keine ordnungsgemäße Lizenz vorliegt
LFCORE_API BOOL __stdcall LFIsSharewareExpired();



// Liefert den Source-Typ eines Laufwerks zurück
LFCORE_API UINT __stdcall LFGetSourceForVolume(CHAR cVolume);

// Wie Win32-Funktion GetLogicalVolumes(), allerdings selektiv (s.o.)
LFCORE_API UINT __stdcall LFGetLogicalVolumes(UINT Mask=LFGLV_Both);

// Gibt einen Zeiger auf die IDs aller registrierten Nachrichten zurück
LFCORE_API LFMessageIDs* __stdcall LFGetMessageIDs();

// Erzeugt einen Link mit DropHandler zur Explorer-Erweiterung
// im SendTo-Ordner des Benutzers
// Wenn force==FALSE wird der Link nur beim ersten Aufruf erzeugt
LFCORE_API void __stdcall LFCreateSendTo(BOOL force=FALSE);

// Initalisiert eine LFProgress-Datenstruktur
LFCORE_API void __stdcall LFInitProgress(LFProgress* pProgress, HWND hWnd, UINT MajorCount=0);



// Informationen über ein Attribut zurückliefern
LFCORE_API void __stdcall LFGetAttributeInfo(LFAttributeDescriptor& Attr, UINT ID);



// Informationen über ein Attribut zurückliefern
LFCORE_API void __stdcall LFGetContextInfo(LFContextDescriptor& ctx, UINT ID);



// Informationen über eine Kategorie zurückliefern
LFCORE_API void __stdcall LFGetItemCategoryInfo(LFItemCategoryDescriptor& cat, UINT ID);



// Neuen LFItemDescriptor erzeugen und zurücksetzen
// Ggf. wird eine unabhängige Kopie von i erzeugt
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(LFItemDescriptor* i=NULL);

// Neuen LFItemDescriptor erzeugen und die Kern-Attribute belegen
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(LFCoreAttributes* Attr);

// Neuen LFItemDescriptor erzeugen und den LFStoreDescriptor konvertieren
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(LFStoreDescriptor* s);

// Existierenden LFItemDescriptor freigeben
LFCORE_API void __stdcall LFFreeItemDescriptor(LFItemDescriptor* i);



// Konvertiert einen FourCC in eine Zeichenkette
LFCORE_API void __stdcall LFFourCCToString(const UINT c, WCHAR* pStr, size_t cCount);

// Konvertiert eine 32-Bit-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFUINTToString(const UINT u, WCHAR* pStr, size_t cCount);

// Konvertiert eine 64-Bit-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFSizeToString(const INT64 i, WCHAR* pStr, size_t cCount);

// Konvertiert einen Bruch in eine Zeichenkette
LFCORE_API void __stdcall LFFractionToString(const LFFraction frac, WCHAR* pStr, size_t cCount);

// Konvertiert eine Double-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFDoubleToString(const DOUBLE d, WCHAR* pStr, size_t cCount);

// Konvertiert eine Koordinaten-Komponente in eine Zeichenkette
LFCORE_API void __stdcall LFGeoCoordinateToString(const DOUBLE c, WCHAR* pStr, size_t cCount, BOOL IsLatitude, BOOL FillZero);

// Konvertiert eine Geo-Position in eine Zeichenkette
LFCORE_API void __stdcall LFGeoCoordinatesToString(const LFGeoCoordinates c, WCHAR* pStr, size_t cCount, BOOL FillZero);

// Konvertiert eine Zeit in eine Zeichenkette
LFCORE_API void __stdcall LFTimeToString(const FILETIME t, WCHAR* pStr, size_t cCount, BOOL IncludeTime=TRUE);

// Konvertiert eine Zeitdauer in eine Zeichenkette
LFCORE_API void __stdcall LFDurationToString(UINT d, WCHAR* pStr, size_t cCount);

// Konvertiert eine Bitrate in eine Zeichenkette
LFCORE_API void __stdcall LFBitrateToString(const UINT r, WCHAR* pStr, size_t cCount);

// Konvertiert eine Megapixel-Angabe in eine Zeichenkette
LFCORE_API void __stdcall LFMegapixelToString(const DOUBLE d, WCHAR* pStr, size_t cCount);

// Konvertiert ein Attribut in eine Zeichenkette
LFCORE_API void __stdcall LFAttributeToString(LFItemDescriptor* i, UINT Attr, WCHAR* pStr, size_t cCount);

// Initalisiert eine LFVariantData-Struktur
LFCORE_API void __stdcall LFInitVariantData(LFVariantData& v, UINT Attr);

// Löscht eine LFVariantData-Struktur
// v->Attr muss gesetzt sein
LFCORE_API void __stdcall LFClearVariantData(LFVariantData& v);

// Prüft, ob eine LVVariantData-Struktur Null ist
LFCORE_API BOOL __stdcall LFIsNullVariantData(LFVariantData& v);

// Konvertiert eine LFVariantData-Struktur in eine Zeichenkette
LFCORE_API void __stdcall LFVariantDataToString(LFVariantData& v, WCHAR* pStr, size_t cCount);

// Erzeugt eine LFVariantData-Struktur aus einer Zeichenkette
LFCORE_API void __stdcall LFVariantDataFromString(LFVariantData& v, WCHAR* pStr);

// Vergleicht zwei Dateiattribute
LFCORE_API INT __stdcall LFCompareVariantData(LFVariantData& v1, LFVariantData& v2);

// Attributwert holen
LFCORE_API void __stdcall LFGetAttributeVariantData(LFItemDescriptor* i, LFVariantData& v);
LFCORE_API void __stdcall LFGetAttributeVariantDataEx(LFItemDescriptor* i, UINT Attr, LFVariantData& v);

// Attributwert setzen
LFCORE_API void __stdcall LFSetAttributeVariantData(LFItemDescriptor* i, LFVariantData& v);

// Entfernt doppelte Eintäge in einem Unicode-Array
LFCORE_API void __stdcall LFSanitizeUnicodeArray(WCHAR* pBuffer, size_t cCount);



// Neuen LFFilter erzeugen, ggf. als Kopie eines existierenden Filters
LFCORE_API LFFilter* __stdcall LFAllocFilter(LFFilter* f=NULL);

// Existierenden LFFilter freigeben
LFCORE_API void __stdcall LFFreeFilter(LFFilter* f);

// Neue LFFilterCondition erzeugen
LFCORE_API LFFilterCondition* __stdcall LFAllocFilterCondition();

// Existierende LFFilterCondition freigeben
LFCORE_API void __stdcall LFFreeFilterCondition(LFFilterCondition* c);



// Neues Suchergebnis mit Kontext ctx erzeugen
LFCORE_API LFSearchResult* __stdcall LFAllocSearchResult(INT ctx, LFSearchResult* Result=NULL);

// Existierendes LFSearchResult freigeben
LFCORE_API void __stdcall LFFreeSearchResult(LFSearchResult* Result);

// LFItemDescriptor zum LFSearchResult hinzufügen
LFCORE_API BOOL __stdcall LFAddItemDescriptor(LFSearchResult* Result, LFItemDescriptor* i);

// LFItemDescriptor aus LFSearchResult entfernen
//
// !!ACHTUNG!!
// Die Sortierreihenfolge geht verloren!
LFCORE_API void __stdcall LFRemoveItemDescriptor(LFSearchResult* Result, UINT idx);

// Alle markierten LFItemDescriptor (DeleteFlag==TRUE) aus LFSearchResult entfernen
//
// !!ACHTUNG!!
// Die Sortierreihenfolge geht verloren!
LFCORE_API void __stdcall LFRemoveFlaggedItemDescriptors(LFSearchResult* Result);

// Sortiert LFSearchResult
LFCORE_API void __stdcall LFSortSearchResult(LFSearchResult* Result, UINT Attr, BOOL descending);

// Gruppiert LFSearchResult und liefert Kopie zurück
LFCORE_API LFSearchResult* __stdcall LFGroupSearchResult(LFSearchResult* Result, UINT Attr, BOOL descending, BOOL groupone, LFFilter* f);



// Neue Dateiliste erzeugen
LFCORE_API LFFileIDList* __stdcall LFAllocFileIDList();
LFCORE_API LFFileIDList* __stdcall LFAllocFileIDList(HLIQUID hLiquid);

// Existierende LFFileIDList freigeben
LFCORE_API void __stdcall LFFreeFileIDList(LFFileIDList* il);

// String zur LFFileIDList hinzufügen
LFCORE_API BOOL __stdcall LFAddFileID(LFFileIDList* il, CHAR* StoreID, CHAR* FileID, void* UserData=NULL);

// Handle zu LIQUIDFILES-Struktur von Dateiliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateLiquidFiles(LFFileIDList* il);



// Neue Datei-Importliste erzeugen
LFCORE_API LFFileImportList* __stdcall LFAllocFileImportList();
LFCORE_API LFFileImportList* __stdcall LFAllocFileImportList(HDROP hDrop);

// Existierende LFFileImportList freigeben
LFCORE_API void __stdcall LFFreeFileImportList(LFFileImportList* il);

// String zur LFFileImportList hinzufügen
LFCORE_API BOOL __stdcall LFAddImportPath(LFFileImportList* il, WCHAR* path);



// Neue LFMaintenanceList erzeugen
LFCORE_API LFMaintenanceList* __stdcall LFAllocMaintenanceList();

// Existierende LFMaintenanceList freigeben
LFCORE_API void __stdcall LFFreeMaintenanceList(LFMaintenanceList* ml);



// Neue Transaktionsliste erzeugen
LFCORE_API LFTransactionList* __stdcall LFAllocTransactionList();

// Existierende LFTransactionList freigeben
LFCORE_API void __stdcall LFFreeTransactionList(LFTransactionList* tl);

// LFItemDescriptor zur LFTransactionList hinzufügen
LFCORE_API BOOL __stdcall LFAddItemDescriptor(LFTransactionList* tl, LFItemDescriptor* i, UINT UserData=0);

// PIDL von Transaktionsliste lösen
LFCORE_API LPITEMIDLIST __stdcall LFDetachPIDL(LFTransactionList* tl, UINT idx);

// Handle zu DROPFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateDropFiles(LFTransactionList* tl);

// Handle zu LIQUIDFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateLiquidFiles(LFTransactionList* tl);



// Name einer Attribut-Kategorie in aktueller Sprache zurückliefern
LFCORE_API void __stdcall LFGetAttrCategoryName(WCHAR* pStr, UINT ID);

// Name einer Datenquelle in aktueller Sprache zurückliefern
LFCORE_API void __stdcall LFGetSourceName(WCHAR* pStr, UINT ID, BOOL qualified);

// Beschreibung eines Fehlers (LFError...) in aktueller Sprache zurückliefern
LFCORE_API void __stdcall LFGetErrorText(WCHAR* pStr, UINT ID);

// Anzeigen eines Fehlers (LFError...) in aktueller Sprache
LFCORE_API void __stdcall LFErrorBox(UINT ID, HWND hWnd=NULL);



// Suchabfrage durchführen
// - Ist f==NULL, so wird eine Liste aller Stores zurückgeliefert
LFCORE_API LFSearchResult* __stdcall LFQuery(LFFilter* f);

// Bestehendes Suchergebnis eingrenzen
// - f muss vom Typ LFFilterModeDirectoryTree oder LFFilterModeSearch sein
// - f muss ein Unterverzeichnis sein
// - first und last müssen einen gültigen Bereich umfassen
LFCORE_API LFSearchResult* __stdcall LFQuery(LFFilter* f, LFSearchResult* base, INT first, INT last);

// Statistik
// - Ist die StoreID leer, so wird die Statistik über alle Stores ermittelt
LFCORE_API LFStatistics* __stdcall LFQueryStatistics(CHAR* StoreID);



//
// Stores
//

// Gibt den physischen Pfad einer Datei zurück
LFCORE_API UINT __stdcall LFGetFileLocation(LFItemDescriptor* i, WCHAR* dst, size_t cCount, BOOL CheckExists, BOOL RemoveNew, BOOL Extended=FALSE);

// Gibt die Daten eines Stores zurück
LFCORE_API UINT __stdcall LFGetStoreSettings(CHAR* StoreID, LFStoreDescriptor* s);
LFCORE_API UINT __stdcall LFGetStoreSettings(GUID guid, LFStoreDescriptor* s);

// Gibt die ID für das Icon eines Stores zurück
LFCORE_API UINT __stdcall LFGetStoreIcon(LFStoreDescriptor* s);

// Prüft, ob ein Store angeschlossen ist
LFCORE_API BOOL __stdcall LFIsStoreMounted(LFStoreDescriptor* s);

// Legt einen neuen Store an
// - Eingabeparameter interner Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - IndexMode: erforderlich
//   - AutoLocation: erforderlich
//   - DatPath: erforderlich, wenn AutoLocation==TRUE
//   - Alle anderen Parameter werden ignoriert bzw. ausgefüllt
// - Eingabeparameter Hybrid-Store und externer Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - IndexMode: erforderlich
//   - DatPath: erforderlich
//   - Alle anderen Parameter werden ignoriert bzw. ausgefüllt
LFCORE_API UINT LFCreateStore(LFStoreDescriptor* s);

// Macht den internen Store zum Default Store
LFCORE_API UINT __stdcall LFMakeDefaultStore(CHAR* StoreID);

// Macht den externen Store zum Hybrid-Store
LFCORE_API UINT __stdcall LFMakeStoreSearchable(CHAR* StoreID, BOOL Searchable=TRUE);

// Setzt Namen und Kommentar eines Stores
// Ist name oder comment NULL, so wird der jeweilige Wert nicht verändert
LFCORE_API UINT __stdcall LFSetStoreAttributes(CHAR* StoreID, WCHAR* name, WCHAR* comment, BOOL InternalCall=FALSE);

// Löscht einen bestehenden Store
LFCORE_API UINT __stdcall LFDeleteStore(CHAR* StoreID, LFProgress* pProgress=NULL);

// Startet geplante Wartungsarbeiten für alle Stores
LFCORE_API LFMaintenanceList* __stdcall LFStoreMaintenance(LFProgress* pProgress=NULL);

// Gibt an, ob ein Default Stores verfügbar ist
LFCORE_API BOOL __stdcall LFDefaultStoreAvailable();

// Gibt die ID des aktuellen Default Stores zurück
LFCORE_API BOOL __stdcall LFGetDefaultStore(CHAR* StoreID);

// Gibt den Standardnamen des Default Stores zurück
LFCORE_API void __stdcall LFGetDefaultStoreName(WCHAR* name, size_t cCount);

// Gibt die Anzahl aller Stores zurück
LFCORE_API UINT __stdcall LFGetStoreCount();

// Prüft, ob Stores auf dem angegebenen Laufwerk vorhanden sind
LFCORE_API BOOL __stdcall LFStoresOnVolume(CHAR cVolume);

// Gibt die IDs aller Stores zurück
LFCORE_API UINT __stdcall LFGetStores(CHAR** IDs, UINT* count);



//
// Filter
//

// Erzeugt eine neue Filterdatei in einem Store
LFCORE_API UINT __stdcall LFSaveFilter(CHAR* key, LFFilter* f, WCHAR* name, WCHAR* comments=NULL, LFItemDescriptor** created=NULL);

// Lädt einen abgespeicherten Filter
LFCORE_API LFFilter* __stdcall LFLoadFilter(WCHAR* fn);
LFCORE_API LFFilter* __stdcall LFLoadFilter(LFItemDescriptor* i);



//
// Transaktionen
//

// Benennt die Datei um
LFCORE_API UINT __stdcall LFTransactionRename(CHAR* StoreID, CHAR* FileID, WCHAR* NewName);


// LFFileImportList
//

// Importiert Dateien in den Store
LFCORE_API void __stdcall LFTransactionImport(CHAR* key, LFFileImportList* il, LFItemDescriptor* it, BOOL recursive, BOOL move, LFProgress* pProgress=NULL);


// LFTransactionList
//

// Ändert bei allen Einträgen in tl bis zu 3 Attributwerte
LFCORE_API void __stdcall LFTransactionUpdate(LFTransactionList* tl, LFVariantData* v1, LFVariantData* v2=NULL, LFVariantData* v3=NULL);

// Archiviert alle Dateien in tl
LFCORE_API void __stdcall LFTransactionArchive(LFTransactionList* tl);

// Löscht alle Dateien in tl
LFCORE_API void __stdcall LFTransactionDelete(LFTransactionList* tl, BOOL PutInTrash=TRUE, LFProgress* pProgress=NULL);

// Stellt alle Dateien in tl wieder her, sofern nicht entgültig gelöscht
LFCORE_API void __stdcall LFTransactionRestore(LFTransactionList* tl, UINT Flags);

// Physische Orte auflösen
LFCORE_API void __stdcall LFTransactionResolvePhysicalLocations(LFTransactionList* tl, BOOL IncludePIDL=FALSE);


// LFFileIDList
//

// Importiert Dateien in den Store
LFCORE_API void __stdcall LFTransactionImport(CHAR* key, LFFileIDList* il, BOOL move, LFProgress* pProgress=NULL);

// Löscht alle Dateien in il
LFCORE_API void __stdcall LFTransactionDelete(LFFileIDList* il, BOOL PutInTrash=TRUE, LFProgress* pProgress=NULL);

// Fügt die angegebenen Dateien zum Suchergebnis hinzu
LFCORE_API void __stdcall LFTransactionAddToSearchResult(LFFileIDList* il, LFSearchResult* sr);


//
// Geotagging
//

// Liefert die Anzahl der Territorien zurück
LFCORE_API UINT __stdcall LFIATAGetCountryCount();

// Liefert die Anzahl der Flughäfen zurück
LFCORE_API UINT __stdcall LFIATAGetAirportCount();

// Liefert Zeiger auf Territorium zurück
LFCORE_API LFCountry* __stdcall LFIATAGetCountry(UINT ID);

// Setzt den Zeiger *pBuffer auf den nächsten Flughafen
LFCORE_API INT __stdcall LFIATAGetNextAirport(INT last, LFAirport** pBuffer);

// Setzt den Zeiger *pBuffer auf den nächsten Flughafen, der im Territorium CountryID liegt.
// *pBuffer kann in jedem Fall überschrieben werden.
LFCORE_API INT __stdcall LFIATAGetNextAirportByCountry(UINT CountryID, INT last, LFAirport** pBuffer);

// Setzt den Zeiger *pBuffer auf den Flughafen mit dem übergebenen Code.
// *pBuffer kann in jedem Fall überschrieben werden.
LFCORE_API BOOL __stdcall LFIATAGetAirportByCode(CHAR* Code, LFAirport** pBuffer);


//
// Shortcuts
//

// Fragt unter Windows XP, ob eine Verknüpfung auf dem Desktop erstellt werden soll
// Bei höheren Windows-Versionen wird immer TRUE zurückgeliefert.
LFCORE_API BOOL __stdcall LFAskCreateShortcut(HWND hwnd);

// Speichert pShellLink auf dem Desktop ab
LFCORE_API void __stdcall LFCreateDesktopShortcut(IShellLink* pShellLink, WCHAR* LinkFilename);

// Liefert einen ShellLink für den angegebenen Store
LFCORE_API IShellLink* __stdcall LFGetShortcutForStore(LFItemDescriptor* i);
LFCORE_API IShellLink* __stdcall LFGetShortcutForStore(LFStoreDescriptor* s);
LFCORE_API IShellLink* __stdcall LFGetShortcutForStore(CHAR* key);

// Erzeugt auf dem Desktop eine Verknüpfung mit dem angegebenen Store
LFCORE_API void __stdcall LFCreateDesktopShortcutForStore(LFItemDescriptor* i);
LFCORE_API void __stdcall LFCreateDesktopShortcutForStore(LFStoreDescriptor* s);
LFCORE_API void __stdcall LFCreateDesktopShortcutForStore(CHAR* key);


//
// Thumbnails
//

LFCORE_API HBITMAP __stdcall LFGetThumbnail(LFItemDescriptor* i);
LFCORE_API HBITMAP __stdcall LFGetThumbnail(LFItemDescriptor* i, SIZE sz);

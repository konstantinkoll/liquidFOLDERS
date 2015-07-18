
#pragma once
#include "LF.h"
#include "LFFileImportList.h"
#include "LFMaintenanceList.h"
#include "LFSearchResult.h"
#include "LFTransactionList.h"

#ifdef LFCore_EXPORTS
#define LFCORE_API __declspec(dllexport)
#else
#define LFCORE_API __declspec(dllimport)
#endif


// Kernfunktionalit�t
//

#define LFGLV_INTERNAL            1
#define LFGLV_EXTERNAL            2
#define LFGLV_BOTH                3
#define LFGLV_NETWORK             4
#define LFGLV_INCLUDEFLOPPIES     8
#define LFGLV_ALL                 15


// liquidFOLDERS initalisieren
LFCORE_API void __stdcall LFInitialize();

// Gibt einen Zeiger auf die IDs aller registrierten Nachrichten zur�ck
LFCORE_API LFMessageIDs* __stdcall LFGetMessageIDs();

// Gibt den Dateinamen der liquidFOLDERS-App zur�ck
LFCORE_API BOOL __stdcall LFGetApplicationPath(WCHAR* pStr, SIZE_T cCount);

// Liefert einen String mit Dateianzahl und -gr��e zur�ck
LFCORE_API void __stdcall LFCombineFileCountSize(UINT Count, INT64 Size, WCHAR* pStr, SIZE_T cCount);


// Gibt TRUE zur�ck, wenn der Explorer Dateiendungen verbirgt
LFCORE_API BOOL __stdcall LFHideFileExt();

// Gibt TRUE zur�ck, wenn der Explorer leere Laufwerke verbirgt
LFCORE_API BOOL __stdcall LFHideVolumesWithNoMedia();


// Liefert den Source-Typ eines Laufwerks zur�ck
LFCORE_API UINT __stdcall LFGetSourceForVolume(CHAR cVolume);

// Wie Win32-Funktion GetLogicalVolumes(), allerdings selektiv (s.o.)
LFCORE_API UINT __stdcall LFGetLogicalVolumes(UINT Mask=LFGLV_BOTH);


// Initalisiert eine LFProgress-Datenstruktur
LFCORE_API void __stdcall LFInitProgress(LFProgress* pProgress, HWND hWnd, UINT MajorCount=0);


// Beschreibung eines Fehlers (LFError...) in aktueller Sprache zur�ckliefern
LFCORE_API void __stdcall LFGetErrorText(WCHAR* pStr, SIZE_T cCount, UINT ID);

// Anzeigen eines Fehlers (LFError...) in aktueller Sprache
LFCORE_API void __stdcall LFErrorBox(UINT nID, HWND hWnd=NULL);


// Gibt TRUE zur�ck, wenn diese Installation freigeschaltet ist
// Die gespeicherten Lizenzinformationen finden sich in License
LFCORE_API BOOL __stdcall LFIsLicensed(LFLicense* pLicense=NULL, BOOL Reload=FALSE);

// Gibt TRUE zur�ck, wenn die Shareware-Version ausgelaufen ist,
// und keine ordnungsgem��e Lizenz vorliegt
LFCORE_API BOOL __stdcall LFIsSharewareExpired();


// Name einer Attribut-Kategorie in aktueller Sprache zur�ckliefern
LFCORE_API void __stdcall LFGetAttrCategoryName(WCHAR* pStr, UINT ID);

// Informationen �ber ein Attribut zur�ckliefern
LFCORE_API void __stdcall LFGetAttributeInfo(LFAttributeDescriptor& AttributeDescriptor, UINT ID);

// Name einer Datenquelle in aktueller Sprache zur�ckliefern
LFCORE_API void __stdcall LFGetSourceName(WCHAR* pStr, UINT ID, BOOL Qualified);

// Informationen �ber eine Kategorie zur�ckliefern
LFCORE_API void __stdcall LFGetItemCategoryInfo(LFItemCategoryDescriptor& ItemCategoryDescriptor, UINT ID);

// Informationen �ber ein Attribut zur�ckliefern
LFCORE_API void __stdcall LFGetContextInfo(LFContextDescriptor& ContextDescriptor, UINT ID);



// LFVariantData
//

// Konvertiert einen FourCC in eine Zeichenkette
LFCORE_API void __stdcall LFFourCCToString(const UINT c, WCHAR* pStr, SIZE_T cCount);

// Konvertiert eine 32-Bit-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFUINTToString(const UINT u, WCHAR* pStr, SIZE_T cCount);

// Konvertiert eine 64-Bit-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFSizeToString(const INT64 i, WCHAR* pStr, SIZE_T cCount);

// Konvertiert einen Bruch in eine Zeichenkette
LFCORE_API void __stdcall LFFractionToString(const LFFraction f, WCHAR* pStr, SIZE_T cCount);

// Konvertiert eine Double-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFDoubleToString(const DOUBLE d, WCHAR* pStr, SIZE_T cCount);

// Konvertiert eine Koordinaten-Komponente in eine Zeichenkette
LFCORE_API void __stdcall LFGeoCoordinateToString(const DOUBLE c, WCHAR* pStr, SIZE_T cCount, BOOL IsLatitude, BOOL FillZero);

// Konvertiert eine Geo-Position in eine Zeichenkette
LFCORE_API void __stdcall LFGeoCoordinatesToString(const LFGeoCoordinates c, WCHAR* pStr, SIZE_T cCount, BOOL FillZero);

// Konvertiert eine Zeit in eine Zeichenkette
LFCORE_API void __stdcall LFTimeToString(const FILETIME t, WCHAR* pStr, SIZE_T cCount, BOOL IncludeTime=TRUE);

// Konvertiert eine Zeitdauer in eine Zeichenkette
LFCORE_API void __stdcall LFDurationToString(UINT d, WCHAR* pStr, SIZE_T cCount);

// Konvertiert eine Bitrate in eine Zeichenkette
LFCORE_API void __stdcall LFBitrateToString(const UINT r, WCHAR* pStr, SIZE_T cCount);

// Konvertiert eine Megapixel-Angabe in eine Zeichenkette
LFCORE_API void __stdcall LFMegapixelToString(const DOUBLE d, WCHAR* pStr, SIZE_T cCount);

// Konvertiert ein Attribut in eine Zeichenkette
LFCORE_API void __stdcall LFAttributeToString(LFItemDescriptor* pItemDescriptor, UINT Attr, WCHAR* pStr, SIZE_T cCount);

// Initalisiert eine LFVariantData-Struktur
LFCORE_API void __stdcall LFInitVariantData(LFVariantData& v, UINT Attr);

// L�scht eine LFVariantData-Struktur
// v->Attr muss gesetzt sein
LFCORE_API void __stdcall LFClearVariantData(LFVariantData& v);

// Pr�ft, ob eine LVVariantData-Struktur Null ist
LFCORE_API BOOL __stdcall LFIsNullVariantData(LFVariantData& v);

// Konvertiert eine LFVariantData-Struktur in eine Zeichenkette
LFCORE_API void __stdcall LFVariantDataToString(LFVariantData& v, WCHAR* pStr, SIZE_T cCount);

// Erzeugt eine LFVariantData-Struktur aus einer Zeichenkette
LFCORE_API void __stdcall LFVariantDataFromString(LFVariantData& v, WCHAR* pStr);

// Vergleicht zwei Dateiattribute
LFCORE_API INT __stdcall LFCompareVariantData(LFVariantData& v1, LFVariantData& v2);

// Attributwert holen
LFCORE_API void __stdcall LFGetAttributeVariantData(LFItemDescriptor* pItemDescriptor, LFVariantData& v);
LFCORE_API void __stdcall LFGetAttributeVariantDataEx(LFItemDescriptor* pItemDescriptor, UINT Attr, LFVariantData& v);

// Attributwert setzen
LFCORE_API void __stdcall LFSetAttributeVariantData(LFItemDescriptor* pItemDescriptor, LFVariantData& v);

// Entfernt doppelte Eint�ge in einem Unicode-Array
LFCORE_API void __stdcall LFSanitizeUnicodeArray(WCHAR* pBuffer, SIZE_T cCount);



// Datenstrukturen
//

// Neuen LFItemDescriptor erzeugen und ggf. die Kern-Attribute belegen
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(LFCoreAttributes* pCoreAttributes=NULL);

// Neuen LFItemDescriptor f�r Store erzeugen
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptorEx(LFStoreDescriptor* pStoreDescriptor);

// Unabh�ngige Kopie von pItemDescriptor erzeugen
LFCORE_API LFItemDescriptor* __stdcall LFCloneItemDescriptor(LFItemDescriptor* pItemDescriptor);

// Existierenden LFItemDescriptor freigeben
LFCORE_API void __stdcall LFFreeItemDescriptor(LFItemDescriptor* pItemDescriptor);


// Neuen LFFilter erzeugen, ggf. als Kopie eines existierenden Filters
LFCORE_API LFFilter* __stdcall LFAllocFilter(LFFilter* pFilter=NULL);

// Existierenden LFFilter freigeben
LFCORE_API void __stdcall LFFreeFilter(LFFilter* pFilter);

// L�dt einen abgespeicherten Filter
LFCORE_API LFFilter* __stdcall LFLoadFilter(LFItemDescriptor* pItemDescriptor);
LFCORE_API LFFilter* __stdcall LFLoadFilterEx(WCHAR* pFilename);

// Speichert einen Filter in einem Store ab
LFCORE_API UINT __stdcall LFSaveFilter(CHAR* StoreID, LFFilter* pFilter, WCHAR* pName, WCHAR* pComments=NULL);

// Neue LFFilterCondition erzeugen
LFCORE_API LFFilterCondition* __stdcall LFAllocFilterCondition(BYTE Compare, LFVariantData& v, LFFilterCondition* pNext=NULL);
LFCORE_API LFFilterCondition* __stdcall LFAllocFilterConditionEx(BYTE Compare, UINT Attr, LFFilterCondition* pNext=NULL);

// Existierende LFFilterCondition freigeben
#define LFFreeFilterCondition(pFilterCondition) delete pFilterCondition;


// Neues Suchergebnis mit Kontext Context erzeugen
LFCORE_API LFSearchResult* __stdcall LFAllocSearchResult(INT Context);

// Existierendes LFSearchResult freigeben
LFCORE_API void __stdcall LFFreeSearchResult(LFSearchResult* pSearchResult);

// LFItemDescriptor zum LFSearchResult hinzuf�gen
LFCORE_API BOOL __stdcall LFAddItem(LFSearchResult* pSearchResult, LFItemDescriptor* pItemDescriptor);

// Alle markierten LFItemDescriptor (RemoveFlag==TRUE) aus LFSearchResult entfernen
// Die Sortierreihenfolge geht verloren!
LFCORE_API void __stdcall LFRemoveFlaggedItems(LFSearchResult* pSearchResult);

// Sortiert LFSearchResult
LFCORE_API void __stdcall LFSortSearchResult(LFSearchResult* pSearchResult, UINT Attr, BOOL Descending);

// Gruppiert LFSearchResult und liefert Kopie zur�ck
LFCORE_API LFSearchResult* __stdcall LFGroupSearchResult(LFSearchResult* pSearchResult, UINT Attr, BOOL Descending, BOOL GroupOne, LFFilter* pFilter);


// Neue Transaktionsliste erzeugen
LFCORE_API LFTransactionList* __stdcall LFAllocTransactionList(HLIQUID hLiquid=NULL);

// Existierende LFTransactionList freigeben
LFCORE_API void __stdcall LFFreeTransactionList(LFTransactionList* tl);

// LFItemDescriptor zur LFTransactionList hinzuf�gen
LFCORE_API BOOL __stdcall LFAddTransactionItem(LFTransactionList* pTransactionList, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData=0);
LFCORE_API BOOL __stdcall LFAddTransactionItemEx(LFTransactionList* pTransactionList, CHAR* StoreID, CHAR* FileID, LFItemDescriptor* pItemDescriptor=NULL, UINT_PTR UserData=0);

// Handle zu DROPFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateDropFiles(LFTransactionList* pTransactionList);

// Handle zu LIQUIDFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateLiquidFiles(LFTransactionList* pTransactionList);


// Neue Datei-Importliste erzeugen
LFCORE_API LFFileImportList* __stdcall LFAllocFileImportList(HDROP hDrop=NULL);

// Existierende LFFileImportList freigeben
LFCORE_API void __stdcall LFFreeFileImportList(LFFileImportList* pFileImportList);

// String zur LFFileImportList hinzuf�gen
LFCORE_API BOOL __stdcall LFAddImportPath(LFFileImportList* pFileImportList, WCHAR* Path);


// Existierende LFMaintenanceList freigeben
LFCORE_API void __stdcall LFFreeMaintenanceList(LFMaintenanceList* ml);



// Geotagging
//

// Liefert die Anzahl der Territorien zur�ck
LFCORE_API UINT __stdcall LFIATAGetCountryCount();

// Liefert die Anzahl der Flugh�fen zur�ck
LFCORE_API UINT __stdcall LFIATAGetAirportCount();

// Liefert Zeiger auf Territorium zur�ck
LFCORE_API LFCountry* __stdcall LFIATAGetCountry(UINT CountryID);

// Setzt den Zeiger *pBuffer auf den n�chsten Flughafen
LFCORE_API INT __stdcall LFIATAGetNextAirport(INT Last, LFAirport** ppAirport);

// Setzt den Zeiger *pBuffer auf den n�chsten Flughafen, der im Territorium CountryID liegt.
// *pBuffer kann in jedem Fall �berschrieben werden.
LFCORE_API INT __stdcall LFIATAGetNextAirportByCountry(UINT CountryID, INT Last, LFAirport** ppAirport);

// Setzt den Zeiger *pBuffer auf den Flughafen mit dem �bergebenen Code.
// *pBuffer kann in jedem Fall �berschrieben werden.
LFCORE_API BOOL __stdcall LFIATAGetAirportByCode(CHAR* Code, LFAirport** ppAirport);



// Suchabfragen
//

// Suchabfrage durchf�hren
// - Ist pFilter==NULL, so wird eine Liste aller Stores zur�ckgeliefert
LFCORE_API LFSearchResult* __stdcall LFQuery(LFFilter* pFilter);

// Bestehendes Suchergebnis eingrenzen
// - pFilter muss vom Typ LFFilterModeDirectoryTree oder LFFilterModeSearch sein
// - pFilter muss ein Unterverzeichnis sein
// - First und Last m�ssen einen g�ltigen Bereich umfassen
LFCORE_API LFSearchResult* __stdcall LFQueryEx(LFFilter* pFilter, LFSearchResult* pSearchResult, INT First, INT Last);

// Statistik
// - Ist die StoreID leer, so wird die Statistik �ber alle Stores ermittelt
LFCORE_API LFStatistics* __stdcall LFQueryStatistics(CHAR* StoreID);



// Thumbnails
//

LFCORE_API HBITMAP __stdcall LFGetThumbnail(LFItemDescriptor* pItemDescriptor, SIZE sz);

// Skaliert eine Bitmap von 256x256 auf 128x128 (nur 32 Bit)
LFCORE_API HBITMAP LFQuarter256Bitmap(HBITMAP hBitmap);



// Shortcuts
//

// Speichert pShellLink auf dem Desktop ab
LFCORE_API void __stdcall LFCreateDesktopShortcut(IShellLink* pShellLink, WCHAR* LinkFilename);

// Liefert einen ShellLink f�r den angegebenen Store
LFCORE_API IShellLink* __stdcall LFGetShortcutForStore(LFItemDescriptor* pItemDescriptor);

// Erzeugt auf dem Desktop eine Verkn�pfung mit dem angegebenen Store
LFCORE_API void __stdcall LFCreateDesktopShortcutForStore(LFItemDescriptor* pItemDescriptor);
LFCORE_API void __stdcall LFCreateDesktopShortcutForStoreEx(LFStoreDescriptor* pStoreDescriptor);



















//
// Stores
//

// Gibt den physischen Pfad einer Datei zur�ck
LFCORE_API UINT __stdcall LFGetFileLocation(LFItemDescriptor* pItemDescriptor, WCHAR* dst, SIZE_T cCount, BOOL CheckExists, BOOL RemoveNew, BOOL Extended=FALSE);

// Gibt die Daten eines Stores zur�ck
LFCORE_API UINT __stdcall LFGetStoreSettings(CHAR* StoreID, LFStoreDescriptor* s);
LFCORE_API UINT __stdcall LFGetStoreSettings(GUID guid, LFStoreDescriptor* s);

// Gibt die ID f�r das Icon eines Stores zur�ck
LFCORE_API UINT __stdcall LFGetStoreIcon(LFStoreDescriptor* s);

// Pr�ft, ob ein Store angeschlossen ist
LFCORE_API BOOL __stdcall LFIsStoreMounted(LFStoreDescriptor* s);

// Legt einen neuen Store an
// - Eingabeparameter interner Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - IndexMode: erforderlich
//   - AutoLocation: erforderlich
//   - DatPath: erforderlich, wenn AutoLocation==TRUE
//   - Alle anderen Parameter werden ignoriert bzw. ausgef�llt
// - Eingabeparameter Hybrid-Store und externer Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - IndexMode: erforderlich
//   - DatPath: erforderlich
//   - Alle anderen Parameter werden ignoriert bzw. ausgef�llt
LFCORE_API UINT LFCreateStore(LFStoreDescriptor* s);

// Macht den internen Store zum Default Store
LFCORE_API UINT __stdcall LFMakeDefaultStore(CHAR* StoreID);

// Macht den externen Store zum Hybrid-Store
LFCORE_API UINT __stdcall LFMakeStoreSearchable(CHAR* StoreID, BOOL Searchable=TRUE);

// Setzt Namen und Kommentar eines Stores
// Ist name oder comment NULL, so wird der jeweilige Wert nicht ver�ndert
LFCORE_API UINT __stdcall LFSetStoreAttributes(CHAR* StoreID, WCHAR* name, WCHAR* comment, BOOL InternalCall=FALSE);

// L�scht einen bestehenden Store
LFCORE_API UINT __stdcall LFDeleteStore(CHAR* StoreID, LFProgress* pProgress=NULL);

// Startet geplante Wartungsarbeiten f�r alle Stores
LFCORE_API LFMaintenanceList* __stdcall LFStoreMaintenance(LFProgress* pProgress=NULL);

// Gibt an, ob ein Default Stores verf�gbar ist
LFCORE_API BOOL __stdcall LFDefaultStoreAvailable();

// Gibt die ID des aktuellen Default Stores zur�ck
LFCORE_API BOOL __stdcall LFGetDefaultStore(CHAR* StoreID);

// Gibt die Anzahl aller Stores zur�ck
LFCORE_API UINT __stdcall LFGetStoreCount();

// Pr�ft, ob Stores auf dem angegebenen Laufwerk vorhanden sind
LFCORE_API BOOL __stdcall LFStoresOnVolume(CHAR cVolume);

// Gibt die IDs aller Stores zur�ck
LFCORE_API UINT __stdcall LFGetStores(CHAR** IDs, UINT* count);






//
// Transaktionen
//

// Benennt die Datei um
LFCORE_API UINT __stdcall LFTransactionRename(CHAR* StoreID, CHAR* FileID, WCHAR* NewName);


// LFFileImportList
//

// Importiert Dateien in den Store
LFCORE_API void __stdcall LFTransactionImport(CHAR* key, LFFileImportList* il, LFItemDescriptor* it, BOOL recursive, BOOL move, LFProgress* pProgress=NULL);

// �ndert bei allen Eintr�gen in tl bis zu 3 Attributwerte
LFCORE_API void __stdcall LFTransactionUpdate(LFTransactionList* tl, LFVariantData* v1, LFVariantData* v2=NULL, LFVariantData* v3=NULL);

// Archiviert alle Dateien in tl
LFCORE_API void __stdcall LFTransactionArchive(LFTransactionList* tl);

// L�scht alle Dateien in tl
LFCORE_API void __stdcall LFTransactionDelete(LFTransactionList* tl, BOOL PutInTrash=TRUE, LFProgress* pProgress=NULL);

// Stellt alle Dateien in tl wieder her, sofern nicht entg�ltig gel�scht
LFCORE_API void __stdcall LFTransactionRestore(LFTransactionList* tl, UINT Flags);

// Physische Orte aufl�sen
LFCORE_API void __stdcall LFTransactionResolvePhysicalLocations(LFTransactionList* tl, BOOL IncludePIDL=FALSE);

// Importiert Dateien in den Store
LFCORE_API void __stdcall LFTransactionImport(CHAR* key, LFTransactionList* il, BOOL move, LFProgress* pProgress=NULL);

// F�gt die angegebenen Dateien zum Suchergebnis hinzu
LFCORE_API void __stdcall LFTransactionAddToSearchResult(LFTransactionList* il, LFSearchResult* sr);

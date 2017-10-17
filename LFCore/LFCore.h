
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


// Kernfunktionalität
//

#define LFGLV_INTERNAL     1
#define LFGLV_EXTERNAL     2
#define LFGLV_NETWORK      4
#define LFGLV_FLOPPIES     8


// liquidFOLDERS initalisieren
LFCORE_API void __stdcall LFInitialize();

// Gibt einen Zeiger auf die IDs aller registrierten Nachrichten zurück
LFCORE_API const LFMessageIDs* __stdcall LFGetMessageIDs();

// Gibt den Dateinamen der liquidFOLDERS-App zurück
LFCORE_API BOOL __stdcall LFGetApplicationPath(LPWSTR pStr, SIZE_T cCount);

// Liefert einen String mit Dateianzahl und -größe zurück
LFCORE_API void __stdcall LFGetFileSummary(LPWSTR pStr, SIZE_T cCount, UINT Count, INT64 Size=0);

// Liefert einen String mit Dateianzahl und  Dateigröße oder Laufzeit zurück
LFCORE_API void __stdcall LFGetFileSummaryEx(LPWSTR pStr, SIZE_T cCount, const LFFileSummary& Summary);


// Gibt TRUE zurück, wenn der Explorer Dateiendungen verbirgt
LFCORE_API BOOL __stdcall LFHideFileExt();

// Gibt TRUE zurück, wenn der Explorer leere Laufwerke verbirgt
LFCORE_API BOOL __stdcall LFHideVolumesWithNoMedia();


// Liefert den Source-Typ eines Laufwerks zurück
LFCORE_API UINT __stdcall LFGetSourceForVolume(CHAR cVolume);

// Wie Win32-Funktion GetLogicalVolumes(), allerdings selektiv (s.o.)
LFCORE_API UINT __stdcall LFGetLogicalVolumes(UINT Mask=LFGLV_INTERNAL | LFGLV_EXTERNAL | LFGLV_NETWORK);


// Initalisiert eine LFProgress-Datenstruktur
LFCORE_API void __stdcall LFInitProgress(LFProgress* pProgress, HWND hWnd, UINT MajorCount=0);


// Beschreibung eines Fehlers (LFError...) in aktueller Sprache zurückliefern
LFCORE_API void __stdcall LFGetErrorText(LPWSTR pStr, SIZE_T cCount, UINT ID);

// Anzeigen eines Fehlers (LFError...) in aktueller Sprache
LFCORE_API void __stdcall LFCoreErrorBox(UINT nID, HWND hWnd=NULL);


// Name einer Attribut-Kategorie in aktueller Sprache zurückliefern
LFCORE_API void __stdcall LFGetAttrCategoryName(LPWSTR pStr, SIZE_T cCount, UINT ID);

// Informationen über ein Attribut zurückliefern
LFCORE_API void __stdcall LFGetAttributeInfo(LFAttributeDescriptor& AttributeDescriptor, UINT ID);

// Name einer Datenquelle in aktueller Sprache zurückliefern
LFCORE_API void __stdcall LFGetSourceName(LPWSTR pStr, SIZE_T cCount, UINT ID, BOOL Qualified);

// Informationen über eine Kategorie zurückliefern
LFCORE_API void __stdcall LFGetItemCategoryInfo(LFItemCategoryDescriptor& ItemCategoryDescriptor, UINT ID);

// Informationen über einen Kontext zurückliefern
LFCORE_API void __stdcall LFGetContextInfo(LFContextDescriptor& ContextDescriptor, UINT ID);

// Gibt eine Liste aller Attribute zurück, die nach ihrer Priorität sortiert sind
LFCORE_API void __stdcall LFGetSortedAttributeList(LFAttributeList& AttributeList);


// Liefert eine Farbe für Dateien zurück
LFCORE_API COLORREF __stdcall LFGetItemColor(UINT ID, UINT Fade=LFItemColorFadePure);


// Gibt TRUE zurück, wenn diese Installation freigeschaltet ist
// Die gespeicherten Lizenzinformationen finden sich in License
LFCORE_API BOOL __stdcall LFIsLicensed(LFLicense* pLicense=NULL, BOOL Reload=FALSE);

// Gibt TRUE zurück, wenn die Shareware-Version ausgelaufen ist,
// und keine ordnungsgemäße Lizenz vorliegt
LFCORE_API BOOL __stdcall LFIsSharewareExpired();




// Stores
//

// Gibt die ID des aktuellen Standard-Stores zurück
LFCORE_API UINT __stdcall LFGetDefaultStore(LPSTR pStoreID=NULL);

// Macht einen Store zum Standard-Store
LFCORE_API UINT __stdcall LFSetDefaultStore(LPCSTR pStoreID);

// Gibt die Anzahl aller Stores zurück
LFCORE_API UINT __stdcall LFGetStoreCount();

// Gibt die IDs aller Stores zurück
LFCORE_API UINT __stdcall LFGetAllStores(CHAR** ppStoreIDs, UINT* pCount);

// Gibt die Daten eines Stores zurück
LFCORE_API UINT __stdcall LFGetStoreSettings(LPCSTR pStoreID, LFStoreDescriptor* pStoreDescriptor);
LFCORE_API UINT __stdcall LFGetStoreSettingsEx(const GUID UniqueID, LFStoreDescriptor* pStoreDescriptor);

// Prüft, ob Stores auf dem angegebenen Laufwerk vorhanden sind
LFCORE_API BOOL __stdcall LFStoresOnVolume(CHAR cVolume);

// Gibt die ID für das Icon eines Stores zurück
LFCORE_API UINT LFGetStoreIcon(const LFStoreDescriptor* pStoreDescriptor, UINT* pType=NULL);

// Prüft, ob ein Store angeschlossen ist
#define LFIsStoreMounted(pStoreDescriptor) ((pStoreDescriptor)->DatPath[0]!=L'\0')

// Erzeugt einen neuen Store
LFCORE_API UINT LFCreateStoreLiquidfolders(LPWSTR pStoreName=NULL, LPCWSTR pComments=NULL, CHAR cVolume='\0', BOOL MakeSearchable=FALSE);
LFCORE_API UINT LFCreateStoreWindows(LPCWSTR pPath, LPWSTR pStoreName=NULL, LFProgress* pProgress=NULL);

// Macht einen Store offline durchsuchbar
LFCORE_API UINT __stdcall LFMakeStoreSearchable(LPCSTR pStoreID, BOOL Searchable=TRUE);

// Löscht einen bestehenden Store
LFCORE_API UINT __stdcall LFDeleteStore(LPCSTR pStoreID, LFProgress* pProgress=NULL);

// Setzt Namen und Kommentar eines Stores
// Ist pName oder pComment NULL, so wird der jeweilige Wert nicht verändert
LFCORE_API UINT __stdcall LFSetStoreAttributes(LPCSTR pStoreID, LPCWSTR pName, LPCWSTR pComment);

// Synchronisiert einen Store
LFCORE_API UINT __stdcall LFSynchronizeStore(LPCSTR pStoreID, LFProgress* pProgress=NULL);

// Synchronisiert alle Stores
LFCORE_API UINT __stdcall LFSynchronizeStores(LFProgress* pProgress=NULL);

// Startet geplante Wartungsarbeiten für alle Stores
LFCORE_API LFMaintenanceList* __stdcall LFScheduledMaintenance(LFProgress* pProgress=NULL);

// Gibt den physischen Pfad einer Datei zurück
LFCORE_API UINT __stdcall LFGetFileLocation(LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount, BOOL RemoveNew, BOOL CheckExists=TRUE);

// Importiert Dateien
LFCORE_API void __stdcall LFDoFileImport(LFFileImportList* pFileImportList, BOOL Recursive, LPCSTR pStoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress=NULL);



// LFVariantData
//

// Prüft, ob ein Hashtag in einem Unicode-Array enthalten ist
LFCORE_API BOOL LFContainsHashtag(LPCWSTR pUnicodeArray, LPCWSTR pHashtag);

// Konvertiert einen FourCC in eine Zeichenkette
LFCORE_API void __stdcall LFFourCCToString(const UINT c, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine 32-Bit-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFUINTToString(const UINT u, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine 64-Bit-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFSizeToString(const INT64 s, LPWSTR pStr, SIZE_T cCount);

// Konvertiert einen Bruch in eine Zeichenkette
LFCORE_API void __stdcall LFFractionToString(const LFFraction f, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine Double-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFDoubleToString(const DOUBLE d, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine Koordinaten-Komponente in eine Zeichenkette
LFCORE_API void __stdcall LFGeoCoordinateToString(const DOUBLE c, LPWSTR pStr, SIZE_T cCount, BOOL IsLatitude, BOOL FillZero);

// Konvertiert eine Geo-Position in eine Zeichenkette
LFCORE_API void __stdcall LFGeoCoordinatesToString(const LFGeoCoordinates& c, LPWSTR pStr, SIZE_T cCount, BOOL FillZero);

// Konvertiert eine Zeit in eine Zeichenkette
LFCORE_API void __stdcall LFTimeToString(const FILETIME t, LPWSTR pStr, SIZE_T cCount, BOOL IncludeTime=TRUE);

// Konvertiert eine Bitrate in eine Zeichenkette
LFCORE_API void __stdcall LFBitrateToString(const UINT r, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine Zeitdauer in eine Zeichenkette
LFCORE_API void __stdcall LFDurationToString(UINT d, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine Megapixel-Angabe in eine Zeichenkette
LFCORE_API void __stdcall LFMegapixelToString(const DOUBLE d, LPWSTR pStr, SIZE_T cCount);

// Konvertiert ein Attribut in eine Zeichenkette
LFCORE_API void __stdcall LFAttributeToString(const LFItemDescriptor* pItemDescriptor, UINT Attr, LPWSTR pStr, SIZE_T cCount);

// Initalisiert eine LFVariantData-Struktur
LFCORE_API void __stdcall LFInitVariantData(LFVariantData& Value, UINT Attr);

// Löscht eine LFVariantData-Struktur
// Value.Attr muss gesetzt sein
LFCORE_API void __stdcall LFClearVariantData(LFVariantData& Value);

// Prüft, ob eine LVVariantData-Struktur Null ist
LFCORE_API BOOL __stdcall LFIsNullVariantData(const LFVariantData& Value);

// Konvertiert eine LFVariantData-Struktur in eine Zeichenkette
LFCORE_API void __stdcall LFVariantDataToString(const LFVariantData& Value, LPWSTR pStr, SIZE_T cCount);

// Erzeugt eine LFVariantData-Struktur aus einer Zeichenkette
LFCORE_API void __stdcall LFVariantDataFromString(LFVariantData& Value, LPCWSTR pStr);

// Vergleicht zwei Dateiattribute
LFCORE_API INT __stdcall LFCompareVariantData(LFVariantData& Value1, LFVariantData& Value2);

// Attributwert holen
LFCORE_API void __stdcall LFGetAttributeVariantData(const LFItemDescriptor* pItemDescriptor, LFVariantData& Value);
LFCORE_API void __stdcall LFGetAttributeVariantDataEx(const LFItemDescriptor* pItemDescriptor, UINT Attr, LFVariantData& Value);

// Attributwert setzen
LFCORE_API void __stdcall LFSetAttributeVariantData(LFItemDescriptor* pItemDescriptor, const LFVariantData& Value);

// Prüfen, ob ein Attributwert existiert
LFCORE_API BOOL __stdcall LFIsNullAttribute(const LFItemDescriptor* pItemDescriptor, UINT Attr);

// Entfernt doppelte Eintäge in einem Unicode-Array
LFCORE_API void __stdcall LFSanitizeUnicodeArray(LPWSTR pStr, SIZE_T cCount);



// Datenstrukturen
//

// Neuen LFItemDescriptor erzeugen und ggf. die Kern-Attribute belegen
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(const LFCoreAttributes* pCoreAttributes=NULL, LPVOID pStoreData=NULL, SIZE_T StoreDataSize=0);

// Neuen LFItemDescriptor für Store erzeugen
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptorEx(const LFStoreDescriptor* pStoreDescriptor);

// Unabhängige Kopie von pItemDescriptor erzeugen
LFCORE_API LFItemDescriptor* __stdcall LFCloneItemDescriptor(const LFItemDescriptor* pItemDescriptor);

// Existierenden LFItemDescriptor freigeben
LFCORE_API void __stdcall LFFreeItemDescriptor(LFItemDescriptor* pItemDescriptor);


// Neuen LFFilter erzeugen, ggf. als Kopie eines existierenden Filters
LFCORE_API LFFilter* __stdcall LFAllocFilter(const LFFilter* pFilter=NULL);

// Existierenden LFFilter freigeben
LFCORE_API void __stdcall LFFreeFilter(LFFilter* pFilter);

// Lädt einen abgespeicherten Filter
LFCORE_API LFFilter* __stdcall LFLoadFilter(LFItemDescriptor* pItemDescriptor);
LFCORE_API LFFilter* __stdcall LFLoadFilterEx(LPCWSTR pPath);

// Speichert einen Filter in einem Store ab
LFCORE_API UINT __stdcall LFSaveFilter(LPCSTR pStoreID, LFFilter* pFilter, LPCWSTR pName, LPCWSTR pComment=NULL);

// Neue LFFilterCondition erzeugen
LFCORE_API LFFilterCondition* __stdcall LFAllocFilterCondition(BYTE Compare, LFVariantData& Value, LFFilterCondition* pNext=NULL);
LFCORE_API LFFilterCondition* __stdcall LFAllocFilterConditionEx(BYTE Compare, UINT Attr, LFFilterCondition* pNext=NULL);

// Existierende LFFilterCondition freigeben
#define LFFreeFilterCondition(pFilterCondition) delete pFilterCondition;


// Neues Suchergebnis mit Kontext Context erzeugen
LFCORE_API LFSearchResult* __stdcall LFAllocSearchResult(BYTE Context);

// Existierendes LFSearchResult freigeben
LFCORE_API void __stdcall LFFreeSearchResult(LFSearchResult* pSearchResult);

// LFItemDescriptor zum LFSearchResult hinzufügen
LFCORE_API BOOL __stdcall LFAddItem(LFSearchResult* pSearchResult, LFItemDescriptor* pItemDescriptor);

// Alle markierten LFItemDescriptor (RemoveFlag==TRUE) aus LFSearchResult entfernen
// Die Sortierreihenfolge geht verloren!
LFCORE_API void __stdcall LFRemoveFlaggedItems(LFSearchResult* pSearchResult);

// Sortiert LFSearchResult
LFCORE_API void __stdcall LFSortSearchResult(LFSearchResult* pSearchResult, UINT Attr, BOOL Descending);

// Gruppiert LFSearchResult und liefert Kopie zurück
LFCORE_API LFSearchResult* __stdcall LFGroupSearchResult(LFSearchResult* pSearchResult, UINT Attr, BOOL Descending, BOOL GroupSingle, LFFilter* pFilter);

// Errechnet die Ordnerfarben nach Änderungen neu
LFCORE_API void LFUpdateFolderColors(LFSearchResult* pCookedFiles, const LFSearchResult* pRawFiles);


// Neue Transaktionsliste auf Basis von LFSearchResult erzeugen
LFCORE_API LFTransactionList* __stdcall LFAllocTransactionList(LFSearchResult* pSearchResult=NULL, BOOL All=FALSE);

// Neue Transaktionsliste auf Basis von HLIQUID-Handle erzeugen
LFCORE_API LFTransactionList* __stdcall LFAllocTransactionListEx(HLIQUID hLiquid);

// Existierende LFTransactionList freigeben
LFCORE_API void __stdcall LFFreeTransactionList(LFTransactionList* tl);

// LFItemDescriptor zur LFTransactionList hinzufügen
LFCORE_API BOOL __stdcall LFAddTransactionItem(LFTransactionList* pTransactionList, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData=0);
LFCORE_API BOOL __stdcall LFAddTransactionItemEx(LFTransactionList* pTransactionList, LPCSTR pStoreID, LPCSTR pFileID, LFItemDescriptor* pItemDescriptor=NULL, UINT_PTR UserData=0);

// Handle zu DROPFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateDropFiles(LFTransactionList* pTransactionList);

// Handle zu LIQUIDFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateLiquidFiles(LFTransactionList* pTransactionList);

// Transaktion ausführen
LFCORE_API void __stdcall LFDoTransaction(LFTransactionList* pTransactionList, UINT TransactionType, LFProgress* pProgress=NULL, UINT_PTR Parameter=0, LFVariantData* pVariantData1=NULL, LFVariantData* pVariantData2=NULL, LFVariantData* pVariantData3=NULL);


// Neue Datei-Importliste erzeugen
LFCORE_API LFFileImportList* __stdcall LFAllocFileImportList(HDROP hDrop=NULL);

// Existierende LFFileImportList freigeben
LFCORE_API void __stdcall LFFreeFileImportList(LFFileImportList* pFileImportList);

// String zur LFFileImportList hinzufügen
LFCORE_API BOOL __stdcall LFAddImportPath(LFFileImportList* pFileImportList, LPCWSTR pPath);


// Existierende LFMaintenanceList freigeben
LFCORE_API void __stdcall LFFreeMaintenanceList(LFMaintenanceList* ml);



// Geotagging
//

// Liefert die Anzahl der Territorien zurück
LFCORE_API UINT __stdcall LFIATAGetCountryCount();

// Liefert die Anzahl der Flughäfen zurück
LFCORE_API UINT __stdcall LFIATAGetAirportCount();

// Liefert Zeiger auf Territorium zurück
LFCORE_API const LFCountry* __stdcall LFIATAGetCountry(UINT CountryID);

// Setzt den Zeiger *ppAirport auf den nächsten Flughafen
LFCORE_API INT __stdcall LFIATAGetNextAirport(INT Last, LFAirport** ppAirport);

// Setzt den Zeiger *ppAirport auf den nächsten Flughafen, der im Territorium CountryID liegt.
// *ppAirport kann in jedem Fall überschrieben werden.
LFCORE_API INT __stdcall LFIATAGetNextAirportByCountry(UINT CountryID, INT Last, LFAirport** ppAirport);

// Setzt den Zeiger *pStr auf den Flughafen mit dem übergebenen Code.
// *pStr kann in jedem Fall überschrieben werden.
LFCORE_API BOOL __stdcall LFIATAGetAirportByCode(LPCSTR pCode, LFAirport** ppAirport);

// Gibt einen Hinweis-String für einen Flughafen zurück.
LFCORE_API void __stdcall LFIATAGetLocationNameForAirport(LFAirport* pAirport, LPWSTR pStr, SIZE_T cCount);

// Gibt einen Hinweis-String für einen IATA-Code zurück.
LFCORE_API void __stdcall LFIATAGetLocationNameForCode(LPCSTR pCode, LPWSTR pStr, SIZE_T cCount);


// ID3
//

// Setzt den Zeiger *ppMusicGenre auf das nächsten Genre
LFCORE_API INT LFID3GetNextMusicGenre(INT Last, const LFMusicGenre** ppMusicGenre);

// Setzt den Zeiger *ppMusicGenre auf das nächsten Genre mit dem Icon IconID.
// *ppMusicGenre kann in jedem Fall überschrieben werden.
LFCORE_API INT LFID3GetNextMusicGenreByIcon(UINT IconID, INT Last, const LFMusicGenre** ppMusicGenre);



// Suchabfragen
//

// Suchabfrage durchführen
// - Ist pFilter==NULL, so wird eine Liste aller Stores zurückgeliefert
LFCORE_API LFSearchResult* __stdcall LFQuery(LFFilter* pFilter);

// Bestehendes Suchergebnis eingrenzen
// - pFilter muss vom Typ LFFilterModeDirectoryTree oder LFFilterModeSearch sein
// - pFilter muss ein Unterverzeichnis sein
// - First und Last müssen einen gültigen Bereich umfassen
LFCORE_API LFSearchResult* __stdcall LFQueryEx(LFFilter* pFilter, LFSearchResult* pSearchResult, INT First, INT Last);

// Statistik
// - Ist die StoreID leer, so wird die Statistik über alle Stores ermittelt
LFCORE_API UINT __stdcall LFQueryStatistics(LFStatistics& Statistics, LPCSTR StoreID=NULL);



// Thumbnails
//

LFCORE_API HBITMAP __stdcall LFGetThumbnail(LFItemDescriptor* pItemDescriptor, SIZE sz);

// Skaliert eine Bitmap von 256x256 auf 128x128 (nur 32 Bit)
LFCORE_API HBITMAP LFQuarter256Bitmap(HBITMAP hBitmap);



// Shortcuts
//

// Speichert pShellLink auf dem Desktop ab
LFCORE_API void __stdcall LFCreateDesktopShortcut(IShellLink* pShellLink, LPCWSTR pLinkFileName);

// Liefert einen ShellLink für den angegebenen Store
LFCORE_API IShellLink* __stdcall LFGetShortcutForStore(LFItemDescriptor* pItemDescriptor);

// Erzeugt auf dem Desktop eine Verknüpfung mit dem angegebenen Store
LFCORE_API void __stdcall LFCreateDesktopShortcutForStore(LFItemDescriptor* pItemDescriptor);
LFCORE_API void __stdcall LFCreateDesktopShortcutForStoreEx(LFStoreDescriptor* pStoreDescriptor);



// Cloud
//

// Liefert den Pfad des Box-Ordners zurück
LFCORE_API BOOL LFGetBoxPath(LPWSTR pPath);


// Liefert den Pfad des iCloud-Drive-Ordners zurück
LFCORE_API BOOL LFGetICloudPath(LPWSTR pPath);


// Liefert die Pfade von OneDrive zurück
LFCORE_API BOOL LFGetOneDrivePaths(LFOneDrivePaths& OneDrivePaths);

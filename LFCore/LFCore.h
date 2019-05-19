
#pragma once
#include "LF.h"
#include "LFFileImportList.h"
#include "LFMaintenanceList.h"
#include "LFMemorySort.h"
#include "LFSearchResult.h"
#include "LFTransactionList.h"

#ifdef LFCore_EXPORTS
#define LFCORE_API __declspec(dllexport,nothrow,noinline)
#else
#define LFCORE_API __declspec(dllimport,nothrow,noinline)
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
LFCORE_API void __stdcall LFInitProgress(LFProgress& Progress, HWND hWnd, UINT MajorCount=0);


// Beschreibung eines Fehlers (LFError...) in aktueller Sprache zurückliefern
LFCORE_API void __stdcall LFGetErrorText(LPWSTR pStr, SIZE_T cCount, UINT ID);

// Anzeigen eines Fehlers (LFError...) in aktueller Sprache
LFCORE_API void __stdcall LFCoreErrorBox(UINT nID, HWND hWnd=NULL);


// Sortiert einen Speicherbereich
LFCORE_API void LFSortMemory(LPVOID pMemory, UINT ItemCount, SIZE_T szData, PFNCOMPARE zCompare, UINT Attr=0, BOOL Descending=FALSE, BOOL Parameter1=FALSE, BOOL Parameter2=FALSE);


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

// Gibt TRUE zurück, wenn es sich bei dem Context um einen Unterordner handelt
#define LFIsSubfolderContext(ID) (ID>=LFContextSearch)


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
LFCORE_API UINT __stdcall LFGetDefaultStore(STOREID& StoreID);

// Macht einen Store zum Standard-Store
LFCORE_API UINT __stdcall LFSetDefaultStore(const ABSOLUTESTOREID& StoreID);

// Prüft, ob es sich bei einer ID um den Default Store handelt
inline BOOL LFIsDefaultStoreID(const STOREID& StoreID)
{
	return (StoreID[0]=='\0');
}

// Gibt die Anzahl aller Stores zurück
LFCORE_API UINT __stdcall LFGetStoreCount();

// Ersetzt die ID eines Stores ggf. durch den Default Store
inline UINT LFResolveStoreID(STOREID& StoreID)
{
	return LFIsDefaultStoreID(StoreID) ? LFGetDefaultStore(StoreID) : LFOk;
}

// Kopiert die ID eines Stores zurück und ersetzt sie ggf. durch den Default Store
LFCORE_API UINT __stdcall LFResolveStoreIDEx(ABSOLUTESTOREID& AbsoluteStoreID, const STOREID& StoreID);

// Gibt die IDs aller Stores zurück
LFCORE_API UINT __stdcall LFGetAllStores(LPCABSOLUTESTOREID& lpcStoreIDs, UINT& Count);

// Gibt die Daten eines Stores zurück
LFCORE_API UINT __stdcall LFGetStoreSettings(const STOREID& StoreID, LFStoreDescriptor& StoreDescriptor, BOOL DiskFreeSpace=FALSE);
LFCORE_API UINT __stdcall LFGetStoreSettingsEx(const GUID UniqueID, LFStoreDescriptor& StoreDescriptor);

// Prüft, ob Stores auf dem angegebenen Laufwerk vorhanden sind
LFCORE_API BOOL __stdcall LFStoresOnVolume(CHAR cVolume);

// Gibt die ID für das Icon eines Stores zurück
LFCORE_API UINT __stdcall LFGetStoreIcon(const LFStoreDescriptor* pStoreDescriptor, UINT* pType=NULL);

// Prüft, ob ein Store angeschlossen ist
inline BOOL LFIsStoreMounted(const LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	return pStoreDescriptor->DatPath[0]!=L'\0';
}

// Erzeugt einen neuen Store
LFCORE_API UINT __stdcall LFCreateStoreLiquidfolders(LPWSTR pStoreName=NULL, LPCWSTR pComments=NULL, CHAR cVolume='\0', BOOL MakeSearchable=FALSE);
LFCORE_API UINT __stdcall LFCreateStoreWindows(LPCWSTR pPath, LPWSTR pStoreName=NULL, LFProgress* pProgress=NULL);

// Macht einen Store offline durchsuchbar
LFCORE_API UINT __stdcall LFMakeStoreSearchable(const ABSOLUTESTOREID& StoreID, BOOL Searchable=TRUE);

// Löscht einen bestehenden Store
LFCORE_API UINT __stdcall LFDeleteStore(const ABSOLUTESTOREID& StoreID, LFProgress* pProgress=NULL);

// Setzt Namen und Kommentar eines Stores
// Ist pName oder pComment NULL, so wird der jeweilige Wert nicht verändert
LFCORE_API UINT __stdcall LFSetStoreAttributes(const ABSOLUTESTOREID& StoreID, LPCWSTR pName, LPCWSTR pComment);

// Synchronisiert Stores
LFCORE_API UINT __stdcall LFSynchronizeStores(const STOREID& StoreID, LFProgress* pProgress=NULL);

// Startet geplante Wartungsarbeiten für alle Stores
LFCORE_API LFMaintenanceList* __stdcall LFScheduledMaintenance(LFProgress* pProgress=NULL);

// Gibt den physischen Pfad einer Datei zurück
LFCORE_API UINT __stdcall LFGetFileLocation(LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount, BOOL RemoveNew=TRUE);

// Importiert Dateien
LFCORE_API UINT __stdcall LFDoFileImport(LFFileImportList* pFileImportList, BOOL Recursive, const STOREID& StoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress=NULL);



// LFVariantData
//

// Prüft, ob ein Hashtag in einem Unicode-Array enthalten ist
LFCORE_API BOOL __stdcall LFContainsHashtag(LPCWSTR pUnicodeArray, LPCWSTR pHashtag);

// Konvertiert einen FourCC in eine Zeichenkette
LFCORE_API void __stdcall LFFourCCToString(const UINT FourCC, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine 32-Bit-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFUINTToString(const UINT Uint, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine 64-Bit-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFSizeToString(const INT64 Size, LPWSTR pStr, SIZE_T cCount);

// Konvertiert einen Bruch in eine Zeichenkette
LFCORE_API void __stdcall LFFractionToString(const LFFraction& Fraction, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine Double-Zahl in eine Zeichenkette
LFCORE_API void __stdcall LFDoubleToString(const DOUBLE Double, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine Koordinaten-Komponente in eine Zeichenkette
LFCORE_API void __stdcall LFGeoCoordinateToString(const DOUBLE Coordinate, LPWSTR pStr, SIZE_T cCount, BOOL IsLatitude, BOOL FillZero);

// Konvertiert eine Geo-Position in eine Zeichenkette
LFCORE_API void __stdcall LFGeoCoordinatesToString(const LFGeoCoordinates& Coordinates, LPWSTR pStr, SIZE_T cCount, BOOL FillZero);

// Konvertiert eine Zeit in eine Zeichenkette
LFCORE_API void __stdcall LFTimeToString(const FILETIME& Time, LPWSTR pStr, SIZE_T cCount, BOOL IncludeTime=TRUE);

// Konvertiert eine Bitrate in eine Zeichenkette
LFCORE_API void __stdcall LFBitrateToString(const UINT Bitrate, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine Zeitdauer in eine Zeichenkette
LFCORE_API void __stdcall LFDurationToString(const UINT Duration, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine Megapixel-Angabe in eine Zeichenkette
LFCORE_API void __stdcall LFMegapixelToString(const DOUBLE Resolution, LPWSTR pStr, SIZE_T cCount);

// Konvertiert eine Bildraten in eine Zeichenkette
LFCORE_API void __stdcall LFFramerateToString(const UINT Framerate, LPWSTR pStr, SIZE_T cCount);

// Konvertiert ein Attribut in eine Zeichenkette
LFCORE_API void __stdcall LFAttributeToString(const LFItemDescriptor* pItemDescriptor, UINT Attr, LPWSTR pStr, SIZE_T cCount);

// Initalisiert eine LFVariantData-Struktur
LFCORE_API void __stdcall LFInitVariantData(LFVariantData& VData, UINT Attr);

// Löscht eine LFVariantData-Struktur
// Value.Attr muss gesetzt sein
LFCORE_API void __stdcall LFClearVariantData(LFVariantData& VData);

// Prüft, ob eine LVVariantData-Struktur Null ist
LFCORE_API BOOL __stdcall LFIsNullVariantData(const LFVariantData& VData);

// Konvertiert eine LFVariantData-Struktur in eine Zeichenkette
LFCORE_API void __stdcall LFVariantDataToString(const LFVariantData& VData, LPWSTR pStr, SIZE_T cCount);

// Erzeugt eine LFVariantData-Struktur aus einer Zeichenkette
LFCORE_API void __stdcall LFVariantDataFromString(LFVariantData& VData, LPCWSTR pStr);

// Vergleicht zwei Dateiattribute
LFCORE_API INT __stdcall LFCompareVariantData(LFVariantData& Data1, LFVariantData& Data2);

// Attributwert holen
LFCORE_API void __stdcall LFGetAttributeVariantData(const LFItemDescriptor* pItemDescriptor, LFVariantData& VData);
LFCORE_API void __stdcall LFGetAttributeVariantDataEx(const LFItemDescriptor* pItemDescriptor, UINT Attr, LFVariantData& VData);

// Attributwert setzen
LFCORE_API void __stdcall LFSetAttributeVariantData(LFItemDescriptor* pItemDescriptor, const LFVariantData& VData);

// Prüfen, ob ein Attributwert existiert
LFCORE_API BOOL __stdcall LFIsNullAttribute(const LFItemDescriptor* pItemDescriptor, UINT Attr);

// Entfernt doppelte Eintäge in einem Unicode-Array
LFCORE_API void __stdcall LFSanitizeUnicodeArray(LPWSTR pStr, SIZE_T cCount);



// Datenstrukturen
//

// Neuen LFItemDescriptor erzeugen und ggf. die Kern-Attribute belegen
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(const LPCCOREATTRIBUTES pCoreAttributes=NULL, LPCVOID pStoreData=NULL, SIZE_T StoreDataSize=0);

// Neuen LFItemDescriptor für Store erzeugen
LFCORE_API LFItemDescriptor* __stdcall LFAllocItemDescriptorEx(const LFStoreDescriptor& StoreDescriptor);

// Unabhängige Kopie von pItemDescriptor erzeugen
LFCORE_API LFItemDescriptor* __stdcall LFCloneItemDescriptor(const LFItemDescriptor* pItemDescriptor);

// Existierenden LFItemDescriptor freigeben
LFCORE_API void __stdcall LFFreeItemDescriptor(LFItemDescriptor* pItemDescriptor);

inline UINT LFGetItemType(const LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	return (pItemDescriptor->Type & LFTypeMask);
}

// Prüft, ob der LFItemDescriptor ein Store ist
inline BOOL LFIsStore(const LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	return LFGetItemType(pItemDescriptor)==LFTypeStore;
}

// Prüft, ob der LFItemDescriptor ein Ordner ist
inline BOOL LFIsFolder(const LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	return LFGetItemType(pItemDescriptor)==LFTypeFolder;
}

// Prüft, ob der LFItemDescriptor aggregiert ist
inline BOOL LFIsAggregated(const LFItemDescriptor* pItemDescriptor)
{
	assert(LFIsFolder(pItemDescriptor));
	assert((pItemDescriptor->AggregateFirst!=-1)==(pItemDescriptor->AggregateLast!=-1));

	return (pItemDescriptor->AggregateFirst!=-1);
}

// Prüft, ob der LFItemDescriptor ein aggregierter Ordner ist
inline BOOL LFIsAggregatedFolder(const LFItemDescriptor* pItemDescriptor)
{
	return LFIsFolder(pItemDescriptor) && LFIsAggregated(pItemDescriptor);
}

// Prüft, ob der LFItemDescriptor eine Datei ist
inline BOOL LFIsFile(const LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	return LFGetItemType(pItemDescriptor)==LFTypeFile;
}

// Prüft, ob der Name des LFItemDescriptor in Haupt- und Nebenüberschrift aufgeteilt ist
inline BOOL LFHasSubcaption(const LFItemDescriptor* pItemDescriptor)
{
	assert(!LFIsStore(pItemDescriptor));
	assert((pItemDescriptor->FolderMainCaptionCount==0) || (pItemDescriptor->FolderSubcaptionStart>pItemDescriptor->FolderMainCaptionCount));

	return pItemDescriptor->FolderMainCaptionCount;
}

// Liefert die Nebenüberschrift zurück
inline LPCWSTR LFGetSubcaption(const LFItemDescriptor* pItemDescriptor)
{
	assert(LFHasSubcaption(pItemDescriptor));

	return &pItemDescriptor->CoreAttributes.FileName[pItemDescriptor->FolderSubcaptionStart];
}


// Gibt den System-Kontext einer Datei zurück
inline BYTE LFGetSystemContextID(const LFCoreAttributes& CoreAttributes)
{
	return CoreAttributes.SystemContextID;
}

// Gibt den System-Kontext einer Datei zurück
inline BYTE LFGetSystemContextID(const LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	return LFGetSystemContextID(pItemDescriptor->CoreAttributes);
}

// Gibt den Benutzer-Kontext einer Datei zurück
inline BYTE LFGetUserContextID(const LFCoreAttributes& CoreAttributes)
{
	return CoreAttributes.UserContextID ? CoreAttributes.UserContextID : LFGetSystemContextID(CoreAttributes);
}

// Gibt den Benutzer-Kontext einer Datei zurück
inline BYTE LFGetUserContextID(const LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	return LFGetUserContextID(pItemDescriptor->CoreAttributes);
}

// Gibt TRUE zurück, wenn die Datei eine Audio-Datei ist
#define LFIsAudioFile(ITEM) (LFGetSystemContextID(ITEM)==LFContextAudio)

// Gibt TRUE zurück, wenn die Datei ein Dokument ist
#define LFIsDocumentFileLoose(ITEM) (LFIsDocumentFileStrict(ITEM) || (LFGetSystemContextID(ITEM)==LFContextColorTables))

// Gibt TRUE zurück, wenn die Datei ein Dokument ist
#define LFIsDocumentFileStrict(ITEM) (LFGetSystemContextID(ITEM)==LFContextDocuments)

// Gibt TRUE zurück, wenn die Datei ein Filter ist
#define LFIsFilterFile(ITEM) (LFGetSystemContextID(ITEM)==LFContextFilters)

// Gibt TRUE zurück, wenn die Datei eine Mediendatei ist
#define LFIsMediaFile(ITEM) ((LFGetSystemContextID(ITEM)>=LFContextAudio) && (LFGetSystemContextID(ITEM)<=LFContextVideos))

// Gibt TRUE zurück, wenn die Datei ein Bild ist
#define LFIsPictureFile(ITEM) (LFGetSystemContextID(ITEM)==LFContextPictures)

// Gibt TRUE zurück, wenn der Ordner durch ein einziges Vorschaubild seines Inhalts repräsentiert werden kann
#define LFIsRepresentativeFolder(ITEM) ((LFGetUserContextID(ITEM)==LFContextMusic) || (LFGetUserContextID(ITEM)==LFContextPodcasts))

// Gibt TRUE zurück, wenn die Datei eine zeitbasierte Mediendatei ist
#define LFIsTimebasedMediaFile(ITEM) ((LFGetSystemContextID(ITEM)==LFContextAudio) || (LFGetSystemContextID(ITEM)==LFContextVideos))

// Gibt TRUE zurück, wenn die Datei ein Video ist
#define LFIsVideoFile(ITEM) (LFGetSystemContextID(pItemDescriptor)==LFContextVideos)


// Neuen LFFilter erzeugen
LFCORE_API LFFilter* __stdcall LFAllocFilter(BYTE Mode=LFFilterModeQuery);

// Existierenden LFFilter freigeben
LFCORE_API void __stdcall LFFreeFilter(LFFilter* pFilter);

// Lädt einen abgespeicherten Filter
LFCORE_API LFFilter* __stdcall LFLoadFilter(LFItemDescriptor* pItemDescriptor);

// Speichert einen Filter in einem Store ab
LFCORE_API UINT __stdcall LFSaveFilter(const STOREID& StoreID, LFFilter* pFilter, LPCWSTR pName, LPCWSTR pComment=NULL);

// Neue LFFilterCondition erzeugen
LFCORE_API LFFilterCondition* __stdcall LFAllocFilterCondition(BYTE Compare, const LFVariantData& VData, LFFilterCondition* pNext=NULL);

// Existierende LFFilterCondition freigeben
inline void LFFreeFilterCondition(LFFilterCondition* pFilterCondition)
{
	delete pFilterCondition;
}

// Liefert das Attribut zurück, nachdem ein Unterordnet gebildet wirde
#define LFGetSubfolderAttribute(pFilter) (pFilter && pFilter->IsSubfolder && pFilter->Query.pConditionList ? pFilter->Query.pConditionList->VData.Attr : -1)


// Neues Suchergebnis mit Kontext Context erzeugen
LFCORE_API LFSearchResult* __stdcall LFAllocSearchResult(BYTE Context);

// Existierendes LFSearchResult freigeben
LFCORE_API void __stdcall LFFreeSearchResult(LFSearchResult* pSearchResult);

// LFItemDescriptor zum LFSearchResult hinzufügen, sofern noch nicht vorhanden
LFCORE_API BOOL __stdcall LFAddItem(LFSearchResult* pSearchResult, LFItemDescriptor* pItemDescriptor);

// Alle markierten LFItemDescriptor (RemoveFlag==TRUE) aus LFSearchResult entfernen
// Die Sortierreihenfolge geht verloren!
LFCORE_API void __stdcall LFRemoveFlaggedItems(LFSearchResult* pSearchResult);

// Sortiert LFSearchResult
LFCORE_API void __stdcall LFSortSearchResult(LFSearchResult* pSearchResult, UINT Attr, BOOL Descending=FALSE);

// Gruppiert LFSearchResult und liefert Kopie zurück
LFCORE_API LFSearchResult* __stdcall LFGroupSearchResult(LFSearchResult* pSearchResult, UINT Attr, BOOL Descending, BOOL GroupSingle, LFFilter* pFilter);

// Errechnet die Ordnerfarben nach Änderungen neu
LFCORE_API void LFUpdateFolderColors(LFSearchResult* pCookedFiles, const LFSearchResult* pRawFiles);


// Neue Transaktionsliste auf Basis von LFSearchResult erzeugen
LFCORE_API LFTransactionList* __stdcall LFAllocTransactionList(LFSearchResult* pSearchResult=NULL, BOOL All=FALSE);

// Neue Transaktionsliste auf Basis von HLIQUIDFILES-Handle erzeugen
LFCORE_API LFTransactionList* __stdcall LFAllocTransactionListEx(HLIQUIDFILES hLiquidFiles);

// Existierende LFTransactionList freigeben
LFCORE_API void __stdcall LFFreeTransactionList(LFTransactionList* pTransactionList);

// LFItemDescriptor zur LFTransactionList hinzufügen
LFCORE_API BOOL __stdcall LFAddTransactionItem(LFTransactionList* pTransactionList, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData=0);

// Handle zu DROPFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateDropFiles(LFTransactionList* pTransactionList);

// Handle zu LIQUIDFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCORE_API HGLOBAL __stdcall LFCreateLiquidFiles(LFTransactionList* pTransactionList);

// Transaktion ausführen
LFCORE_API UINT __stdcall LFDoTransaction(LFTransactionList* pTransactionList, UINT TransactionType, LFProgress* pProgress=NULL, UINT_PTR Parameter=0, const LFVariantData* pVariantData1=NULL, const LFVariantData* pVariantData2=NULL, const LFVariantData* pVariantData3=NULL);


// Neue Datei-Importliste erzeugen
LFCORE_API LFFileImportList* __stdcall LFAllocFileImportList(HDROP hDrop=NULL);

// Existierende LFFileImportList freigeben
inline void LFFreeFileImportList(LFFileImportList* pFileImportList)
{
	delete pFileImportList;
}

// String zur LFFileImportList hinzufügen
LFCORE_API BOOL __stdcall LFAddImportPath(LFFileImportList* pFileImportList, LPCWSTR pPath);


// Existierende LFMaintenanceList freigeben
inline void LFFreeMaintenanceList(LFMaintenanceList* pMaintenanceList)
{
	delete pMaintenanceList;
}



// Geotagging
//

// Liefert die Anzahl der Territorien zurück
LFCORE_API UINT __stdcall LFIATAGetCountryCount();

// Liefert die Anzahl der Flughäfen zurück
LFCORE_API UINT __stdcall LFIATAGetAirportCount();

// Liefert Zeiger auf Territorium zurück
LFCORE_API LPCCOUNTRY __stdcall LFIATAGetCountry(UINT CountryID);

// Setzt den Zeiger *ppAirport auf den nächsten Flughafen
LFCORE_API INT __stdcall LFIATAGetNextAirport(INT Last, LPCAIRPORT& lpcAirport);

// Setzt den Zeiger *ppAirport auf den nächsten Flughafen, der im Territorium CountryID liegt.
// *ppAirport kann in jedem Fall überschrieben werden.
LFCORE_API INT __stdcall LFIATAGetNextAirportByCountry(UINT CountryID, INT Last, LPCAIRPORT& lpcAirport);

// Setzt den Zeiger *pStr auf den Flughafen mit dem übergebenen Code.
// *pStr kann in jedem Fall überschrieben werden.
LFCORE_API BOOL __stdcall LFIATAGetAirportByCode(LPCSTR lpcszCode, LPCAIRPORT& lpcAirport);

// Gibt einen Hinweis-String für einen Flughafen zurück.
LFCORE_API void __stdcall LFIATAGetLocationNameForAirport(LPCAIRPORT lpcAirport, LPWSTR pStr, SIZE_T cCount);

// Gibt einen Hinweis-String für einen IATA-Code zurück.
LFCORE_API void __stdcall LFIATAGetLocationNameForCode(LPCSTR lpcszCode, LPWSTR pStr, SIZE_T cCount);


// ID3
//

// Setzt den Zeiger *ppMusicGenre auf das nächsten Genre
LFCORE_API INT __stdcall LFID3GetNextMusicGenre(INT Last, LPCMUSICGENRE& lpcMusicGenre);

// Setzt den Zeiger *ppMusicGenre auf das nächsten Genre mit dem Icon IconID.
// *ppMusicGenre kann in jedem Fall überschrieben werden.
LFCORE_API INT __stdcall LFID3GetNextMusicGenreByIcon(UINT IconID, INT Last, LPCMUSICGENRE& lpcMusicGenre);



// Suchabfragen
//

// Suchabfrage durchführen
// - Ist pFilter==NULL, so wird eine Liste aller Stores zurückgeliefert
LFCORE_API LFSearchResult* __stdcall LFQuery(LFFilter* pFilter);

// Bestehendes Suchergebnis eingrenzen
// - pFilter muss vom Typ LFFilterModeDirectoryTree oder LFFilterModeQuery sein
// - pFilter muss ein Unterverzeichnis sein
// - First und Last müssen einen gültigen Bereich umfassen
LFCORE_API LFSearchResult* __stdcall LFQueryEx(LFFilter* pFilter, LFSearchResult* pSearchResult, INT First, INT Last);

// Statistik
// - Ist die StoreID leer, so wird die Statistik über alle Stores ermittelt
LFCORE_API UINT __stdcall LFQueryStatistics(LFStatistics& Statistics, const STOREID& StoreID, UINT64* pGlobalContextSet=NULL);



// Thumbnails
//

LFCORE_API HBITMAP __stdcall LFGetThumbnail(LFItemDescriptor* pItemDescriptor, SIZE Size);

// Bereitet eine Bitmap für die Ausgabe vor, da viele Thumbnail-Handler korrupte Vorschaubilder zurückliefern.
// Die Bitmap wird ggf. auf maximal 128x128 Pixel skaliert.
LFCORE_API HBITMAP __stdcall LFSanitizeThumbnail(HBITMAP hBitmap);



// Shortcuts
//

// Erstellt ein IShellLink-Objekt
#define LFCreateShellLink(ppShellLink) SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&ppShellLink))

// Liefert einen ShellLink für das angegebene Element
LFCORE_API UINT __stdcall LFGetShortcutForItem(LFItemDescriptor* pItemDescriptor, IShellLink*& pShellLink);

// Erzeugt auf dem Desktop eine Verknüpfung mit dem angegebenen Store
LFCORE_API UINT __stdcall LFCreateDesktopShortcutForItem(LFItemDescriptor* pItemDescriptor);
LFCORE_API UINT __stdcall LFCreateDesktopShortcutForStore(const LFStoreDescriptor& StoreDescriptor);



// Cloud
//

// Liefert den Pfad des Box-Ordners zurück
LFCORE_API BOOL __stdcall LFGetBoxPath(LPWSTR pPath);

// Liefert den Pfad des Google-Drive-Ordners zurück
LFCORE_API BOOL __stdcall LFGetGoogleDrivePath(LPWSTR pPath);

// Liefert die Pfade von iCloud zurück
LFCORE_API BOOL __stdcall LFGetICloudPaths(LFICloudPaths& iCloudPaths);

// Liefert die Pfade von OneDrive zurück
LFCORE_API BOOL __stdcall LFGetOneDrivePaths(LFOneDrivePaths& OneDrivePaths);

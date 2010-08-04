// Folgender ifdef-Block ist die Standardmethode zum Erstellen von Makros, die das Exportieren 
// aus einer DLL vereinfachen. Alle Dateien in dieser DLL werden mit dem LFCore_EXPORTS-Symbol
// kompiliert, das in der Befehlszeile definiert wurde. Das Symbol darf nicht für ein Projekt definiert werden,
// das diese DLL verwendet. Alle anderen Projekte, deren Quelldateien diese Datei beinhalten, erkennen 
// LFCore_API-Funktionen als aus einer DLL importiert, während die DLL
// mit diesem Makro definierte Symbole als exportiert ansieht.

#ifdef LFCore_EXPORTS
#define LFCore_API __declspec(dllexport)
#else
#define LFCore_API __declspec(dllimport)
#endif

#pragma once
#include "liquidFOLDERS.h"
#pragma warning(disable:4428)


//
// Kernfunktionalität
//

#define LFGLD_Internal                   1
#define LFGLD_External                   2
#define LFGLD_Both                       3
#define LFGLD_Network                    4
#define LFGLD_All                        7

#define DRIVE_EXTHD                      100

// Wie Win32-Funktion GetLogicalDrives(), allerdings selektiv (s.o.)
LFCore_API unsigned int LFGetLogicalDrives(unsigned int mask=LFGLD_Both);

// Gibt den Index für das Laufwerks-Icon zurück
LFCore_API unsigned int LFGetDriveIcon(char Drv, bool IsMounted=true);

// Gibt einen Zeiger auf die IDs aller registrierten Nachrichten zurück
LFCore_API LFMessageIDs* LFGetMessageIDs();

// Gibt true zurück, wenn diese Installation freigeschaltet ist
// Die gespeicherten Lizenzinformationen finden sich in License
LFCore_API bool LFIsLicensed(LFLicense* License=NULL, bool Reload=false);

// Erzeugt einen Link mit DropHandler zur Explorer-Erweiterung
// im SendTo-Ordner des Benutzers
// Wenn force==false wird der Link nur beim ersten Aufruf erzeugt
LFCore_API void LFCreateSendTo(bool force=false);



// Neuen LFAttributeDescriptor erzeugen
LFCore_API LFAttributeDescriptor* LFAllocAttributeDescriptor();

// Informationen über ein Attribut zurückliefern
LFCore_API LFAttributeDescriptor* LFGetAttributeInfo(unsigned int ID);

// Existierenden LFAttributeDescriptor freigeben
LFCore_API void LFFreeAttributeDescriptor(LFAttributeDescriptor* a);



// Neuen LFItemCategoryDescriptor erzeugen
LFCore_API LFItemCategoryDescriptor* LFAllocItemCategoryDescriptor();

// Informationen über eine Kategorie zurückliefern
LFCore_API LFItemCategoryDescriptor* LFGetItemCategoryInfo(unsigned int ID);

// Existierenden LFItemCategoryDescriptor freigeben
LFCore_API void LFFreeItemCategoryDescriptor(LFItemCategoryDescriptor* c);



// Neuen LFContextDescriptor erzeugen
LFCore_API LFContextDescriptor* LFAllocContextDescriptor();

// Informationen über ein Attribut zurückliefern
LFCore_API LFContextDescriptor* LFGetContextInfo(unsigned int ID);

// Existierenden LFContextDescriptor freigeben
LFCore_API void LFFreeContextDescriptor(LFContextDescriptor* c);



// Neuen LFDomainDescriptor erzeugen
LFCore_API LFDomainDescriptor* LFAllocDomainDescriptor();

// Informationen über eine Domain zurückliefern
LFCore_API LFDomainDescriptor* LFGetDomainInfo(unsigned int ID);

// Existierenden LFDomainDescriptor freigeben
LFCore_API void LFFreeDomainDescriptor(LFDomainDescriptor* f);



// Neuen LFItemDescriptor erzeugen und zurücksetzen
// Ggf. wird eine unabhängige Kopie von i erzeugt
LFCore_API LFItemDescriptor* LFAllocItemDescriptor(LFItemDescriptor* i=NULL);

// Existierenden LFItemDescriptor freigeben
LFCore_API void LFFreeItemDescriptor(LFItemDescriptor* i);

// Attributwert holen
LFCore_API void LFGetAttributeVariantData(LFItemDescriptor* i, LFVariantData* v);

// Attributwert setzen
LFCore_API void LFSetAttributeVariantData(LFItemDescriptor* i, LFVariantData* v);



// Konvertiert einen FourCC in eine Zeichenkette
LFCore_API void LFFourCCToString(const unsigned int c, wchar_t* str, size_t cCount);

// Konvertiert eine 32-Bit-Zahl in eine Zeichenkette
LFCore_API void LFUINTToString(const unsigned int v, wchar_t* str, size_t cCount);

// Konvertiert eine 64-Bit-Zahl in eine Zeichenkette
LFCore_API void LFINT64ToString(const __int64 v, wchar_t* str, size_t cCount);

// Konvertiert einen Bruch in eine Zeichenkette
LFCore_API void LFFractionToString(const LFFraction frac, wchar_t* str, size_t cCount);

// Konvertiert eine Double-Zahl in eine Zeichenkette
LFCore_API void LFDoubleToString(const double d, wchar_t* str, size_t cCount);

// Konvertiert eine Koordinaten-Komponente in eine Zeichenkette
LFCore_API void LFGeoCoordinateToString(const double c, wchar_t* str, size_t cCount, bool IsLatitude, bool FillZero);

// Konvertiert eine Geo-Position in eine Zeichenkette
LFCore_API void LFGeoCoordinatesToString(const LFGeoCoordinates c, wchar_t* str, size_t cCount, bool FillZero);

// Konvertiert eine Zeit in eine Zeichenkette
LFCore_API void LFTimeToString(const FILETIME t, wchar_t* str, size_t cCount, unsigned int mask=3);

// Konvertiert eine Zeitdauer in eine Zeichenkette
LFCore_API void LFDurationToString(unsigned int d, wchar_t* str, size_t cCount);

// Konvertiert ein Attribut in eine Zeichenkette
LFCore_API void LFAttributeToString(LFItemDescriptor* i, unsigned int attr, wchar_t* str, size_t cCount);

// Konvertiert eine LFVariantData-Struktur in eine Zeichenkette
LFCore_API void LFVariantDataToString(LFVariantData* v, wchar_t* str, size_t cCount);

// Erzeugt eine neutrale LFVariantData-Struktur (Null-Element)
// v->Attr muss gesetzt sein
LFCore_API void LFGetNullVariantData(LFVariantData* v);

// Prüft, ob eine LVVariantData-Strultur Null ist
LFCore_API bool LFIsNullVariantData(LFVariantData* v);

// Prüft, ob ein Dateiattribut gleich einer LFVariantData-Struktur ist
LFCore_API bool LFIsEqualToVariantData(LFItemDescriptor* i, LFVariantData* v);

// Entfernt doppelte Eintäge in einem Unicode-Array
LFCore_API void LFSanitizeUnicodeArray(wchar_t* buf, size_t cCount);



// Neuen LFFilter erzeugen, ggf. als Kopie eines existierenden Filters
LFCore_API LFFilter* LFAllocFilter(LFFilter* f=NULL);

// Existierenden LFFilter freigeben
LFCore_API void LFFreeFilter(LFFilter* f);

// Neue LFFilterCondition erzeugen
LFCore_API LFFilterCondition* LFAllocFilterCondition();

// Existierende LFFilterCondition freigeben
LFCore_API void LFFreeFilterCondition(LFFilterCondition* c);



// Neues Suchergebnis mit Kontext ctx erzeugen
LFCore_API LFSearchResult* LFAllocSearchResult(int ctx, LFSearchResult* res=NULL);

// Existierendes LFSearchResult freigeben
LFCore_API void LFFreeSearchResult(LFSearchResult* res);

// LFItemDescriptor zum LFSearchResult hinzufügen
LFCore_API bool LFAddItemDescriptor(LFSearchResult* res, LFItemDescriptor* i);

// LFItemDescriptor aus LFSearchResult entfernen
//
// !!ACHTUNG!!
// Die Sortierreihenfolge geht verloren, LFItemDescriptor.Position wird bei
// LFSearchResults, die nicht "hohl" sind (m_RawCopy==true), jedoch angepasst.
LFCore_API void LFRemoveItemDescriptor(LFSearchResult* res, unsigned int idx);

// Alle markierten LFItemDescriptor (DeleteFlag==true) aus LFSearchResult entfernen
//
// !!ACHTUNG!!
// Die Sortierreihenfolge geht verloren, LFItemDescriptor.Position wird bei
// LFSearchResults, die nicht "hohl" sind (m_RawCopy==true), jedoch angepasst.
LFCore_API void LFRemoveFlaggedItemDescriptors(LFSearchResult* res);

// Sortiert LFSearchResult
LFCore_API void LFSortSearchResult(LFSearchResult* res, unsigned int attr, bool descending, bool categories=false);

// Gruppiert LFSearchResult
LFCore_API void LFGroupSearchResult(LFSearchResult* res, unsigned int attr, bool descending, bool categories, unsigned int icon, bool groupone, LFFilter* f);


// Neue Datei-Importliste erzeugen
LFCore_API LFFileImportList* LFAllocFileImportList();

// Existierendes LFFileImportList freigeben
LFCore_API void LFFreeFileImportList(LFFileImportList* il);

// String zur LFFileImportList hinzufügen
LFCore_API bool LFAddImportPath(LFFileImportList* il, wchar_t* path);



// Neue Transaktionsliste erzeugen
LFCore_API LFTransactionList* LFAllocTransactionList();

// Existierendes LFTransactionList freigeben
LFCore_API void LFFreeTransactionList(LFTransactionList* tl);

// LFItemDescriptor zur LFTransactionList hinzufügen
LFCore_API bool LFAddItemDescriptor(LFTransactionList* tl, LFItemDescriptor* i, unsigned int UserData=0);



// Neuen LFStoreDescriptor erzeugen
LFCore_API LFStoreDescriptor* LFAllocStoreDescriptor();

// Existierenden LFStoreDescriptor freigeben
LFCore_API void LFFreeStoreDescriptor(LFStoreDescriptor* s);



// Name einer Item-Kategorie in aktueller Sprache zurückliefern
LFCore_API wchar_t* LFGetItemCategoryName(unsigned int ID);

// Name einer Attribut-Kategorie in aktueller Sprache zurückliefern
LFCore_API wchar_t* LFGetAttrCategoryName(unsigned int ID);

// Beschreibung eines Fehlers (LFError...) in aktueller Sprache zurückliefern
LFCore_API wchar_t* LFGetErrorText(unsigned int ID);

// Anzeigen eines Fehlers (LFError...) in aktueller Sprache
LFCore_API void LFErrorBox(unsigned int ID, HWND hWnd=NULL);


// Attribut als Sortierkriterium für eine Ansicht erlaubt ?
LFCore_API bool LFAttributeSortableInView(unsigned int Attr, unsigned int ViewMode);



// Suchabfrage durchführen
// - Ist filter==NULL, so wird eine Liste aller Stores zurückgeliefert
LFCore_API LFSearchResult* LFQuery(LFFilter* filter);

// Bestehendes Suchergebnis eingrenzen
// - filter muss vom Typ LFFilterModeDirectoryTree oder LFFilterModeSearch sein
// - filter muss ein Unterverzeichnis sein
// - first und last müssen einen gültigen Bereich umfassen
LFCore_API LFSearchResult* LFQuery(LFFilter* filter, LFSearchResult* base, int first, int last);

// Gleicht eine Datei mit einem Filter ab
LFCore_API bool LFPassesFilter(LFItemDescriptor* i, LFFilter* filter);


//
// Stores
//

// Gibt den physischen Pfad einer Datei zurück
LFCore_API unsigned int LFGetFileLocation(char* StoreID, LFCoreAttributes* ca, char* dst, size_t cCount);

// Gibt den physischen Pfad einer Datei zurück
LFCore_API unsigned int LFGetFileLocation(LFItemDescriptor* i, char* dst, size_t cCount);

// Gibt die Daten eines Stores zurück
LFCore_API unsigned int LFGetStoreSettings(char* key, LFStoreDescriptor* s);
LFCore_API unsigned int LFGetStoreSettings(GUID guid, LFStoreDescriptor* s);

// Legt einen neuen Store an
// - Eingabeparameter interner Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - StoreMode: erforderlich
//   - AutoLocation: erforderlich
//   - DatPath: erforderlich, wenn AutoLocation==true
//   - Alle anderen Parameter werden ignoriert bzw. ausgefüllt
// - Eingabeparameter Hybrid-Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - StoreMode: erforderlich
//   - DatPath: erforderlich
//   - Alle anderen Parameter werden ignoriert bzw. ausgefüllt
// - Eingabeparameter externer Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - StoreMode: erforderlich
//   - DatPath: erforderlich
//   - Alle anderen Parameter werden ignoriert bzw. ausgefüllt
LFCore_API unsigned int LFCreateStore(LFStoreDescriptor* s, bool MakeDefault=false, HWND hWndSource=NULL);

// Macht den internen Store zum Default Store
LFCore_API unsigned int LFMakeDefaultStore(char* key, HWND hWndSource=NULL, bool InternalCall=false);

// Macht den externen Store zum Hybrid-Store
LFCore_API unsigned int LFMakeHybridStore(char* key, HWND hWndSource=NULL);

// Setzt Namen und Kommentar eines Stores
// Ist name oder comment NULL, so wird der jeweilige Wert nicht verändert
LFCore_API unsigned int LFSetStoreAttributes(char* key, wchar_t* name, wchar_t* comment, HWND hWndSource=NULL, bool InternalCall=false);

// Löscht einen bestehenden Store
LFCore_API unsigned int LFDeleteStore(char* key, HWND hWndSource=NULL);

// Anzeigen einer MessageBox zum Löschen des Stores in aktueller Sprache
LFCore_API bool LFAskDeleteStore(LFItemDescriptor* s, HWND hWnd=NULL);

// Anzeigen einer MessageBox zum Löschen des Stores in aktueller Sprache
LFCore_API bool LFAskDeleteStore(LFStoreDescriptor* s, HWND hWnd=NULL);

// Startet geplante Wartungsarbeiten für einen Store
LFCore_API unsigned int LFStoreMaintenance(char* key);

// Startet geplante Wartungsarbeiten für alle Store
LFCore_API unsigned int LFStoreMaintenance(unsigned int* Repaired=NULL, unsigned int* NoAccess=NULL, unsigned int* NoFreeSpace=NULL, unsigned int* RepairError=NULL);

// Gibt an, ob ein Default Stores verfügbar ist
LFCore_API bool LFDefaultStoreAvailable();

// Gibt den Key des aktuellen Default Stores zurück
LFCore_API char* LFGetDefaultStore();

// Gibt den Standardnamen des Default Stores zurück
LFCore_API void LFGetDefaultStoreName(char* name, size_t cCount);

// Gibt die Anzahl aller Stores zurück
LFCore_API unsigned int LFGetStoreCount();

// Mountet alle Hybrid-Stores und externen Stores auf Laufwerk d
LFCore_API unsigned int LFMountDrive(char d, bool InternalCall=false);

// Unmountet alle Hybrid-Stores und externen Stores auf Laufwerk d
LFCore_API unsigned int LFUnmountDrive(char d, bool InternalCall=false);

// Importiert Dateien in einen Store
LFCore_API unsigned int LFImportFiles(char* key, LFFileImportList* il, LFItemDescriptor* it=NULL, bool move=false);



//
// Transaktionen
//

// Ändert bei allen Einträgen in tl bis zu 3 Attributwerte
// hWndSource enthält das Window-Handle des auslösenden Fensters, welches bei allen globalen Nachrichten
// als LPARAM mitgeschickt wird (ggf. NULL)
LFCore_API void LFTransactionUpdate(LFTransactionList* tl, HWND hWndSource, LFVariantData* value1, LFVariantData* value2=NULL, LFVariantData* value3=NULL);

// Löscht alle Dateien in tl
LFCore_API void LFTransactionDelete(LFTransactionList* tl);


//
// Geotagging
//

// Liefert die Anzahl der Territorien zurück
LFCore_API unsigned int LFIATAGetCountryCount();

// Liefert die Anzahl der Flughäfen zurück
LFCore_API unsigned int LFIATAGetAirportCount();

// Liefert Zeiger auf Territorium zurück
LFCore_API LFCountry* LFIATAGetCountry(unsigned int ID);

// Setzt den Zeiger *pBuffer auf den nächsten Flughafen
LFCore_API int LFIATAGetNextAirport(int last, LFAirport** pBuffer);

// Setzt den Zeiger *pBuffer auf den nächsten Flughafen, der im Territorium CountryID liegt.
// *pBuffer kann in jedem Fall überschrieben werden.
LFCore_API int LFIATAGetNextAirportByCountry(unsigned int CountryID, int last, LFAirport** pBuffer);

// Setzt den Zeiger *pBuffer auf den Flughafen mit dem übergebenen Code.
// *pBuffer kann in jedem Fall überschrieben werden.
LFCore_API bool LFIATAGetAirportByCode(char* Code, LFAirport** pBuffer);

// Liefert zu einem LFItemDescriptor anhand von PreferredAttr oder ggf. LFAttrLocationIATA die Position.
// *coord kann in jedem Fall überschrieben werden.
LFCore_API bool LFGetItemCoordinates(LFItemDescriptor* i, unsigned int PreferredAttr, LFGeoCoordinates* coord);

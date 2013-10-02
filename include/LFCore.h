// Folgender ifdef-Block ist die Standardmethode zum Erstellen von Makros, die das Exportieren 
// aus einer DLL vereinfachen. Alle Dateien in dieser DLL werden mit dem LFCore_EXPORTS-Symbol
// kompiliert, das in der Befehlszeile definiert wurde. Das Symbol darf nicht f�r ein Projekt definiert werden,
// das diese DLL verwendet. Alle anderen Projekte, deren Quelldateien diese Datei beinhalten, erkennen 
// LFCore_API-Funktionen als aus einer DLL importiert, w�hrend die DLL
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
// Kernfunktionalit�t
//

#define LFGLD_Internal            1
#define LFGLD_External            2
#define LFGLD_Both                3
#define LFGLD_Network             4
#define LFGLD_IncludeFloppies     8
#define LFGLD_All                 15



// liquidFOLDERS initalisieren
LFCore_API void __stdcall LFInitialize();



// Gibt true zur�ck, wenn diese Installation freigeschaltet ist
// Die gespeicherten Lizenzinformationen finden sich in License
LFCore_API bool __stdcall LFIsLicensed(LFLicense* License=NULL, bool Reload=false);

// Gibt true zur�ck, wenn die Shareware-Version ausgelaufen ist,
// und keine ordnungsgem��e Lizenz vorliegt
LFCore_API bool __stdcall LFIsSharewareExpired();



// Gibt true zur�ck, wenn der Explorer Dateiendungen verbirgt
LFCore_API bool __stdcall LFHideFileExt();

// Gibt true zur�ck, wenn der Explorer leere Laufwerke verbirgt
LFCore_API bool __stdcall LFHideDrivesWithNoMedia();



// Liefert den Source-Typ eines Laufwerks zur�ck
LFCore_API unsigned int __stdcall LFGetSourceForDrive(char cDrive);

// Wie Win32-Funktion GetLogicalDrives(), allerdings selektiv (s.o.)
LFCore_API unsigned int __stdcall LFGetLogicalDrives(unsigned int mask=LFGLD_Both);

// Gibt einen Zeiger auf die IDs aller registrierten Nachrichten zur�ck
LFCore_API LFMessageIDs* __stdcall LFGetMessageIDs();

// Erzeugt einen Link mit DropHandler zur Explorer-Erweiterung
// im SendTo-Ordner des Benutzers
// Wenn force==false wird der Link nur beim ersten Aufruf erzeugt
LFCore_API void __stdcall LFCreateSendTo(bool force=false);

// Gibt ein Icon dieser DLL zur�ck
LFCore_API HICON __stdcall LFGetIcon(unsigned int ResID, int cx, int cy);

// Initalisiert eine LFProgress-Datenstruktur
LFCore_API void __stdcall LFInitProgress(LFProgress* pProgress, HWND hWnd, unsigned int MajorCount=0);


// Neuen LFAttributeDescriptor erzeugen
LFCore_API LFAttributeDescriptor* __stdcall LFAllocAttributeDescriptor();

// Informationen �ber ein Attribut zur�ckliefern
LFCore_API LFAttributeDescriptor* __stdcall LFGetAttributeInfo(unsigned int ID);

// Existierenden LFAttributeDescriptor freigeben
LFCore_API void __stdcall LFFreeAttributeDescriptor(LFAttributeDescriptor* a);



// Neuen LFItemCategoryDescriptor erzeugen
LFCore_API LFItemCategoryDescriptor* __stdcall LFAllocItemCategoryDescriptor();

// Informationen �ber eine Kategorie zur�ckliefern
LFCore_API LFItemCategoryDescriptor* __stdcall LFGetItemCategoryInfo(unsigned int ID);

// Existierenden LFItemCategoryDescriptor freigeben
LFCore_API void __stdcall LFFreeItemCategoryDescriptor(LFItemCategoryDescriptor* c);



// Neuen LFContextDescriptor erzeugen
LFCore_API LFContextDescriptor* __stdcall LFAllocContextDescriptor();

// Informationen �ber ein Attribut zur�ckliefern
LFCore_API LFContextDescriptor* __stdcall LFGetContextInfo(unsigned int ID);

// Existierenden LFContextDescriptor freigeben
LFCore_API void __stdcall LFFreeContextDescriptor(LFContextDescriptor* c);



// Neuen LFItemDescriptor erzeugen und zur�cksetzen
// Ggf. wird eine unabh�ngige Kopie von i erzeugt
LFCore_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(LFItemDescriptor* i=NULL);

// Neuen LFItemDescriptor erzeugen und die Kern-Attribute belegen
LFCore_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(LFCoreAttributes* attr);

// Neuen LFItemDescriptor erzeugen und den LFStoreDescriptor konvertieren
LFCore_API LFItemDescriptor* __stdcall LFAllocItemDescriptor(LFStoreDescriptor* s);

// Existierenden LFItemDescriptor freigeben
LFCore_API void __stdcall LFFreeItemDescriptor(LFItemDescriptor* i);

// Attributwert holen
LFCore_API void __stdcall LFGetAttributeVariantData(LFItemDescriptor* i, LFVariantData* v);

// Attributwert setzen
LFCore_API void __stdcall LFSetAttributeVariantData(LFItemDescriptor* i, LFVariantData* v);



// Konvertiert einen FourCC in eine Zeichenkette
LFCore_API void __stdcall LFFourCCToString(const unsigned int c, wchar_t* str, size_t cCount);

// Konvertiert eine 32-Bit-Zahl in eine Zeichenkette
LFCore_API void __stdcall LFUINTToString(const unsigned int v, wchar_t* str, size_t cCount);

// Konvertiert eine 64-Bit-Zahl in eine Zeichenkette
LFCore_API void __stdcall LFINT64ToString(const __int64 v, wchar_t* str, size_t cCount);

// Konvertiert einen Bruch in eine Zeichenkette
LFCore_API void __stdcall LFFractionToString(const LFFraction frac, wchar_t* str, size_t cCount);

// Konvertiert eine Double-Zahl in eine Zeichenkette
LFCore_API void __stdcall LFDoubleToString(const double d, wchar_t* str, size_t cCount);

// Konvertiert eine Koordinaten-Komponente in eine Zeichenkette
LFCore_API void __stdcall LFGeoCoordinateToString(const double c, wchar_t* str, size_t cCount, bool IsLatitude, bool FillZero);

// Konvertiert eine Geo-Position in eine Zeichenkette
LFCore_API void __stdcall LFGeoCoordinatesToString(const LFGeoCoordinates c, wchar_t* str, size_t cCount, bool FillZero);

// Konvertiert eine Zeit in eine Zeichenkette
LFCore_API void __stdcall LFTimeToString(const FILETIME t, wchar_t* str, size_t cCount, unsigned int mask=3);

// Konvertiert eine Zeitdauer in eine Zeichenkette
LFCore_API void __stdcall LFDurationToString(unsigned int d, wchar_t* str, size_t cCount);

// Konvertiert eine Bitrate in eine Zeichenkette
LFCore_API void __stdcall LFBitrateToString(const unsigned int r, wchar_t* str, size_t cCount);

// Konvertiert eine Megapixel-Angabe in eine Zeichenkette
LFCore_API void __stdcall LFMegapixelToString(const double d, wchar_t* str, size_t cCount);

// Konvertiert ein Attribut in eine Zeichenkette
LFCore_API void __stdcall LFAttributeToString(LFItemDescriptor* i, unsigned int attr, wchar_t* str, size_t cCount);

// Konvertiert eine LFVariantData-Struktur in eine Zeichenkette
LFCore_API void __stdcall LFVariantDataToString(LFVariantData* v, wchar_t* str, size_t cCount);

// Erzeugt eine LFVariantData-Struktur aus einer Zeichenkette
LFCore_API void __stdcall LFVariantDataFromString(LFVariantData* v, wchar_t* str);

// Erzeugt eine neutrale LFVariantData-Struktur (Null-Element)
// v->Attr muss gesetzt sein
LFCore_API void __stdcall LFGetNullVariantData(LFVariantData* v);

// Pr�ft, ob eine LVVariantData-Struktur Null ist
LFCore_API bool __stdcall LFIsNullVariantData(LFVariantData* v);

// Pr�ft, ob ein Dateiattribut gleich einer LFVariantData-Struktur ist
LFCore_API bool __stdcall LFIsEqualToVariantData(LFItemDescriptor* i, LFVariantData* v);

// Entfernt doppelte Eint�ge in einem Unicode-Array
LFCore_API void __stdcall LFSanitizeUnicodeArray(wchar_t* buf, size_t cCount);



// Neuen LFFilter erzeugen, ggf. als Kopie eines existierenden Filters
LFCore_API LFFilter* __stdcall LFAllocFilter(LFFilter* f=NULL);

// Existierenden LFFilter freigeben
LFCore_API void __stdcall LFFreeFilter(LFFilter* f);

// Neue LFFilterCondition erzeugen
LFCore_API LFFilterCondition* __stdcall LFAllocFilterCondition();

// Existierende LFFilterCondition freigeben
LFCore_API void __stdcall LFFreeFilterCondition(LFFilterCondition* c);



// Neues Suchergebnis mit Kontext ctx erzeugen
LFCore_API LFSearchResult* __stdcall LFAllocSearchResult(int ctx, LFSearchResult* res=NULL);

// Existierendes LFSearchResult freigeben
LFCore_API void __stdcall LFFreeSearchResult(LFSearchResult* res);

// LFItemDescriptor zum LFSearchResult hinzuf�gen
LFCore_API bool __stdcall LFAddItemDescriptor(LFSearchResult* res, LFItemDescriptor* i);

// LFItemDescriptor aus LFSearchResult entfernen
//
// !!ACHTUNG!!
// Die Sortierreihenfolge geht verloren!
LFCore_API void __stdcall LFRemoveItemDescriptor(LFSearchResult* res, unsigned int idx);

// Alle markierten LFItemDescriptor (DeleteFlag==true) aus LFSearchResult entfernen
//
// !!ACHTUNG!!
// Die Sortierreihenfolge geht verloren!
LFCore_API void __stdcall LFRemoveFlaggedItemDescriptors(LFSearchResult* res);

// Sortiert LFSearchResult
LFCore_API void __stdcall LFSortSearchResult(LFSearchResult* res, unsigned int attr, bool descending);

// Gruppiert LFSearchResult und liefert Kopie zur�ck
LFCore_API LFSearchResult* __stdcall LFGroupSearchResult(LFSearchResult* res, unsigned int attr, bool descending, bool groupone, LFFilter* f);



// Neue Dateiliste erzeugen
LFCore_API LFFileIDList* __stdcall LFAllocFileIDList();
LFCore_API LFFileIDList* __stdcall LFAllocFileIDList(HLIQUID hLiquid);

// Existierende LFFileIDList freigeben
LFCore_API void __stdcall LFFreeFileIDList(LFFileIDList* il);

// String zur LFFileIDList hinzuf�gen
LFCore_API bool __stdcall LFAddFileID(LFFileIDList* il, char* StoreID, char* FileID, void* UserData=NULL);

// Handle zu LIQUIDFILES-Struktur von Dateiliste auf globalem Heap erzeugen
LFCore_API HGLOBAL __stdcall LFCreateLiquidFiles(LFFileIDList* il);



// Neue Datei-Importliste erzeugen
LFCore_API LFFileImportList* __stdcall LFAllocFileImportList();
LFCore_API LFFileImportList* __stdcall LFAllocFileImportList(HDROP hDrop);

// Existierende LFFileImportList freigeben
LFCore_API void __stdcall LFFreeFileImportList(LFFileImportList* il);

// String zur LFFileImportList hinzuf�gen
LFCore_API bool __stdcall LFAddImportPath(LFFileImportList* il, wchar_t* path);



// Neue LFMaintenanceList erzeugen
LFCore_API LFMaintenanceList* __stdcall LFAllocMaintenanceList();

// Existierende LFMaintenanceList freigeben
LFCore_API void __stdcall LFFreeMaintenanceList(LFMaintenanceList* ml);



// Neue Transaktionsliste erzeugen
LFCore_API LFTransactionList* __stdcall LFAllocTransactionList();

// Existierende LFTransactionList freigeben
LFCore_API void __stdcall LFFreeTransactionList(LFTransactionList* tl);

// LFItemDescriptor zur LFTransactionList hinzuf�gen
LFCore_API bool __stdcall LFAddItemDescriptor(LFTransactionList* tl, LFItemDescriptor* i, unsigned int UserData=0);

// PIDL von Transaktionsliste l�sen
LFCore_API LPITEMIDLIST __stdcall LFDetachPIDL(LFTransactionList* tl, unsigned int idx);

// Handle zu DROPFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCore_API HGLOBAL __stdcall LFCreateDropFiles(LFTransactionList* tl);

// Handle zu LIQUIDFILES-Struktur aus Transaktionsliste auf globalem Heap erzeugen
LFCore_API HGLOBAL __stdcall LFCreateLiquidFiles(LFTransactionList* tl);



// Name einer Item-Kategorie in aktueller Sprache zur�ckliefern
LFCore_API wchar_t* __stdcall LFGetItemCategoryName(unsigned int ID);

// Name einer Attribut-Kategorie in aktueller Sprache zur�ckliefern
LFCore_API wchar_t* __stdcall LFGetAttrCategoryName(unsigned int ID);

// Name einer Datenquelle in aktueller Sprache zur�ckliefern
LFCore_API wchar_t* __stdcall LFGetSourceName(unsigned int ID, bool qualified);

// Beschreibung eines Fehlers (LFError...) in aktueller Sprache zur�ckliefern
LFCore_API wchar_t* __stdcall LFGetErrorText(unsigned int ID);

// Anzeigen eines Fehlers (LFError...) in aktueller Sprache
LFCore_API void __stdcall LFErrorBox(unsigned int ID, HWND hWnd=NULL);



// Suchabfrage durchf�hren
// - Ist filter==NULL, so wird eine Liste aller Stores zur�ckgeliefert
LFCore_API LFSearchResult* __stdcall LFQuery(LFFilter* filter);

// Bestehendes Suchergebnis eingrenzen
// - filter muss vom Typ LFFilterModeDirectoryTree oder LFFilterModeSearch sein
// - filter muss ein Unterverzeichnis sein
// - first und last m�ssen einen g�ltigen Bereich umfassen
LFCore_API LFSearchResult* __stdcall LFQuery(LFFilter* filter, LFSearchResult* base, int first, int last);

// Gleicht eine Datei mit einem Filter ab
LFCore_API bool __stdcall LFPassesFilter(LFItemDescriptor* i, LFFilter* filter);


//
// Stores
//

// Gibt den physischen Pfad einer Datei zur�ck
LFCore_API unsigned int __stdcall LFGetFileLocation(LFItemDescriptor* i, wchar_t* dst, size_t cCount, bool CheckExists, bool RemoveNew, bool Extended=false);

// Gibt die Daten eines Stores zur�ck
LFCore_API unsigned int __stdcall LFGetStoreSettings(char* key, LFStoreDescriptor* s);
LFCore_API unsigned int __stdcall LFGetStoreSettings(GUID guid, LFStoreDescriptor* s);

// Gibt die ID f�r das Icon eines Stores zur�ck
LFCore_API unsigned int __stdcall LFGetStoreIcon(LFStoreDescriptor* s);

// Pr�ft, ob ein Store angeschlossen ist
LFCore_API bool __stdcall LFIsStoreMounted(LFStoreDescriptor* s);

// Legt einen neuen Store an
// - Eingabeparameter interner Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - StoreMode: erforderlich
//   - AutoLocation: erforderlich
//   - DatPath: erforderlich, wenn AutoLocation==true
//   - Alle anderen Parameter werden ignoriert bzw. ausgef�llt
// - Eingabeparameter Hybrid-Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - StoreMode: erforderlich
//   - DatPath: erforderlich
//   - Alle anderen Parameter werden ignoriert bzw. ausgef�llt
// - Eingabeparameter externer Store:
//   - StoreName: optional (wird ggf. durch Standardname ersetzt)
//   - Comment: optional
//   - StoreMode: erforderlich
//   - DatPath: erforderlich
//   - Alle anderen Parameter werden ignoriert bzw. ausgef�llt
LFCore_API unsigned int __stdcall LFCreateStore(LFStoreDescriptor* s, bool MakeDefault=false, HWND hWndSource=NULL);

// Macht den internen Store zum Default Store
LFCore_API unsigned int __stdcall LFMakeDefaultStore(char* key, HWND hWndSource=NULL, bool InternalCall=false);

// Macht den externen Store zum Hybrid-Store
LFCore_API unsigned int __stdcall LFMakeStoreSearchable(char* key, bool Searchable=true, HWND hWndSource=NULL);

// Setzt Namen und Kommentar eines Stores
// Ist name oder comment NULL, so wird der jeweilige Wert nicht ver�ndert
LFCore_API unsigned int __stdcall LFSetStoreAttributes(char* key, wchar_t* name, wchar_t* comment, HWND hWndSource=NULL, bool InternalCall=false);

// L�scht einen bestehenden Store
LFCore_API unsigned int __stdcall LFDeleteStore(char* key, HWND hWndSource=NULL, LFProgress* pProgress=NULL);

// Anzeigen einer MessageBox zum L�schen des Stores in aktueller Sprache
LFCore_API bool __stdcall LFAskDeleteStore(LFItemDescriptor* s, HWND hWnd=NULL);

// Anzeigen einer MessageBox zum L�schen des Stores in aktueller Sprache
LFCore_API bool __stdcall LFAskDeleteStore(LFStoreDescriptor* s, HWND hWnd=NULL);

// Startet geplante Wartungsarbeiten f�r alle Stores
LFCore_API LFMaintenanceList* __stdcall LFStoreMaintenance(HWND hWndSource=NULL, LFProgress* pProgress=NULL);

// Gibt an, ob ein Default Stores verf�gbar ist
LFCore_API bool __stdcall LFDefaultStoreAvailable();

// Gibt den Key des aktuellen Default Stores zur�ck
LFCore_API char* __stdcall LFGetDefaultStore();

// Gibt den Standardnamen des Default Stores zur�ck
LFCore_API void __stdcall LFGetDefaultStoreName(wchar_t* name, size_t cCount);

// Gibt die Anzahl aller Stores zur�ck
LFCore_API unsigned int __stdcall LFGetStoreCount();

// Pr�ft, ob Stores auf dem angegebenen Laufwerk vorhanden sind
LFCore_API bool __stdcall LFStoresOnVolume(char d);

// Gibt die IDs aller Stores zur�ck
LFCore_API unsigned int __stdcall LFGetStores(char** keys, unsigned int* count);



//
// Filter
//

// Erzeugt eine neue Filterdatei in einem Store
LFCore_API unsigned int __stdcall LFSaveFilter(char* key, LFFilter* filter, wchar_t* name, wchar_t* comments=NULL, LFItemDescriptor** created=NULL);

// L�dt einen abgespeicherten Filter
LFCore_API LFFilter* __stdcall LFLoadFilter(wchar_t* fn);
LFCore_API LFFilter* __stdcall LFLoadFilter(LFItemDescriptor* i);



//
// Transaktionen
//

// Benennt die Datei um
LFCore_API unsigned int __stdcall LFTransactionRename(char* StoreID, char* FileID, wchar_t* NewName);


// LFFileImportList
//

// Importiert Dateien in den Store
LFCore_API void __stdcall LFTransactionImport(char* key, LFFileImportList* il, LFItemDescriptor* it, bool recursive, bool move, LFProgress* pProgress=NULL);


// LFTransactionList
//

// �ndert bei allen Eintr�gen in tl bis zu 3 Attributwerte
// hWndSource enth�lt das Window-Handle des ausl�senden Fensters, welches bei allen globalen Nachrichten
// als LPARAM mitgeschickt wird (ggf. NULL)
LFCore_API void __stdcall LFTransactionUpdate(LFTransactionList* tl, HWND hWndSource, LFVariantData* value1, LFVariantData* value2=NULL, LFVariantData* value3=NULL);

// L�scht alle Dateien in tl
LFCore_API void __stdcall LFTransactionDelete(LFTransactionList* tl, bool PutInTrash=true, LFProgress* pProgress=NULL);

// Stellt alle Dateien in tl wieder her, sofern nicht entg�ltig gel�scht
LFCore_API void __stdcall LFTransactionRestore(LFTransactionList* tl);

// Physische Orte aufl�sen
LFCore_API void __stdcall LFTransactionResolvePhysicalLocations(LFTransactionList* tl, bool IncludePIDL=false);


// LFFileIDList
//

// Importiert Dateien in den Store
LFCore_API void __stdcall LFTransactionImport(char* key, LFFileIDList* il, bool move, LFProgress* pProgress=NULL);

// L�scht alle Dateien in il
LFCore_API void __stdcall LFTransactionDelete(LFFileIDList* il, bool PutInTrash=true, LFProgress* pProgress=NULL);

// F�gt die angegebenen Dateien zum Suchergebnis hinzu
LFCore_API void __stdcall LFTransactionAddToSearchResult(LFFileIDList* il, LFSearchResult* res);


//
// Geotagging
//

// Liefert die Anzahl der Territorien zur�ck
LFCore_API unsigned int __stdcall LFIATAGetCountryCount();

// Liefert die Anzahl der Flugh�fen zur�ck
LFCore_API unsigned int __stdcall LFIATAGetAirportCount();

// Liefert Zeiger auf Territorium zur�ck
LFCore_API LFCountry* __stdcall LFIATAGetCountry(unsigned int ID);

// Setzt den Zeiger *pBuffer auf den n�chsten Flughafen
LFCore_API int __stdcall LFIATAGetNextAirport(int last, LFAirport** pBuffer);

// Setzt den Zeiger *pBuffer auf den n�chsten Flughafen, der im Territorium CountryID liegt.
// *pBuffer kann in jedem Fall �berschrieben werden.
LFCore_API int __stdcall LFIATAGetNextAirportByCountry(unsigned int CountryID, int last, LFAirport** pBuffer);

// Setzt den Zeiger *pBuffer auf den Flughafen mit dem �bergebenen Code.
// *pBuffer kann in jedem Fall �berschrieben werden.
LFCore_API bool __stdcall LFIATAGetAirportByCode(char* Code, LFAirport** pBuffer);


//
// Shortcuts
//

// Fragt unter Windows XP, ob eine Verkn�pfung auf dem Desktop erstellt werden soll
// Bei h�heren Windows-Versionen wird immer true zur�ckgeliefert.
LFCore_API bool __stdcall LFAskCreateShortcut(HWND hwnd);

// Speichert pShellLink auf dem Desktop ab
LFCore_API void __stdcall LFCreateDesktopShortcut(IShellLink* pShellLink, wchar_t* LinkFilename);

// Liefert einen ShellLink f�r den angegebenen Store
LFCore_API IShellLink* __stdcall LFGetShortcutForStore(LFItemDescriptor* i);
LFCore_API IShellLink* __stdcall LFGetShortcutForStore(LFStoreDescriptor* s);
LFCore_API IShellLink* __stdcall LFGetShortcutForStore(char* key);

// Erzeugt auf dem Desktop eine Verkn�pfung mit dem angegebenen Store
LFCore_API void __stdcall LFCreateDesktopShortcutForStore(LFItemDescriptor* i);
LFCore_API void __stdcall LFCreateDesktopShortcutForStore(LFStoreDescriptor* s);
LFCore_API void __stdcall LFCreateDesktopShortcutForStore(char* key);


//
// Thumbnails
//

LFCore_API HBITMAP __stdcall LFGetThumbnail(LFItemDescriptor* i);
LFCore_API HBITMAP __stdcall LFGetThumbnail(LFItemDescriptor* i, SIZE sz);

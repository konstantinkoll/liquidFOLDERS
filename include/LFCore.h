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


//
// Kernfunktionalit�t
//

#define LFGLD_Internal                   1
#define LFGLD_External                   2
#define LFGLD_Both                       3
#define LFGLD_Network                    4
#define LFGLD_All                        7

// Wie Win32-Funktion GetLogicalDrives(), allerdings selektiv (s.o.)
LFCore_API unsigned int LFGetLogicalDrives(unsigned int mask=LFGLD_Both);

// Gibt einen Zeiger auf die IDs aller registrierten Nachrichten zur�ck
LFCore_API LFMessageIDs* LFGetMessageIDs();

// Gibt true zur�ck, wenn diese Installation freigeschaltet ist
LFCore_API bool LFIsLicensed();



// Neuen LFAttributeDescriptor erzeugen
LFCore_API LFAttributeDescriptor* LFAllocAttributeDescriptor();

// Informationen �ber ein Attribut zur�ckliefern
LFCore_API LFAttributeDescriptor* LFGetAttributeInfo(unsigned int ID);

// Existierenden LFAttributeDescriptor freigeben
LFCore_API void LFFreeAttributeDescriptor(LFAttributeDescriptor* a);



// Neuen LFItemCategoryDescriptor erzeugen
LFCore_API LFItemCategoryDescriptor* LFAllocItemCategoryDescriptor();

// Informationen �ber eine Kategorie zur�ckliefern
LFCore_API LFItemCategoryDescriptor* LFGetItemCategoryInfo(unsigned int ID);

// Existierenden LFItemCategoryDescriptor freigeben
LFCore_API void LFFreeItemCategoryDescriptor(LFItemCategoryDescriptor* c);



// Neuen LFContextDescriptor erzeugen
LFCore_API LFContextDescriptor* LFAllocContextDescriptor();

// Informationen �ber ein Attribut zur�ckliefern
LFCore_API LFContextDescriptor* LFGetContextInfo(unsigned int ID);

// Existierenden LFContextDescriptor freigeben
LFCore_API void LFFreeContextDescriptor(LFContextDescriptor* c);



// Neuen LFDomainDescriptor erzeugen
LFCore_API LFDomainDescriptor* LFAllocDomainDescriptor();

// Informationen �ber eine Domain zur�ckliefern
LFCore_API LFDomainDescriptor* LFGetDomainInfo(unsigned int ID);

// Existierenden LFDomainDescriptor freigeben
LFCore_API void LFFreeDomainDescriptor(LFDomainDescriptor* f);



// Neuen LFItemDescriptor erzeugen und zur�cksetzen
LFCore_API LFItemDescriptor* LFAllocItemDescriptor();

// Neuen LFItemDescriptor als Kopie erzeugen
LFCore_API LFItemDescriptor* LFAllocItemDescriptor(LFItemDescriptor* i);

// Existierenden LFItemDescriptor freigeben
LFCore_API void LFFreeItemDescriptor(LFItemDescriptor* i);

// Attributwert holen
LFCore_API void LFGetAttributeVariantData(LFItemDescriptor* i, LFVariantData* v);

// Attributwert setzen
LFCore_API void LFSetAttributeVariantData(LFItemDescriptor* i, LFVariantData* v, wchar_t* ustr=NULL);



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
LFCore_API void LFGeoCoordinateToString(const double c, wchar_t* str, size_t cCount, bool IsLatitude);

// Konvertiert eine Geo-Position in eine Zeichenkette
LFCore_API void LFGeoCoordinatesToString(const LFGeoCoordinates c, wchar_t* str, size_t cCount);

// Konvertiert eine Zeit in eine Zeichenkette
LFCore_API void LFTimeToString(const FILETIME t, wchar_t* str, size_t cCount, unsigned int mask=3);

// Konvertiert eine Zeitdauer in eine Zeichenkette
LFCore_API void LFDurationToString(unsigned int d, wchar_t* str, size_t cCount);



// Erzeugt eine neutrale LFVariantData-Struktur (Null-Element)
// v->Attr muss gesetzt sein
LFCore_API void LFGetNullVariantData(LFVariantData* v);

// Pr�ft, ob ein Dateiattribut gleich einer LFVariantData-Struktur ist
LFCore_API bool LFIsEqualToVariantData(LFItemDescriptor* i, LFVariantData* v);



// Neuen LFFilter erzeugen, ggf. als Kopie eines existierenden Filters
LFCore_API LFFilter* LFAllocFilter(LFFilter* f=NULL);

// Existierenden LFFilter freigeben
LFCore_API void LFFreeFilter(LFFilter* f);



// Neues Suchergebnis mit Kontext ctx erzeugen
LFCore_API LFSearchResult* LFAllocSearchResult(int ctx, LFSearchResult* res=NULL);

// Existierendes LFSearchResult freigeben
LFCore_API void LFFreeSearchResult(LFSearchResult* res);

// LFItemDescriptor zum LFSearchResult hinzuf�gen
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



// Neue Transaktionsliste erzeugen
LFCore_API LFTransactionList* LFAllocTransactionList();

// Existierendes LFTransactionList freigeben
LFCore_API void LFFreeTransactionList(LFTransactionList* tl);

// LFItemDescriptor zur LFTransactionList hinzuf�gen
LFCore_API bool LFAddItemDescriptor(LFTransactionList* tl, LFItemDescriptor* i, unsigned int UserData=0);

// Eintrag aus LFTransactionList entfernen
//
// !!ACHTUNG!!
// Die Sortierreihenfolge geht verloren
LFCore_API void LFRemoveEntry(LFTransactionList* tl, unsigned idx);

// Alle markierten Eintr�ge (Item.DeleteFlag==true) aus LFTransactionList entfernen
//
// !!ACHTUNG!!
// Die Sortierreihenfolge geht verloren
LFCore_API void LFRemoveFlaggedEntries(LFTransactionList* tl);

// Alle Eintr�ge mit Fehler (Item.LastError!=LFOk) aus LFTransactionList entfernen
//
// !!ACHTUNG!!
// Die Sortierreihenfolge geht verloren
LFCore_API void LFRemoveErrorEntries(LFTransactionList* tl);



// Neuen LFStoreDescriptor erzeugen
LFCore_API LFStoreDescriptor* LFAllocStoreDescriptor();

// Existierenden LFStoreDescriptor freigeben
LFCore_API void LFFreeStoreDescriptor(LFStoreDescriptor* s);



// Name einer Item-Kategorie in aktueller Sprache zur�ckliefern
LFCore_API wchar_t* LFGetItemCategoryName(unsigned int ID);

// Name einer Attribut-Kategorie in aktueller Sprache zur�ckliefern
LFCore_API wchar_t* LFGetAttrCategoryName(unsigned int ID);

// Beschreibung eines Fehlers (LFError...) in aktueller Sprache zur�ckliefern
LFCore_API wchar_t* LFGetErrorText(unsigned int ID);

// Anzeigen eines Fehlers (LFError...) in aktueller Sprache
LFCore_API void LFErrorBox(unsigned int ID, HWND hWnd=NULL);


// Attribut als Sortierkriterium f�r eine Ansicht erlaubt ?
LFCore_API bool LFAttributeSortableInView(unsigned int Attr, unsigned int ViewMode);



// Suchabfrage durchf�hren
// - Ist filter==NULL, so wird eine Liste aller Stores zur�ckgeliefert
LFCore_API LFSearchResult* LFQuery(LFFilter* filter);

// Gleicht eine Datei mit einem Filter ab
LFCore_API bool LFPassesFilter(LFItemDescriptor* i, LFFilter* filter);


//
// Stores
//

// Gibt die Daten eines Stores zur�ck
LFCore_API unsigned int LFGetStoreSettings(char* key, LFStoreDescriptor* s);
LFCore_API unsigned int LFGetStoreSettings(GUID guid, LFStoreDescriptor* s);

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
LFCore_API unsigned int LFCreateStore(LFStoreDescriptor* s, bool MakeDefault=false, HWND hWndSource=NULL);

// Macht den internen Store zum Default Store
LFCore_API unsigned int LFMakeDefaultStore(char* key, HWND hWndSource=NULL, bool InternalCall=false);

// Macht den externen Store zum Hybrid-Store
LFCore_API unsigned int LFMakeHybridStore(char* key, HWND hWndSource=NULL);

// Setzt Namen und Kommentar eines Stores
// Ist name oder comment NULL, so wird der jeweilige Wert nicht ver�ndert
LFCore_API unsigned int LFSetStoreAttributes(char* key, wchar_t* name, wchar_t* comment, HWND hWndSource=NULL, bool InternalCall=false);

// L�scht einen bestehenden Store
LFCore_API unsigned int LFDeleteStore(char* key, HWND hWndSource=NULL);

// Anzeigen einer MessageBox zum L�schen des Stores in aktueller Sprache
LFCore_API bool LFAskDeleteStore(LFItemDescriptor* s, HWND hWnd=NULL);

// Anzeigen einer MessageBox zum L�schen des Stores in aktueller Sprache
LFCore_API bool LFAskDeleteStore(LFStoreDescriptor* s, HWND hWnd=NULL);

// Startet geplante Wartungsarbeiten f�r einen Store
LFCore_API unsigned int LFStoreMaintenance(char* key);

// Startet geplante Wartungsarbeiten f�r alle Store
LFCore_API unsigned int LFStoreMaintenance();

// Gibt an, ob ein Default Stores verf�gbar ist
LFCore_API bool LFDefaultStoreAvailable();

// Gibt den Key des Default Stores zur�ck
LFCore_API char* LFGetDefaultStore();

// Gibt die Anzahl aller Stores zur�ck
LFCore_API unsigned int LFGetStoreCount();

// Mountet alle Hybrid-Stores und externen Stores auf Laufwerk d
LFCore_API unsigned int LFMountDrive(char d);

// Unmountet alle Hybrid-Stores und externen Stores auf Laufwerk d
LFCore_API unsigned int LFUnmountDrive(char d);



//
// Transaktionen
//

// �ndert bei allen Eintr�gen in tl bis zu 3 Attributwerte
// hWndSource enth�lt das Window-Handle des ausl�senden Fensters, welches bei allen globalen Nachrichten
// als LPARAM mitgeschickt wird (ggf. NULL)
LFCore_API void LFTransactionUpdate(LFTransactionList* tl, HWND hWndSource, LFVariantData* value1, wchar_t* ustr1=NULL, LFVariantData* value2=NULL, wchar_t* ustr2=NULL, LFVariantData* value3=NULL, wchar_t* ustr3=NULL);



//
// Geotagging
//

// Liefert die Anzahl der Territorien zur�ck
LFCore_API unsigned int LFIATAGetCountryCount();

// Liefert die Anzahl der Flugh�fen zur�ck
LFCore_API unsigned int LFIATAGetAirportCount();

// Liefert Zeiger auf Territorium zur�ck
LFCore_API LFCountry* LFIATAGetCountry(unsigned int ID);

// Setzt den Zeiger *pBuffer auf den n�chsten Flughafen
LFCore_API int LFIATAGetNextAirport(int last, LFAirport** pBuffer);

// Setzt den Zeiger *pBuffer auf den n�chsten Flughafen, der im Territorium CountryID liegt.
// *pBuffer kann in jedem Fall �berschrieben werden.
LFCore_API int LFIATAGetNextAirportByCountry(unsigned int CountryID, int last, LFAirport** pBuffer);

// Setzt den Zeiger *pBuffer auf den Flughafen mit dem �bergebenen Code.
// *pBuffer kann in jedem Fall �berschrieben werden.
LFCore_API bool LFIATAGetAirportByCode(char* Code, LFAirport** pBuffer);

// Erzeugt aus einem Flughafen einen virtuellen Ordner
LFCore_API LFItemDescriptor* LFIATACreateFolderForAirport(LFAirport* airport);

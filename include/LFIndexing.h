// Folgender ifdef-Block ist die Standardmethode zum Erstellen von Makros, die das Exportieren 
// aus einer DLL vereinfachen. Alle Dateien in dieser DLL werden mit dem LFIndexing_EXPORTS-Symbol
// kompiliert, das in der Befehlszeile definiert wurde. Das Symbol darf nicht für ein Projekt definiert werden,
// das diese DLL verwendet. Alle anderen Projekte, deren Quelldateien diese Datei beinhalten, erkennen 
// LFIndexing_API-Funktionen als aus einer DLL importiert, während die DLL
// mit diesem Makro definierte Symbole als exportiert ansieht.
#ifdef LFIndexing_EXPORTS
#define LFIndexing_API __declspec(dllexport)
#else
#define LFIndexing_API __declspec(dllimport)
#endif

#include <assert.h>
#include "BitArray.h"
#include "..\\LFIndexing\\CHeapfile.h"

// dllmain.cpp : Definiert die Initialisierungsroutinen für die DLL.
//

#include "stdafx.h"
#include "LFCore.h"
#include <afxwin.h>
#include <afxdllx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

AFX_EXTENSION_MODULE LFCommDlgDLL = { NULL, NULL };
LFMessageIDs* MessageIDs = LFGetMessageIDs();

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Entfernen Sie dies, wenn Sie lpReserved verwenden.
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// Einmalige Initialisierung der Erweiterungs-DLL
		if (!AfxInitExtensionModule(LFCommDlgDLL, hInstance))
			return 0;

		// Diese DLL in Ressourcenkette einfügen
		// HINWEIS: Wird diese Erweiterungs-DLL implizit durch
		//  eine reguläre MFC-DLL (z.B. ein ActiveX-Steuerelement)
		//  anstelle einer MFC-Anwendung verknüpft, dann sollten Sie
		//  folgende Zeilen aus DllMain entfernen, und diese in eine separate
		//  Funktion einfügen, die aus der Erweiterungs-DLL exportiert wurde. Die reguläre DLL,
		//  die diese Erweiterungs-DLL verwendet, sollte dann explizit die
		//  Funktion aufrufen, um die Erweiterungs-DLL zu initialisieren. Andernfalls
		//  wird das CDynLinkLibrary-Objekt nicht mit der Ressourcenkette der
		//  regulären DLL verbunden, was zu ernsthaften Problemen
		//  führen kann.

		new CDynLinkLibrary(LFCommDlgDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		// Bibliothek vor dem Aufruf der Destruktoren schließen.
		AfxTermExtensionModule(LFCommDlgDLL);
	}
	return 1;   // OK
}

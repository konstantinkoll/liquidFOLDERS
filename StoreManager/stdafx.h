
// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete, projektspezifische Includedateien,
// die nur selten geändert werden.

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Selten verwendete Teile der Windows-Header ausschließen
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // einige CString-Konstruktoren sind explizit

// Deaktiviert das Ausblenden einiger häufiger und oft ignorierter Warnungen durch MFC
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC-Kern- und -Standardkomponenten
#include <afxext.h>         // MFC-Erweiterungen


#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC-Unterstützung für allgemeine Steuerelemente von Internet Explorer 4
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC-Unterstützung für allgemeine Windows-Steuerelemente
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC-Unterstützung für Multifunktionsleisten und Steuerleisten


#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

#include <GL/gl.h>
#include <GL/glu.h>

#define GL_BGR                        0x80E0
#define GL_BGRA                       0x80E1
#define GL_GENERATE_MIPMAP_SGIS       0x8191

#ifndef IMPLEMENT_IUNKNOWN

#define IMPLEMENT_IUNKNOWN_ADDREF(ObjectClass, InterfaceClass) \
	STDMETHODIMP_(ULONG) ObjectClass::X##InterfaceClass::AddRef(void) \
	{ \
		METHOD_PROLOGUE(ObjectClass, InterfaceClass); \
		return pThis->ExternalAddRef(); \
	}

#define IMPLEMENT_IUNKNOWN_RELEASE(ObjectClass, InterfaceClass) \
	STDMETHODIMP_(ULONG) ObjectClass::X##InterfaceClass::Release(void) \
	{ \
		METHOD_PROLOGUE(ObjectClass, InterfaceClass); \
		return pThis->ExternalRelease(); \
	}

#define IMPLEMENT_IUNKNOWN_QUERYINTERFACE(ObjectClass, InterfaceClass) \
	STDMETHODIMP ObjectClass::X##InterfaceClass::QueryInterface(REFIID riid, LPVOID* ppVoid) \
	{ \
		METHOD_PROLOGUE(ObjectClass, InterfaceClass); \
		return (HRESULT)pThis->ExternalQueryInterface(&riid, ppVoid); \
	}

#define IMPLEMENT_IUNKNOWN(ObjectClass, InterfaceClass) \
	IMPLEMENT_IUNKNOWN_ADDREF(ObjectClass, InterfaceClass) \
	IMPLEMENT_IUNKNOWN_RELEASE(ObjectClass, InterfaceClass) \
	IMPLEMENT_IUNKNOWN_QUERYINTERFACE(ObjectClass, InterfaceClass)

#endif // IMPLEMENT_IUNKNOWN

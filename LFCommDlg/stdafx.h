
// stdafx.h: Includedatei f�r Standardsystem-Includedateien
// oder h�ufig verwendete, projektspezifische Includedateien,
// die nur selten ge�ndert werden.

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Selten verwendete Teile der Windows-Header ausschlie�en
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _AFX_ALL_WARNINGS

#include <afxwin.h>             // MFC-Kern- und -Standardkomponenten
#include <afxext.h>             // MFC-Erweiterungen
#include <afxcontrolbars.h>     // MFC-Unterst�tzung f�r Multifunktionsleisten und Steuerleisten


#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")


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

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _AFX_ALL_WARNINGS

#include <afxwin.h>             // MFC-Kern- und -Standardkomponenten
#include <afxext.h>             // MFC-Erweiterungen
#include <afxcontrolbars.h>     // MFC-Unterstützung für Multifunktionsleisten und Steuerleisten


#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

#include <GL/gl.h>
#include <GL/glu.h>

#define GL_RESCALE_NORMAL                    0x803A
#define GL_BGR                               0x80E0
#define GL_BGRA                              0x80E1
#define GL_GENERATE_MIPMAP_SGIS              0x8191
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT      0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT     0x83F1

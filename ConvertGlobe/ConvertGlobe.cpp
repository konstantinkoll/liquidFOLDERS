// ConvertGlobe.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "ConvertGlobe.h"
#include <io.h>
#include <wchar.h>


// Das einzige Anwendungsobjekt
//

CWinApp theApp;
CString path;

using namespace std;

struct Vertex
{
	FLOAT X, Y, Z, U, V;
};
Vertex Vertices[25000];

struct Poly
{
	UINT A, B, C;
};
Poly Polygons[10000];

void ConvertFile(CString Suffix)
{
	CString tmpStr;

	// Daten
	UINT VertexCount = 0;
	UINT PolyCount = 0;

	// Einlesen
	CStdioFile input;
	input.Open(path+"..\\..\\res\\Globe_"+Suffix+".asc", CFile::modeRead | CFile::typeText);

	while (input.ReadString(tmpStr))
	{
		UINT id;
		FLOAT X;
		FLOAT Y;
		FLOAT Z;
		FLOAT U;
		FLOAT V;
		if (sscanf_s(tmpStr.GetBuffer(), "Vertex %d:  X:%f     Y:%f     Z:%f     U:%f     V:%f", &id, &X, &Y, &Z, &U, &V)==6)
		{
			if (id+1>VertexCount)
				VertexCount = id+1;

			Vertices[id].X = X;
			Vertices[id].Y = Y;
			Vertices[id].Z = Z;
			Vertices[id].U = U;
			Vertices[id].V = V;
		}

		UINT A;
		UINT B;
		UINT C;
		UINT AB;
		UINT BC;
		UINT CA;
		if (sscanf_s(tmpStr.GetBuffer(), "Face %d:  A:%d B:%d C:%d AB:%d BC:%d CA:%d", &id, &A, &B, &C, &AB, &BC, &CA)==7)
		{
			if (id+1>PolyCount)
				PolyCount = id+1;

			Polygons[id].A = A;
			Polygons[id].B = B;
			Polygons[id].C = C;
		}
	}

	input.Close();

	// Ausgeben
	CStdioFile output;
	output.Open(path+"..\\..\\StoreManager\\Globe_"+Suffix+".h", CFile::modeWrite | CFile::modeCreate | CFile::typeText);

	tmpStr.Format("static UINT Globe%sCount = %d;\n", Suffix, PolyCount*3);
	output.WriteString(tmpStr);
	tmpStr.Format("static FLOAT Globe%sNodes[] = {\n", Suffix);
	output.WriteString(tmpStr);
	for (UINT a=0; a<PolyCount; a++)
	{
		FLOAT LastU = 0.0f;
		BOOL FirstTriple = TRUE;

#define Print(id) { \
		FLOAT U = Vertices[id].U-0.25f; \
		if (U<0.0f) \
			U += 1.0f; \
		if (!FirstTriple) \
		{ \
			if ((LastU>0.75f) && (U<0.25f)) \
				U += 1.0f; \
			if ((LastU<0.25f) && (U>0.75f)) \
				U -= 1.0f; \
		} \
		tmpStr.Format("\t%ff, %ff, %ff, %ff, %ff,\n", U, Vertices[id].V, Vertices[id].X, Vertices[id].Y, Vertices[id]. Z); \
		output.WriteString(tmpStr); \
		LastU = U; \
		FirstTriple = FALSE; \
	}

		Print(Polygons[a].A);
		Print(Polygons[a].B);
		Print(Polygons[a].C);
	}
	output.WriteString("};\n");

	output.Close();
}

INT _tmain(INT /*argc*/, TCHAR* /*argv[]*/, TCHAR* /*envp[]*/)
{
	// MFC initialisieren und drucken. Bei Fehlschlag Fehlermeldung aufrufen.
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: Den Fehlercode an Ihre Anforderungen anpassen.
		_tprintf(_T("Schwerwiegender Fehler bei der MFC-Initialisierung\n"));
		return 1;
	}

	// Pfad
	TCHAR szPathName[MAX_PATH];
	::GetModuleFileName(NULL, szPathName, MAX_PATH);
	LPTSTR pszFileName = _tcsrchr(szPathName, '\\')+1;
	*pszFileName = '\0';
	path = szPathName;

	// Konvertieren
	ConvertFile("High");
	ConvertFile("Low");

	return 0;
}

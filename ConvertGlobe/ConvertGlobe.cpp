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
		FLOAT U[3];
		U[0] = Vertices[Polygons[a].A].U;
		U[1] = Vertices[Polygons[a].B].U;
		U[2] = Vertices[Polygons[a].C].U;

		FLOAT V[3];
		V[0] = Vertices[Polygons[a].A].V;
		V[1] = Vertices[Polygons[a].B].V;
		V[2] = Vertices[Polygons[a].C].V;

		for (UINT b=0; b<3; b++)
		{
			U[b] -= 0.2501f;
			if (U[b]<0.0f)
				U[b] += 1.0f;
		}

		for (UINT b=1; b<3; b++)
		{
			if ((U[b-1]>0.75f) && (U[b]<0.25f))
				U[b] += 1.0f;
			if ((U[b-1]<0.25f) && (U[b]>0.75f))
				U[b] -= 1.0f;
		}

		for (UINT b=0; b<3; b++)
		{
			if ((V[b]<0.02f) || (V[b]>0.98f))
			{
				UINT eins = (b==0) ? 1 : (b==1) ? 2 : 0;
				UINT zwei = (b==0) ? 2 : (b==1) ? 0 : 1;
				U[b] = (U[eins]+U[zwei])/2.0f;
				break;
			}
		}

#define Print(id, no) { \
		tmpStr.Format("\t%ff, %ff, %ff, %ff, %ff,\n", U[no], V[no], Vertices[id].X, Vertices[id].Y, Vertices[id]. Z); \
		output.WriteString(tmpStr); \
	}

		Print(Polygons[a].A, 0);
		Print(Polygons[a].B, 1);
		Print(Polygons[a].C, 2);
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

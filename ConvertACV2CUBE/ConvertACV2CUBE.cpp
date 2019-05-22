// ConvertACV2CUBE.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "ConvertACV2CUBE.h"
#include <io.h>
#include <wchar.h>


#define SOURCEPATH          _T("C:\\Users\\ROOT\\Desktop\\Afterlight\\")
#define DESTINATIONPATH     _T("C:\\Users\\ROOT\\Dropbox\\LUTs\\AL4\\")
using namespace std;

struct CURVES
{
	BOOL Valid;
	DOUBLE Master[256];
	DOUBLE Red[256];
	DOUBLE Green[256];
	DOUBLE Blue[256];
};

void ParseCurve(USHORT* Buffer, UINT& Ptr, DOUBLE* Curve)
{
	const UINT Count = Buffer[Ptr++];

	UINT LastIndex = 0;
	UINT LastValue = 0;

	for (UINT a=0; a<Count; a++)
	{
		if (a>0)
		{
			ASSERT(Buffer[Ptr+1]>=LastIndex);
			const UINT Slope = Buffer[Ptr+1]-LastIndex;

			for (UINT b=0; b<Slope; b++)
				if (LastIndex+b<=255)
					Curve[LastIndex+b+1] = (LastValue*(Slope-b-1)+Buffer[Ptr]*b)/(255.0*Slope);
		}
		else
		{
			for (UINT b=0; b<=Buffer[Ptr+1]; b++)
				Curve[b] = Buffer[Ptr]/255.0;
		}

		LastValue = Buffer[Ptr++];
		LastIndex = Buffer[Ptr++];
	}

	for (UINT a=LastIndex+1; a<=255; a++)
		Curve[a] = LastValue / 255.0;
}

void ReadACV(CURVES& Curves, const CString& FN)
{
	// Initalisieren
	Curves.Valid = FALSE;

	for (UINT a=0; a<128; a++)
	{
		Curves.Master[a] = Curves.Red[a] = Curves.Green[a] = Curves.Blue[a] = 0.0;
		Curves.Master[a+128] = Curves.Red[a+128] = Curves.Green[a+128] = Curves.Blue[a+128] = 1.0;
	}

	USHORT Buffer[2048];

	HANDLE hFile = CreateFile(SOURCEPATH+FN+_T(".ACV"), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hFile)
		return;

	DWORD wmRead;
	BOOL Result = ReadFile(hFile, &Buffer, sizeof(Buffer), &wmRead, NULL);
	Result &= (wmRead>=10);

	CloseHandle(hFile);

	if (!Result)
		return;

	for (UINT a=0; a<2048; a++)
		Buffer[a] = 256*(Buffer[a] & 0xFF) | (Buffer[a]>>8);

	// Parsen
	if ((Buffer[0]!=4) || (Buffer[1]<4))
		return;

	UINT Ptr = 2;
	ParseCurve(Buffer, Ptr, Curves.Master);
	ParseCurve(Buffer, Ptr, Curves.Red);
	ParseCurve(Buffer, Ptr, Curves.Green);
	ParseCurve(Buffer, Ptr, Curves.Blue);

	Curves.Valid = TRUE;
}

void ConvertFile(const CString& FN)
{
	wcout << FN.GetString() << _T("\n");

	// Einlesen
	CURVES Curves;
	ReadACV(Curves, FN);

	if (!Curves.Valid)
		return;

	// Ausgabe
	CStdioFile output;
	output.Open(DESTINATIONPATH+FN+_T(".cube"), CFile::modeWrite| CFile::modeCreate | CFile::typeText);
	
	CString tmpStr;
	tmpStr.Format(_T("TITLE \"%s\"\nLUT_3D_SIZE 32\n\n"), FN);
	output.WriteString(tmpStr);

	for (UINT Blue=0; Blue<32; Blue++)
		for (UINT Green=0; Green<32; Green++)
			for (UINT Red=0; Red<32; Red++)
			{
				const UINT R = (Red*255)/31;
				const UINT G = (Green*255)/31;
				const UINT B = (Blue*255)/31;

				tmpStr.Format(_T("%.6f %.6f %.6f\n"),
					Curves.Master[(UINT)(255.0*Curves.Red[R])],
					Curves.Master[(UINT)(255.0*Curves.Green[G])],
					Curves.Master[(UINT)(255.0*Curves.Blue[B])]);
				output.WriteString(tmpStr);
			}

	output.Close();
}

INT _tmain(INT /*argc*/, TCHAR* /*argv[]*/, TCHAR* /*envp[]*/)
{
	// Konvertieren
	CString Mask(SOURCEPATH);
	Mask.Append(_T("*.ACV"));

	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile(Mask, &FindData);

	if (hFind!=INVALID_HANDLE_VALUE)
	do
	{
		WCHAR FN[MAX_PATH];
		wcscpy_s(FN, MAX_PATH, FindData.cFileName);

		WCHAR* pChar = wcsrchr(FN, L'.');
		if (pChar)
			*pChar = '\0';

		ConvertFile(FN);
	}
	while (FindNextFile(hFind, &FindData)!=0);

	FindClose(hFind);

	return 0;
}

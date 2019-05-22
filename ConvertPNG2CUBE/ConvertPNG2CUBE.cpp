// ConvertPNG2CUBE.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "ConvertPNG2CUBE.h"
#include <io.h>
#include <wchar.h>


#define SOURCEPATH          _T("C:\\Users\\ROOT\\Desktop\\Afterlight PNG\\")
#define DESTINATIONPATH     _T("C:\\Users\\ROOT\\Dropbox\\LUTs\\Afterlight\\")
using namespace std;

void ConvertFile(const CString& FN)
{
	wcout << FN.GetString() << _T("\n");

	// Einlesen
	Bitmap bmp(SOURCEPATH+FN+_T(".PNG"));

	if ((bmp.GetWidth()!=512) || (bmp.GetHeight()!=512))
		return;

	Rect rect(0, 0, 512, 512);
	BitmapData Data;
	bmp.LockBits(&rect, ImageLockModeRead, bmp.GetPixelFormat(), &Data);

	// Ausgabe
	CStdioFile output;
	output.Open(DESTINATIONPATH+FN+_T(".cube"), CFile::modeWrite| CFile::modeCreate | CFile::typeText);
	
	CString tmpStr;
	tmpStr.Format(_T("TITLE \"%s\"\nLUT_3D_SIZE 64\n\n"), FN);
	output.WriteString(tmpStr);

	for (UINT Blue=0; Blue<64; Blue++)
	{
		const UINT SqX = (Blue & 7)*64;
		const UINT SqY = (Blue / 8)*64;

		for (UINT Green=0; Green<64; Green++)
		{
			const UINT Y = SqY + Green;

			for (UINT Red=0; Red<64; Red++)
			{
				const UINT X = SqX + Red;

				LPBYTE pColor = (LPBYTE)Data.Scan0+X*(Data.Stride/512)+Y*Data.Stride;

				tmpStr.Format(_T("%.6f %.6f %.6f\n"), *(pColor+2)/255.0, *(pColor+1)/255.0, *(pColor+0)/255.0);
				output.WriteString(tmpStr);
			}
		}
	}

	output.Close();
	bmp.UnlockBits(&Data);
}

INT _tmain(INT /*argc*/, TCHAR* /*argv[]*/, TCHAR* /*envp[]*/)
{
	// Initialize GDI+
	ULONG_PTR GdiPlusToken;
	GdiplusStartupInput StartupInput;
	GdiplusStartup(&GdiPlusToken, &StartupInput, NULL);

	// Konvertieren
	CString Mask(SOURCEPATH);
	Mask.Append(_T("*.PNG"));

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

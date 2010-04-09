// ConvertIATA.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "ConvertIATA.h"
#include <io.h>
#include <wchar.h>


// Das einzige Anwendungsobjekt
//

CWinApp theApp;
CString path;

using namespace std;

int Compare(CString s1, CString s2)
{
	s1.Replace('Ä', 'A');
	s1.Replace('Ö', 'O');
	s1.Replace('Ü', 'U');
	s1.Replace('ä', 'a');
	s1.Replace('ö', 'o');
	s1.Replace('ü', 'u');
	s1.Replace('ß', 'z');
	s2.Replace('Ä', 'A');
	s2.Replace('Ö', 'O');
	s2.Replace('Ü', 'U');
	s2.Replace('ä', 'a');
	s2.Replace('ö', 'o');
	s2.Replace('ü', 'u');
	s2.Replace('ß', 'z');

	return s1.CompareNoCase(s2);
}

double GetCoord(CString s)
{
	USES_CONVERSION;

	int curPos = s.Find("*");
	double c = atof(s.Mid(0, curPos));
	s = s.Mid(curPos+1);

	curPos = s.Find("'");
	c += atof(s.Mid(0, curPos))/60;
	s = s.Mid(curPos+1);

	curPos = s.Find("\"");
	c += atof(s.Mid(0, curPos))/3600;
	s = s.Mid(curPos+1);

	if ((s=="N") || (s=="W"))
		c = -c;

	return c;
}

void ConvertFile(CString LanguageSuffix)
{
	CString tmpStr;

	// Daten
	int CountryCount = 0;
	CString CountryNames[512];

	struct Airport
	{
		int CountryID;
		CString Code;
		CString MetroCode;
		CString Name;
		double Latitude;
		double Longitude;
		UINT Divider;
	};

	UINT AirportCount = 0;
	Airport Airports[26*26*26];

	// Einlesen
	CStdioFile input;
	input.Open(path+"..\\..\\res\\IATA-"+LanguageSuffix+".TXT", CFile::modeRead | CFile::typeText);

	while (input.ReadString(tmpStr))
	{
		tmpStr.OemToAnsi();

		int curPos= 0;
		CString NamesAndCodes = tmpStr.Tokenize(";", curPos);
		CString Dummy = tmpStr.Tokenize(";", curPos);
		CString Coord = "";
		if (curPos!=-1)
			Coord = tmpStr.Tokenize(";", curPos);

		int CountryPos = NamesAndCodes.Find(", ", 0);
		if (CountryPos)
		{
			Airports[AirportCount].Code = NamesAndCodes.Mid(0, 3);
			Airports[AirportCount].MetroCode = NamesAndCodes.Mid(4, 3).Trim();
			if ((Airports[AirportCount].Code!=Airports[AirportCount].MetroCode) && (Coord==""))
				continue;

			CString Country = NamesAndCodes.Mid(CountryPos+2);
			NamesAndCodes.Truncate(CountryPos);

			int CountryID = -1;
			for (int a=0; a<CountryCount; a++)
				if (Country==CountryNames[a])
				{
					CountryID = a;
					break;
				}

			if (CountryID==-1)
			{
				if (CountryCount==0)
				{
					CountryID = 0;
					CountryNames[0] = Country;
				}
				else
				{
					for (int InsertPos=0; InsertPos<=CountryCount; InsertPos++)
						if ((InsertPos==CountryCount) || (Compare(CountryNames[InsertPos],Country)>0))
						{
							for (int a=CountryCount; a>InsertPos; a--)
								CountryNames[a] = CountryNames[a-1];
							CountryID = InsertPos;
							CountryNames[InsertPos] = Country;
							for (UINT a=0; a<AirportCount; a++)
								if (Airports[a].CountryID>=InsertPos)
									Airports[a].CountryID++;
							break;
						}
				}
				CountryCount++;
			}

			Airports[AirportCount].Name = NamesAndCodes.Mid(8);
			Airports[AirportCount].CountryID = CountryID;

			if (Coord=="")
			{
				Airports[AirportCount].Latitude = 0;
				Airports[AirportCount].Longitude = 0;
				Airports[AirportCount].Divider = 0;
			}
			else
			{
				curPos = Coord.Find(", ");
				Airports[AirportCount].Latitude = GetCoord(Coord.Mid(0, curPos));
				Airports[AirportCount].Longitude = GetCoord(Coord.Mid(curPos+2));
				Airports[AirportCount].Divider = 1;
			}

			AirportCount++;
		}
	}

	input.Close();

	// Metropolitan-Codes berechnen
	for (UINT a=0; a<AirportCount; a++)
		if (Airports[a].MetroCode==Airports[a].Code)
			for (UINT b=0; b<AirportCount; b++)
				if ((a!=b) && (Airports[a].MetroCode==Airports[b].MetroCode))
				{
					Airports[a].Latitude += Airports[b].Latitude;
					Airports[a].Longitude += Airports[b].Longitude;
					Airports[a].Divider++;
				}

	// Ausgeben
	CStdioFile output;
	output.Open(path+"..\\..\\LFCore\\IATA_"+LanguageSuffix+".h", CFile::modeWrite | CFile::modeCreate | CFile::typeText);

	output.WriteString("// Countries\n");
	tmpStr.Format("#define CountryCount_%s %d\n", LanguageSuffix, CountryCount);
	output.WriteString(tmpStr);
	tmpStr.Format("LFCountry Countries_%s[CountryCount_%s] = {\n", LanguageSuffix, LanguageSuffix);
	output.WriteString(tmpStr);
	for (int a=0; a<CountryCount; a++)
	{
		CString Delimiter = (a<CountryCount-1) ? "," : "";
		tmpStr.Format("\t{ %3d, \"%s\" }%s\n", a, CountryNames[a], Delimiter);
		output.WriteString(tmpStr);
	}
	output.WriteString("};\n\n");

	output.WriteString("// Airports\n");
	tmpStr.Format("#define AirportCount_%s %d\n", LanguageSuffix, AirportCount);
	output.WriteString(tmpStr);
	tmpStr.Format("LFAirport Airports_%s[AirportCount_%s] = {\n", LanguageSuffix, LanguageSuffix);
	output.WriteString(tmpStr);
	for (UINT a=0; a<AirportCount; a++)
	{
		tmpStr.Format("\t{ %3d, \"%s\", \"%s\", \"%s\", ",
			Airports[a].CountryID, Airports[a].Code, Airports[a].MetroCode, Airports[a].Name);
		output.WriteString(tmpStr);
		CString Delimiter = (a<AirportCount-1) ? "," : "";
		tmpStr.Format("{ %f, %f } }%s\n",
			Airports[a].Latitude/Airports[a].Divider, Airports[a].Longitude/Airports[a].Divider, Delimiter);
		output.WriteString(tmpStr);
	}
	output.WriteString("};\n");

	output.Close();
}

int _tmain(int /*argc*/, TCHAR* /*argv[]*/, TCHAR* /*envp[]*/)
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
	ConvertFile("DE");
	ConvertFile("EN");

	return 0;
}

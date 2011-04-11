// DriveTest.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "DriveTest.h"
#include <io.h>
#include <wchar.h>
#include <winioctl.h>


// Das einzige Anwendungsobjekt
//

CWinApp theApp;

unsigned int GetDriveBus(char d)
{
	unsigned int res = BusTypeMaxReserved;

	char szBuf[MAX_PATH] = "\\\\?\\ :";
	szBuf[4] = d;
	HANDLE hDevice = CreateFileA(szBuf, 0, 0, NULL, OPEN_EXISTING, NULL, NULL);

	if (hDevice!=INVALID_HANDLE_VALUE)
	{
		STORAGE_ADAPTER_DESCRIPTOR* pDevDesc = (STORAGE_ADAPTER_DESCRIPTOR*)new BYTE[sizeof(STORAGE_ADAPTER_DESCRIPTOR)];
		pDevDesc->Size = sizeof(STORAGE_ADAPTER_DESCRIPTOR);

		STORAGE_PROPERTY_QUERY Query;
		Query.PropertyId = StorageAdapterProperty;
		Query.QueryType = PropertyStandardQuery;

		DWORD dwOutBytes;
		if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
			&Query, sizeof(STORAGE_PROPERTY_QUERY), pDevDesc, pDevDesc->Size,
			&dwOutBytes, NULL))
			res = pDevDesc->BusType;

		delete[] pDevDesc;
		CloseHandle(hDevice);
	}

	return res;
}

bool IsHotplug(char d)
{
	bool res = false;

	char szBuf[MAX_PATH] = "\\\\?\\ :";
	szBuf[4] = d;
	HANDLE hDevice = CreateFileA(szBuf, 0, 0, NULL, OPEN_EXISTING, NULL, NULL);

	if (hDevice!=INVALID_HANDLE_VALUE)
	{
		STORAGE_HOTPLUG_INFO* pDevDesc = (STORAGE_HOTPLUG_INFO*)new BYTE[sizeof(STORAGE_HOTPLUG_INFO)];
		pDevDesc->Size = sizeof(STORAGE_ADAPTER_DESCRIPTOR);

		STORAGE_PROPERTY_QUERY Query;
		Query.PropertyId = StorageAdapterProperty;
		Query.QueryType = PropertyStandardQuery;

		DWORD dwOutBytes;
		if (DeviceIoControl(hDevice, IOCTL_STORAGE_GET_HOTPLUG_INFO,
			&Query, sizeof(STORAGE_PROPERTY_QUERY), pDevDesc, pDevDesc->Size,
			&dwOutBytes, NULL))
			res = pDevDesc->MediaRemovable==TRUE;

		delete[] pDevDesc;
		CloseHandle(hDevice);
	}

	return res;
}

void AnalyseLogicalDrives()
{
	DWORD DrivesOnSystem = GetLogicalDrives() & ~3;
	DWORD Index = 1;
	char szDriveRoot[] = " :\\";

	for (char cDrive='A'; cDrive<='Z'; cDrive++, Index<<=1)
	{
		if (!(DrivesOnSystem & Index))
			continue;

		szDriveRoot[0] = cDrive;
		std::cout << cDrive << _T(": ") << GetDriveTypeA(szDriveRoot) << _T(", ") << GetDriveBus(cDrive) << (IsHotplug(cDrive) ? _T(", HOTPLUG") : _T("")) << _T("\n");
	}
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

	AnalyseLogicalDrives();

	return 0;
}

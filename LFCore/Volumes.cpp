
#include "stdafx.h"
#include "LFCore.h"
#include "Volumes.h"
//#include <shlobj.h>
#include <winioctl.h>


extern OSVERSIONINFO osInfo;
VOLUME Volumes[26];


void InitVolumes()
{
	ZeroMemory(&Volumes, sizeof(Volumes));
}


// Registry settings
//

LFCORE_API BOOL LFHideFileExt()
{
	DWORD HideFileExt = 0;

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", &hKey)==ERROR_SUCCESS)
	{
		DWORD dwSize = sizeof(HideFileExt);
		RegQueryValueEx(hKey, L"HideFileExt", 0, NULL, (LPBYTE)&HideFileExt, &dwSize);

		RegCloseKey(hKey);
	}

	return (HideFileExt!=0);
}

LFCORE_API BOOL LFShowCompColor()
{
	DWORD ShowCompColor = 0;

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", &hKey)==ERROR_SUCCESS)
	{
		DWORD dwSize = sizeof(ShowCompColor);
		RegQueryValueEx(hKey, (osInfo.dwMajorVersion>=10) ? L"ShowEncryptCompressedColor" : L"ShowCompColor", 0, NULL, (LPBYTE)&ShowCompColor, &dwSize);

		RegCloseKey(hKey);
	}

	return (ShowCompColor!=0);
}

LFCORE_API BOOL LFHideVolumesWithNoMedia()
{
	DWORD HideVolumesWithNoMedia = (osInfo.dwMajorVersion<6) ? 0 : 1;

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", &hKey)==ERROR_SUCCESS)
	{
		DWORD dwSize = sizeof(HideVolumesWithNoMedia);
		RegQueryValueEx(hKey, L"HideVolumesWithNoMedia", 0, NULL, (LPBYTE)&HideVolumesWithNoMedia, &dwSize);

		RegCloseKey(hKey);
	}

	return (HideVolumesWithNoMedia!=0);
}


// Volume information
//

BYTE GetVolumeBus(CHAR cVolume)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	BYTE VolumeBus = BusTypeMaxReserved;

	WCHAR szDevice[7] = L"\\\\?\\ :";
	szDevice[4] = cVolume;

	HANDLE hDevice = CreateFile(szDevice, 0, 0, NULL, OPEN_EXISTING, NULL, NULL);
	if (hDevice!=INVALID_HANDLE_VALUE)
	{
		STORAGE_ADAPTER_DESCRIPTOR OutBuffer;
		OutBuffer.Size = sizeof(STORAGE_ADAPTER_DESCRIPTOR);

		STORAGE_PROPERTY_QUERY Query;
		Query.PropertyId = StorageAdapterProperty;
		Query.QueryType = PropertyStandardQuery;

		DWORD dwOutBytes;
		if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
			&Query, sizeof(STORAGE_PROPERTY_QUERY), &OutBuffer, sizeof(STORAGE_ADAPTER_DESCRIPTOR),
			&dwOutBytes, NULL))
			VolumeBus = OutBuffer.BusType;

		CloseHandle(hDevice);
	}

	return VolumeBus;
}

LPVOLUME GetVolume(CHAR cVolume, BOOL ForceAvailable)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	const UINT VolumeID = cVolume-'A';
	LPVOLUME pVolume = &Volumes[VolumeID];

	// Is volume already mounted? Does volume exist?
	if (!(pVolume->Flags & LFFlagsMounted) && (ForceAvailable || (GetLogicalDrives() & (1<<VolumeID))))
	{
		// Assume internal volume
		pVolume->Flags = LFFlagsMounted;
		pVolume->Source = LFSourceInternal;
		pVolume->LogicalVolumeType = LFGLV_INTERNAL;

		// Detect drive type and volume source
		CHAR szRoot[] = " :\\";
		szRoot[0] = cVolume;

		switch (GetDriveTypeA(szRoot))
		{
		case DRIVE_REMOVABLE:
			// Assume external volume
			pVolume->LogicalVolumeType = LFGLV_EXTERNAL;

		case DRIVE_FIXED:
			// Inspect volume bus
			switch (GetVolumeBus(cVolume))
			{
			case BusType1394:
				pVolume->LogicalVolumeType = LFGLV_EXTERNAL;
				pVolume->Source = LFSource1394;
				break;

			case BusTypeUsb:
				pVolume->LogicalVolumeType = LFGLV_EXTERNAL;
				pVolume->Source = LFSourceUSB;
				break;
			}

			break;

		case DRIVE_REMOTE:
			// Network volume
			pVolume->LogicalVolumeType = LFGLV_NETWORK;
			pVolume->Source = LFSourceNethood;
			break;

		default:
			pVolume->LogicalVolumeType = 0;
		}

		// Other Flags
		WCHAR Root[4] = L" :\\";
		Root[0] = cVolume;

		DWORD dwFlags;
		if (GetVolumeInformation(Root, NULL, 0, NULL, NULL, &dwFlags, NULL, 0))
		{
			if (dwFlags & FILE_FILE_COMPRESSION)
				pVolume->Flags |= LFFlagsCompressionAllowed;

			if ((dwFlags & FILE_READ_ONLY_VOLUME)==0)
				pVolume->Flags |= LFFlagsWriteable;
		}
	}

	return pVolume;
}

void UnmountVolume(CHAR cVolume)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	const UINT VolumeID = cVolume-'A';
	Volumes[VolumeID].Flags = 0;
}

LFCORE_API SOURCE LFGetSourceForVolume(CHAR cVolume)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	LPCVOLUME lpcVolume = GetVolume(cVolume);

	return (lpcVolume->Flags & LFFlagsMounted) ? lpcVolume->Source : LFSourceUnknown;
}

LFCORE_API UINT LFGetLogicalVolumes(BYTE Mask)
{
	DWORD VolumesOnSystem = GetLogicalDrives();
	if ((Mask & LFGLV_FLOPPIES)==0)
		VolumesOnSystem &= ~3;

	DWORD Index = 1<<2;
	for (CHAR cVolume='C'; cVolume<='Z'; cVolume++, Index<<=1)
	{
		LPVOLUME lpVolume = GetVolume(cVolume, TRUE);
		if (VolumesOnSystem & Index)
		{
			if ((Mask & lpVolume->LogicalVolumeType)==0)
				VolumesOnSystem &= ~Index;
		}
		else
		{
			lpVolume->Flags = 0;
		}
	}

	return VolumesOnSystem;
}

// dllmain.cpp : Definiert den Einstiegspunkt f�r die DLL-Anwendung.

#include "stdafx.h"
#include "Mutex.h"
#include "IATA.h"
#include "StoreCache.h"


extern HMODULE LFCoreModuleHandle;
extern LFMessageIDs LFMessages;
extern char BootDrive;

bool APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		LFCoreModuleHandle = hModule;

		LFMessages.LookChanged = RegisterWindowMessageA("liquidFOLDERS.LookChanged");
		LFMessages.ItemsDropped = RegisterWindowMessageA("liquidFOLDERS.ItemsDropped");
		LFMessages.StoresChanged = RegisterWindowMessageA("liquidFOLDERS.StoresChanged");
		LFMessages.StoreAttributesChanged = RegisterWindowMessageA("liquidFOLDERS.StoreAttributesChanged");
		LFMessages.DefaultStoreChanged = RegisterWindowMessageA("liquidFOLDERS.DefaultStoreChanged");
		LFMessages.DrivesChanged = RegisterWindowMessageA("liquidFOLDERS.DrivesChanged");

		char WindowsDir[MAX_PATH];
		GetWindowsDirectoryA(WindowsDir, MAX_PATH);
		BootDrive = WindowsDir[0];

		InitMutex();
		InitAirportDatabase();
		InitStoreCache();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

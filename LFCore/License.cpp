
#include "stdafx.h"
#include "LFCore.h"
#include "License.h"

#pragma warning(push, 3)
#pragma warning(disable: 4702)
#pragma warning(disable: 4706)
#include "base64.h"
#include "rsa.h"
#include "pssr.h"
#pragma warning(pop)

using namespace CryptoPP;


#pragma data_seg(".shared")

BOOL LicenseRead = FALSE;
BOOL ExpireRead = FALSE;
LFLicense LicenseBuffer = { 0 };
FILETIME ExpireBuffer = { 0 };

#pragma data_seg()


#define BUFSIZE    4096


void ParseInput(CHAR* pStr, LFLicense* pLicense)
{
	assert(pStr);
	assert(pLicense);

	ZeroMemory(pLicense, sizeof(LFLicense));

	while (*pStr)
	{
		CHAR* Ptr = strstr(pStr, "\n");
		*Ptr = '\0';

		CHAR* Trenner = strchr(pStr, '=');
		if (Trenner)
		{
			*(Trenner++) = '\0';

			if (strcmp(pStr, LICENSE_ID)==0)
			{
				strcpy_s(pLicense->PurchaseID, 256, Trenner);
			}
			else
				if (strcmp(pStr, LICENSE_PRODUCT)==0)
				{
					strcpy_s(pLicense->ProductID, 256, Trenner);
				}
				else
					if (strcmp(pStr, LICENSE_DATE)==0)
					{
						strcpy_s(pLicense->PurchaseDate, 256, Trenner);
					}
					else
						if (strcmp(pStr, LICENSE_QUANTITY)==0)
						{
							strcpy_s(pLicense->Quantity, 256, Trenner);
						}
						else
							if (strcmp(pStr, LICENSE_NAME)==0)
							{
								strcpy_s(pLicense->RegName, 256, Trenner);
							}
							else
								if (strcmp(pStr, LICENSE_VERSION)==0)
								{
									sscanf_s(Trenner, "%u.%u.%u", &pLicense->Version.Major, &pLicense->Version.Minor, &pLicense->Version.Build);
								}
		}

		pStr = Ptr+1;
	}
}

__forceinline BOOL ReadCodedLicense(CHAR* pStr, SIZE_T cCount)
{
	BOOL Result = FALSE;

	HKEY hKey;
	if (RegOpenKey(HKEY_CURRENT_USER, L"Software\\liquidFOLDERS", &hKey)==ERROR_SUCCESS)
	{
		DWORD dwSize = (DWORD)cCount;
		Result = (RegQueryValueExA(hKey, "License", 0, NULL, (BYTE*)pStr, &dwSize)==ERROR_SUCCESS);

		RegCloseKey(hKey);
	}

	return Result;
}

BOOL GetLicense(LFLicense* pLicense)
{
	assert(pLicense);

	CHAR Message[BUFSIZE];
	ZeroMemory(Message, sizeof(Message));
	if (!ReadCodedLicense(Message, sizeof(Message)))
		return FALSE;

	// Setup
	Integer n("745495196278906388636775394847083621342948184497620571684486911233963026358348142924980767925246631723125776567861840016140759057887626699111750486589518844265093743018380979709327527515518976285922706516923147828076538170972730183425557516081175385650534524185881211094278086683594714172177608841889993609400198766281044688600596754569489192345101979343468669802344086907480228591789172201629911453850773648840583343122891763767764228796196156401170554177938285696830486894331437834556251102634591813052294727051913850611273897873094152049052538993868633785883333899830540017013351013051436649700047349078185669990895492280131774298910733408039488338775031855217004993409862255738766029617966149166800537682141977654630013676816397200968712319762658485930029154225302095517962261669873874532952773591788024202484800434032440378140651213784614438189252406134607226451954778487476382220064125800227678929995859762762265522856822435862521744384622820138233752235289143337592718212618381294424731866372596352871531111041688119666919042905495747876323829528637851924273124345938360066547750112529335899447558317824780247359979724026700097382563761302560657179092084838455014801002071816886727980707589178515801870998113718231400298837471.");
	Integer e("17.");

	CHAR Recovered[BUFSIZE];
	ZeroMemory(Recovered, sizeof(Recovered));

	// Verify and recover
	RSASS<PSSR, SHA256>::Verifier Verifier(n, e);

	try
	{
		StringSource(Message, TRUE,
			new Base64Decoder(
				new SignatureVerificationFilter(Verifier,
					new ArraySink((BYTE*)Recovered, BUFSIZE-1),
					SignatureVerificationFilter::THROW_EXCEPTION | SignatureVerificationFilter::PUT_MESSAGE)));
	}
	catch(CryptoPP::Exception /*&e*/)
	{
		return FALSE;
	}

	ParseInput(Recovered, pLicense);

	return TRUE;
}

LFCORE_API BOOL LFIsLicensed(LFLicense* pLicense, BOOL Reload)
{
	// Setup
	if (!LicenseRead || Reload)
	{
		LicenseRead = TRUE;

		if (!GetLicense(&LicenseBuffer))
			return FALSE;
	}

	if (pLicense)
		*pLicense = LicenseBuffer;

	return strncmp(LicenseBuffer.ProductID, "liquidFOLDERS", 13)==0;
}

LFCORE_API BOOL LFIsSharewareExpired()
{
	if (LFIsLicensed())
		return FALSE;

	// Setup
	if (!ExpireRead)
	{
		ExpireRead = TRUE;

		BOOL Result = FALSE;

		HKEY hKey;
		if (RegOpenKey(HKEY_CURRENT_USER, L"Software\\liquidFOLDERS", &hKey)==ERROR_SUCCESS)
		{
			DWORD dwSize = sizeof(DWORD);
			if (RegQueryValueEx(hKey, L"Seed", 0, NULL, (BYTE*)&ExpireBuffer.dwHighDateTime, &dwSize)==ERROR_SUCCESS)
			{
				dwSize = sizeof(DWORD);
				if (RegQueryValueEx(hKey, L"Envelope", 0, NULL, (BYTE*)&ExpireBuffer.dwLowDateTime, &dwSize)==ERROR_SUCCESS)
					Result = TRUE;
			}

			if (!Result)
			{
				GetSystemTimeAsFileTime(&ExpireBuffer);
				RegSetValueEx(hKey, L"Seed", 0, REG_DWORD, (BYTE*)&ExpireBuffer.dwHighDateTime, sizeof(DWORD));
				RegSetValueEx(hKey, L"Envelope", 0, REG_DWORD, (BYTE*)&ExpireBuffer.dwLowDateTime, sizeof(DWORD));
			}

			RegCloseKey(hKey);
		}
	}

	// Check
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);

	ULARGE_INTEGER FirstInstall;
	FirstInstall.HighPart = ExpireBuffer.dwHighDateTime;
	FirstInstall.LowPart = ExpireBuffer.dwLowDateTime;

	ULARGE_INTEGER Now;
	Now.HighPart = ft.dwHighDateTime;
	Now.LowPart = ft.dwLowDateTime;

#define SECOND ((ULONGLONG)10000000)
#define MINUTE (60*SECOND)
#define HOUR   (60*MINUTE)
#define DAY    (24*HOUR)

	FirstInstall.QuadPart += 30*DAY;

	return Now.QuadPart>=FirstInstall.QuadPart;
}

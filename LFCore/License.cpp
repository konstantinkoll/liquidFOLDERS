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

USING_NAMESPACE(CryptoPP)
USING_NAMESPACE(std)


// Der Inhalt dieses Segments wird über alle Instanzen von LFCore geteilt.

#pragma data_seg("common_license")

bool LicenseRead = false;
LFLicense LicenseBuffer = { 0 };

#pragma data_seg()
#pragma comment(linker, "/SECTION:common_license,RWS")


void ParseVersion(string& tmpStr, LFLicenseVersion* Version)
{
	char Point;

	stringstream ss(tmpStr);
	ss >> Version->Major;
	ss >> Point;
	ss >> Version->Minor;
	ss >> Point;
	ss >> Version->Release;
}

void ParseInput(string& tmpStr, LFLicense* License)
{
	ZeroMemory(License, sizeof(LFLicense));

	stringstream ss(tmpStr);
	string line;

	while (!ss.eof())
	{
		getline(ss, line);
		std::string::size_type delimiterPos = line.find_first_of("=");

		if (std::string::npos!=delimiterPos)
		{
			std::string name = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos+1);

			if (name==LICENSE_ID)
			{
				MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, License->PurchaseID, 256);
			}
			else
				if (name==LICENSE_PRODUCT)
				{
					MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, License->ProductID, 256);
				}
				else
					if (name==LICENSE_DATE)
					{
						MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, License->PurchaseDate, 16);
					}
					else
						if (name==LICENSE_QUANTITY)
						{
							MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, License->Quantity, 8);
						}
						else
							if (name==LICENSE_NAME)
							{
								MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, License->RegName, 256);
							}
							else
								if (name==LICENSE_VERSION)
								{
									ParseVersion(value, &License->Version);
								}
		}
	}
}

bool ReadCodedLicense(string& Message)
{
	bool res = false;

	HKEY k;
	if (RegOpenKeyA(HKEY_CURRENT_USER, "Software\\liquidFOLDERS", &k)==ERROR_SUCCESS)
	{
		char tmpStr[4096];
		DWORD sz = sizeof(tmpStr);
		if (RegQueryValueExA(k, "License", 0, NULL, (BYTE*)&tmpStr, &sz)==ERROR_SUCCESS)
		{
			Message = tmpStr;
			res = true;
		}

		RegCloseKey(k);
	}

	return res;
}

bool GetLicense(LFLicense* License)
{
	string Message;
	string Recovered;

	if (!ReadCodedLicense(Message))
		return false;

	// Setup
	Integer n("745495196278906388636775394847083621342948184497620571684486911233963026358348142924980767925246631723125776567861840016140759057887626699111750486589518844265093743018380979709327527515518976285922706516923147828076538170972730183425557516081175385650534524185881211094278086683594714172177608841889993609400198766281044688600596754569489192345101979343468669802344086907480228591789172201629911453850773648840583343122891763767764228796196156401170554177938285696830486894331437834556251102634591813052294727051913850611273897873094152049052538993868633785883333899830540017013351013051436649700047349078185669990895492280131774298910733408039488338775031855217004993409862255738766029617966149166800537682141977654630013676816397200968712319762658485930029154225302095517962261669873874532952773591788024202484800434032440378140651213784614438189252406134607226451954778487476382220064125800227678929995859762762265522856822435862521744384622820138233752235289143337592718212618381294424731866372596352871531111041688119666919042905495747876323829528637851924273124345938360066547750112529335899447558317824780247359979724026700097382563761302560657179092084838455014801002071816886727980707589178515801870998113718231400298837471.");
	Integer e("17.");

	// Verify and recover
	RSASS<PSSR, SHA256>::Verifier Verifier(n, e);

	try
	{
		StringSource(Message, true,
			new Base64Decoder(
				new SignatureVerificationFilter(Verifier,
					new StringSink(Recovered),
					SignatureVerificationFilter::THROW_EXCEPTION | SignatureVerificationFilter::PUT_MESSAGE)));
	}
	catch(CryptoPP::Exception /*&e*/)
	{
		return false;
	}

	ParseInput(Recovered, License);
	return true;
}

bool IsLicensed(LFLicense* License, bool Reload)
{
	if (!LicenseRead || Reload)
	{
		LicenseRead = true;

		if (!GetLicense(&LicenseBuffer))
			return false;
	}

	if (License)
		*License = LicenseBuffer;

	return (wcsncmp(LicenseBuffer.ProductID, L"liquidFOLDERS", 13)==0);
}

#include "stdafx.h"
#include "LFCore.h"
#include "License.h"
#include "base64.h"
#include "rsa.h"
#include "pssr.h"

USING_NAMESPACE(CryptoPP)
USING_NAMESPACE(std)


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

	while(!ss.eof())
	{
		getline(ss, line);
		std::string::size_type delimiterPos = line.find_first_of("=");

		if (std::string::npos!=delimiterPos)
		{
			std::string name = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos+1);

			if (name==LICENSE_ID)
			{
				MultiByteToWideChar(CP_UTF8, 0, value.c_str(), value.length()+1, License->PurchaseID, 256);
			}
			else
				if (name==LICENSE_PRODUCT)
				{
					MultiByteToWideChar(CP_UTF8, 0, value.c_str(), value.length()+1, License->ProductID, 256);
				}
				else
					if (name==LICENSE_DATE)
					{
						MultiByteToWideChar(CP_UTF8, 0, value.c_str(), value.length()+1, License->PurchaseDate, 16);
					}
					else
						if (name==LICENSE_QUANTITY)
						{
							MultiByteToWideChar(CP_UTF8, 0, value.c_str(), value.length()+1, License->Quantity, 8);
						}
						else
							if (name==LICENSE_NAME)
							{
								MultiByteToWideChar(CP_UTF8, 0, value.c_str(), value.length()+1, License->RegName, 256);
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
	Message = "TElDRU5TRV9JRD0wOjEKTElDRU5TRV9EQVRFPTA0LzA3LzIwMDQKTElDRU5TRV9QUk9EVUNU\
PWxpcXVpZEZPTERFUlMKTElDRU5TRV9RVUFOVElUWT0xCkxJQ0VOU0VfVkVSU0lPTj0wLjgu\
MApMSUNFTlNFX05BTUU9ZWxlbWVudCA1IOOBlOWIqeeUqOOBhOOBn+OBoOOBjeOAgeOBguOC\
iuOBjOOBqOOBhuOBlOOBluOBhOOBvuOBl+OBn+OAgiBLw7ZsbgogksfIcnihz1cSCd9NiJ8r\
UwKiCQpPsCrUwKW41kH47R7/xbU7UuU93RqNJ3b+hWZNkNvCSoYSYw4GzD2WHwzqZ9+zuE8X\
lcDrONwzouKeb/8AGcfPZyc+Nc5LRSTJgY8yI7dQT5C+64WUk8gjDIlebfe0mdE0qS+PuO8q\
UB/4N6B2xEPsODSdQpjRAs3aKxkufRvEfk3y6vVKltVrhcxzpEOY5OUNvwJqdqnaANmIvo4j\
zTZjCd3HUu3MAoPra+/qkkKuTsp2rZXn2RSpoWz5zsdadG74udnVRFsvGnhiVDs7QEO7uC7/\
BkC4iRU+gGmbJCNVBDsnJi26Lo2dZTtu/LowfAXnRGS4kmmcDOwwqCKIxLaSyoOEgODGuU7z\
eY1vSUrLyAmQVVCLtt9x/vpZQv+idgUwQ5Py96ROz0n6enTHN5EzW97JLZ+oKrdrs9sauWn2\
finH3wvcbTc1GGjJZNRropn86484w5NPR08RmFZH65oFRi64D+rHXC6qZmmgkQCESv6e/k+I\
j5jWIg79GI5h7+2Om54BH2CL7lbyCZ9SOntX+UuuB8JojcqizuDWh0IiIhvUAjm9L0BUwG4I\
nXobX5goaDyKV78LEOTT9H1VdLtsuXSNaFveLN3OGeO3BtMeecnF4qBpbVUPxwUYVtBDUaHK\
T2+XMh3nEfmzqQ==";

	return true;
}

bool GetLicense(LFLicense* License)
{
	string Message;
	string Recovered;

	if (!ReadCodedLicense(Message))
		return false;

	////////////////////////////////////////////////
	// Setup
	Integer n("745495196278906388636775394847083621342948184497620571684486911233963026358348142924980767925246631723125776567861840016140759057887626699111750486589518844265093743018380979709327527515518976285922706516923147828076538170972730183425557516081175385650534524185881211094278086683594714172177608841889993609400198766281044688600596754569489192345101979343468669802344086907480228591789172201629911453850773648840583343122891763767764228796196156401170554177938285696830486894331437834556251102634591813052294727051913850611273897873094152049052538993868633785883333899830540017013351013051436649700047349078185669990895492280131774298910733408039488338775031855217004993409862255738766029617966149166800537682141977654630013676816397200968712319762658485930029154225302095517962261669873874532952773591788024202484800434032440378140651213784614438189252406134607226451954778487476382220064125800227678929995859762762265522856822435862521744384622820138233752235289143337592718212618381294424731866372596352871531111041688119666919042905495747876323829528637851924273124345938360066547750112529335899447558317824780247359979724026700097382563761302560657179092084838455014801002071816886727980707589178515801870998113718231400298837471.");
	Integer e("17.");

	////////////////////////////////////////////////
	// Verify and Recover
	RSASS<PSSR, SHA256>::Verifier Verifier(n, e);

	try
	{
		StringSource(Message, true,
			new Base64Decoder(
				new SignatureVerificationFilter(Verifier,
					new StringSink(Recovered),
					SignatureVerificationFilter::THROW_EXCEPTION | SignatureVerificationFilter::PUT_MESSAGE)));
	}
	catch(CryptoPP::Exception &e)
	{
		return false;
	}

	ParseInput(Recovered, License);
	return true;
}

bool IsLicensed(LFLicense* License)
{
	LFLicense Buffer;
	if (!License)
		License = &Buffer;

	if (!GetLicense(License))
		return false;

	return false;
	return true;
}

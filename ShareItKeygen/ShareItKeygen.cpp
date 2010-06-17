// ShareItKeygen.cpp : Definiert den Einstiegspunkt f�r die Konsolenanwendung.
//

#include "stdafx.h"

#define ERC_SUCCESS          0
#define ERC_SUCCESS_BIN      1
#define ERC_ERROR            10
#define ERC_MEMORY           11
#define ERC_FILE_IO          12
#define ERC_BAD_ARGS         13
#define ERC_BAD_INPUT        14
#define ERC_EXPIRED          15
#define ERC_INTENAL          16

#define VERSION "1.0.0"
#define PRODUCT "liquidFOLDERS"

char* n = "745495196278906388636775394847083621342948184497620571684486911233963026358348142924980767925246631723125776567861840016140759057887626699111750486589518844265093743018380979709327527515518976285922706516923147828076538170972730183425557516081175385650534524185881211094278086683594714172177608841889993609400198766281044688600596754569489192345101979343468669802344086907480228591789172201629911453850773648840583343122891763767764228796196156401170554177938285696830486894331437834556251102634591813052294727051913850611273897873094152049052538993868633785883333899830540017013351013051436649700047349078185669990895492280131774298910733408039488338775031855217004993409862255738766029617966149166800537682141977654630013676816397200968712319762658485930029154225302095517962261669873874532952773591788024202484800434032440378140651213784614438189252406134607226451954778487476382220064125800227678929995859762762265522856822435862521744384622820138233752235289143337592718212618381294424731866372596352871531111041688119666919042905495747876323829528637851924273124345938360066547750112529335899447558317824780247359979724026700097382563761302560657179092084838455014801002071816886727980707589178515801870998113718231400298837471.";
char* d = "12181294056844875631319859392926202963120068374144126988308609660685670365332485995506221698124944962796172819736304575427136585913196514691368472003096713141586499068927793786100122998619591115783050760080443591962034937434194937637672508432698944209976054316762764887161406645156776375362379229442646954401964032128775240009813672460285771116750032342213540356247452400449023343003091049046240383232855778575826525214426336009277193280983597326816512323168926236876315145332213036512357044160695944657717234102155455075347612710344675687076021878984781597808551207513570915310675670147899291661765479560101073039066022108451454408274187692658010355441562719849458638790021356970255423858011383951446080370170553023217597744480235158046012378553678539808324790639575473719081587759335850855449473797832337100157421965577987346429638456917570823918805268632534116833771374832951821591132860993375298262806571904797242322693789055253743204422676720288064044725490664460034669448906493272426741859311692860773478004919621954838617989903933343602770969035084022422765307894946392906002513869858767688786349481218175235579228295478611739949337614971665109746034846788106477956377688633304983228040540372237724133583622990520445783785013.";
char* e = "17.";

class ShareITData
{
public:
	bool utf8;
	std::string purchaseId;
	std::string runningNo;
	std::string purchaseDate;
	std::string productId;
	std::string quantity;
	std::string regName;
	std::string language;
};

ShareITData parseInput(char* filename)
{
	std::string line;

	std::ifstream input(filename);

	ShareITData result;
	result.utf8 = false;

	if (input.is_open())
		while (!input.eof())
		{
			getline(input, line);

			std::string::size_type delimiterPos = line.find_first_of("=");

			if (std::string::npos!=delimiterPos)
			{
				std::string name = line.substr(0, delimiterPos);
				std::string value = line.substr(delimiterPos+1);

				if ((name=="ENCODING") && (value=="UTF8"))
				{
					result.utf8 = true;
				}
				else
					if (name=="PURCHASE_ID")
					{
						result.purchaseId = value;
					}
					else
						if (name=="RUNNING_NO")
						{
							result.runningNo = value;
						}
						else
							if (name=="PURCHASE_DATE")
							{
								result.purchaseDate = value;
							}
							else
								if (name=="PRODUCT_ID")
								{
									result.productId = value;
								}
								else
									if (name=="QUANTITY")
									{
										result.quantity = value;
									}
									else
										if (name=="REG_NAME")
										{
											result.regName = value;
										}
			}
		}

	return result;
}

int main(int argc, char* argv[])
{
	if (argc<4)
		return ERC_BAD_ARGS;

	ShareITData input = parseInput(argv[1]);

	///////////////////////////////////////
	// Pseudo Random Number Generator
	AutoSeededRandomPool rng;

	///////////////////////////////////////
	// Create Private Key from key material
	InvertibleRSAFunction params;
	params.Initialize(Integer(n), Integer(e), Integer(d));
	RSA::PrivateKey privateKey(params);

	stringstream ss;
	ss << LICENSE_ID << "=" << input.purchaseId << ":" << input.runningNo << endl;
	ss << LICENSE_DATE << "="<< input.purchaseDate << endl;
	ss << LICENSE_PRODUCT << "="<< PRODUCT << endl;
	ss << LICENSE_QUANTITY << "="<< input.quantity << endl;
	ss << LICENSE_VERSION << "="<< VERSION << endl;
	ss << LICENSE_NAME << "="<< input.regName << endl;
	std::string message = ss.str();

	////////////////////////////////////////////////
	// Sign and Encode
	RSASS<PSSR, SHA256>::Signer signer(privateKey);

	std::string signature;
	StringSource(message, true, new SignerFilter(rng, signer, new StringSink(signature)));

	std::string result;
	StringSource(message+signature, true, new Base64Encoder(new StringSink(result), false));

	std::ofstream keyMetadataOutput(argv[2]);
	keyMetadataOutput << "text/plain:liquidFOLDERS.reg" << std::endl;
	keyMetadataOutput.close();

	std::ofstream keyOutput(argv[3]);
	keyOutput << "Windows Registry Editor Version 5.00" << std::endl << std::endl;
	keyOutput << "[HKEY_CURRENT_USER\\Software\\liquidFOLDERS]" << std::endl;
	keyOutput << "\"License\"=\"" << result << "\"" << std::endl;
	keyOutput.close();

	return ERC_SUCCESS_BIN;
}

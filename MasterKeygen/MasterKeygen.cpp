// MasterKeygen.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"

int main(int argc, char* argv[])
{
	if (argc<2)
		return -1;

	///////////////////////////////////////
	// Pseudo Random Number Generator
	AutoSeededRandomPool rng;

	///////////////////////////////////////
	// Generate Parameters
	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, 4096);

	Integer n = params.GetModulus();
	Integer p = params.GetPrime1();
	Integer q = params.GetPrime2();
	Integer d = params.GetPrivateExponent();
	Integer e = params.GetPublicExponent();

	cout << argv[1] << endl;

	std::ofstream out(argv[1]);
	out << "RSA Parameters:" << endl;
	out << "char* n = \"" << params.GetModulus() << "\";" << endl;
	out << "char* d = \"" << params.GetPrivateExponent() << "\";" << endl;
	out << "char* e = \"" << params.GetPublicExponent() << "\";" << endl;
	out << endl;

	return 0;
}

#pragma once
#include "stdafx.h"
#include "liquidFOLDERS.h"

#define LICENSE_ID         "LICENSE_ID"
#define LICENSE_DATE       "LICENSE_DATE"
#define LICENSE_PRODUCT    "LICENSE_PRODUCT"
#define LICENSE_QUANTITY   "LICENSE_QUANTITY"
#define LICENSE_VERSION    "LICENSE_VERSION"
#define LICENSE_NAME       "LICENSE_NAME"

struct Version
{
	unsigned int major, minor, release;
};

class LicenseData {
public:
	std::string purchaseId;
	std::string purchaseDate;
	std::string productId;
	std::string quantity;
	Version version;
	std::string regName;
};

// Interner Gebrauch
bool IsLicensed();

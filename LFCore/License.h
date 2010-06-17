#pragma once
#include "stdafx.h"
#include "liquidFOLDERS.h"

// share*it
#define LICENSE_ID           "LICENSE_ID"
#define LICENSE_DATE         "LICENSE_DATE"
#define LICENSE_PRODUCT      "LICENSE_PRODUCT"
#define LICENSE_QUANTITY     "LICENSE_QUANTITY"
#define LICENSE_VERSION      "LICENSE_VERSION"
#define LICENSE_NAME         "LICENSE_NAME"

// Interner Gebrauch
bool IsLicensed(LFLicense* License, bool Reload);

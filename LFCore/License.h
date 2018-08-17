
#pragma once
#include "stdafx.h"
#include "LF.h"


// share*it
#define LICENSE_DATE         "LICENSE_DATE"
#define LICENSE_PRODUCT      "LICENSE_PRODUCT"
#define LICENSE_QUANTITY     "LICENSE_QUANTITY"
#define LICENSE_NAME         "LICENSE_NAME"


LFCORE_API BOOL LFIsLicensed(LFLicense* pLicense, BOOL Reload);
LFCORE_API BOOL LFIsSharewareExpired();


#include "stdafx.h"
#include "IndexTables.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "ShellProperties.h"
#include <assert.h>
#include <shlobj.h>
#include <shlwapi.h>


extern const BYTE AttrTypes[];


#pragma data_seg(".shared")

#pragma pack(push,1)

static const FMTID PropertyStorage =
	{ 0xB725F130, 0x47EF, 0x101A, { 0xA5, 0xF1, 0x02, 0x60, 0x8C, 0x9E, 0xEB, 0xAC } };
static const FMTID PropertyQuery =
	{ 0x49691C90, 0x7E17, 0x101A, { 0xA9, 0x1C, 0x08, 0x00, 0x2B, 0x2E, 0xCD, 0xA9 } };
static const FMTID PropertySummary =
	{ 0xF29F85E0, 0x4FF9, 0x1068, { 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9 } };
static const FMTID PropertyDocuments =
	{ 0xD5CDD502, 0x2E9C, 0x101B, { 0x93, 0x97, 0x08, 0x00, 0x2B, 0x2C, 0xF9, 0xAE } };
static const FMTID PropertyImage =
	{ 0x6444048F, 0x4C8B, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
static const FMTID PropertyAudio =
	{ 0x64440490, 0x4C8B, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
static const FMTID PropertyVideo =
	{ 0x64440491, 0x4C8B, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
static const FMTID PropertyMedia =
	{ 0x64440492, 0x4C8B, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
static const FMTID PropertyPhoto =
	{ 0x14B81DA1, 0x0135, 0x4D31, { 0x96, 0xD9, 0x6C, 0xBF, 0xC9, 0x67, 0x1A, 0x99 } };
static const FMTID PropertyMusic =
	{ 0x56A3372E, 0xCE9C, 0x11D2, { 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6 } };
static const FMTID PropertyVersion =
	{ 0x0CEF7D53, 0xFA64, 0x11D1, { 0xA2, 0x03, 0x00, 0x00, 0xF8, 0x1F, 0xED, 0xEE } };
static const FMTID PropertyUnnamed1 =
	{ 0x3F8472B5, 0xE0AF, 0x4DB2, { 0x80, 0x71, 0xC5, 0x3F, 0xE7, 0x6A, 0xE7, 0xCE } };
static const FMTID PropertyUnnamed2 =
	{ 0x72FAB781, 0xACDA, 0x43E5, { 0xB1, 0x55, 0xB2, 0x43, 0x4F, 0x85, 0xE6, 0x78 } };
static const FMTID PropertyUnnamed3 =
	{ 0xE3E0584C, 0xB788, 0x4A5A, { 0xBB, 0x20, 0x7F, 0x5A, 0x44, 0xC9, 0xAC, 0xDD } };
static const FMTID PropertyUnnamed4 =
	{ 0x2E4B640D, 0x5019, 0x46D8, { 0x88, 0x81, 0x55, 0x41, 0x4C, 0xC5, 0xCA, 0xA0 } };
static const FMTID PropertyUnnamed5 =
	{ 0x2CBAA8F5, 0xD81F, 0x47CA, { 0xB1, 0x7A, 0xF8, 0xD8, 0x22, 0x30, 0x01, 0x31 } };
static const FMTID PropertyUnnamed6 =
	{ 0x43F8D7B7, 0xA444, 0x4F87, { 0x93, 0x83, 0x52, 0x27, 0x1C, 0x9B, 0x91, 0x5C } };
static const FMTID PropertyUnnamed7 =
	{ 0x276D7BB0, 0x5B34, 0x4FB0, { 0xAA, 0x4B, 0x15, 0x8E, 0xD1, 0x2A, 0x18, 0x09 } };

static const BYTE ContextSlaves[LFLastQueryContext+1] = {
	IDXTABLE_MASTER,					// LFContextAllFiles
	IDXTABLE_MASTER,					// LFContextFavorites
	IDXTABLE_AUDIO,						// LFContextAudio
	IDXTABLE_PICTURES,					// LFContextPictures
	IDXTABLE_VIDEOS,					// LFContextVideos
	IDXTABLE_DOCUMENTS,					// LFContextDocuments
	IDXTABLE_MASTER,					// LFContextContacts
	IDXTABLE_MESSAGES,					// LFContextMessages
	IDXTABLE_MASTER,					// LFContextEvents
	IDXTABLE_MASTER,					// LFContextNew
	IDXTABLE_MASTER,					// LFContextTrash
	IDXTABLE_MASTER						// LFContextFilters
};

#include "ContextTable.h"

LFShellProperty AttrProperties[LFAttributeCount] = {
	{ PropertyStorage, 10 },		// LFAttrFileName
	{ 0, 0 },						// LFAttrStoreID
	{ PropertyStorage, 8 },			// LFAttrFileID
	{ PropertySummary, 6 },			// LFAttrComments
	{ PropertyVersion, 3 },			// LFAttrDescription
	{ PropertyStorage, 15 },		// LFAttrCreationTime
	{ PropertyStorage, 14 },		// LFAttrFileTime
	{ PropertyUnnamed5, 100 },		// LFAttrAddTime
	{ 0, 0 },						// LFAttrDeleteTime
	{ PropertyUnnamed6, 100 },		// LFAttrArchiveTime
	{ PropertyStorage, 4 },			// LFAttrFileFormat
	{ 0, 0 },						// LFAttrFileCount
	{ PropertyStorage, 12 },		// LFAttrFileSize
	{ 0, 0 },						// LFAttrFlags
	{ PropertyQuery, 9 },			// LFAttrURL
	{ PropertySummary, 5 },			// LFAttrHashtags
	{ PropertyMedia, 9 },			// LFAttrRating
	{ PropertyUnnamed3, 11 },		// LFAttrPriority
	{ 0, 0 },						// LFAttrLocationName
	{ 0, 0 },						// LFAttrLocationIATA
	{ 0, 0 },						// LFAttrLocationGPS

	{ PropertyImage, 3 },			// LFAttrWidth
	{ PropertyImage, 4 },			// LFAttrHeight
	{ 0, 0 },						// LFAttrDimension
	{ 0, 0 },						// LFAttrAspectRatio
	{ PropertyVideo, 44 },			// LFAttrVideoCodec
	{ PropertyPhoto, 18248 },		// LFAttrRoll

	{ 0, 0 },						// LFAttrExposure
	{ 0, 0 },						// LFAttrFocus
	{ 0, 0 },						// LFAttrAperture
	{ 0, 0 },						// LFAttrChip

	{ PropertyMusic, 4 },			// LFAttrAlbum
	{ PropertyMedia, 7 },			// LFAttrChannels
	{ PropertyMedia, 5 },			// LFAttrSamplerate
	{ 0, 0 },						// LFAttrAudioCodec

	{ PropertyAudio, 3 },			// LFAttrDuration
	{ PropertyMedia, 4 },			// LFAttrBitrate

	{ PropertySummary, 4 },			// LFAttrArtist
	{ PropertySummary, 2 },			// LFAttrTitle
	{ PropertyMedia, 11 },			// LFAttrCopyright
	{ 0, 0 },						// LFAttrISBN
	{ 0, 0 },						// LFAttrLanguage
	{ PropertyDocuments, 14 },		// LFAttrPages
	{ PropertyUnnamed4, 100 },		// LFAttrRecordingTime
	{ PropertyPhoto, 272 },			// LFAttrRecordingEquipment
	{ 0, 0 },						// LFAttrSignature

	{ 0, 0 },						// LFAttrFrom
	{ 0, 0 },						// LFAttrTo
	{ 0, 0 },						// LFAttrResponsible
	{ PropertyUnnamed1, 100 },		// LFAttrDueTime
	{ PropertyUnnamed2, 100 },		// LFAttrDoneTime
	{ PropertyUnnamed7, 100 },		// LFAttrClient
	{ 0, 0 }						// LFAttrLikeCount
};

#pragma pack(pop)

#pragma data_seg()


#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")


BYTE GetHardcodedContext(CHAR* Extension)
{
	INT First = 0;
	INT Last = (sizeof(Registry)/sizeof(RegisteredFile))-1;

	while (First<=Last)
	{
		INT Mid = (First+Last)/2;

		INT Result = strcmp(Registry[Mid].Format, Extension);
		if (Result==0)
			return Registry[Mid].ContextID;

		if (Result<0)
		{
			First = Mid+1;
		}
		else
		{
			Last = Mid-1;
		}
	}

	return 0;
}

BYTE GetPerceivedContext(CHAR* Extension)
{
	WCHAR ExtensionW[17];
	ExtensionW[0] = '.';
	MultiByteToWideChar(CP_ACP, 0, Extension, -1, &ExtensionW[1], 16);

	PERCEIVED Type;
	PERCEIVEDFLAG Flag;
	if (AssocGetPerceivedType(ExtensionW, &Type, &Flag, NULL)==S_OK)
		switch(Type)
		{
		case PERCEIVED_TYPE_IMAGE:
			return LFContextPictures;

		case PERCEIVED_TYPE_AUDIO:
			return LFContextAudio;

		case PERCEIVED_TYPE_VIDEO:
			return LFContextVideos;

		case PERCEIVED_TYPE_CONTACTS:
			return LFContextContacts;
		}

	return 0;
}

void SetFileContext(LFCoreAttributes* pCoreAttributes, BOOL Force)
{
	assert(pCoreAttributes);

	#ifdef _DEBUG
	// Test: ist die Kontext-Liste korrekt sortiert?
	for (UINT a=0; a<(sizeof(Registry)/sizeof(RegisteredFile))-1; a++)
		if (strcmp(Registry[a].Format, Registry[a+1].Format)>-1)
			MessageBoxA(NULL, Registry[a].Format, "Registry sort error", 0);
	#endif

	if ((!pCoreAttributes->ContextID) || Force)
		pCoreAttributes->ContextID = GetHardcodedContext(pCoreAttributes->FileFormat);

	if (!pCoreAttributes->ContextID)
		pCoreAttributes->ContextID = GetPerceivedContext(pCoreAttributes->FileFormat);
}


void SetNameExtFromFile(LFItemDescriptor* pItemDescriptor, WCHAR* pFilename)
{
	assert(pItemDescriptor);
	assert(pFilename);

	// Type
	pItemDescriptor->Type = (pItemDescriptor->Type & ~LFTypeMask) | LFTypeFile;

	// Name
	WCHAR Name[256];
	WCHAR* Ptr = wcsrchr(pFilename, L'\\');
	wcscpy_s(Name, 256, Ptr ? Ptr+1 : pFilename);

	// Erweiterung
	WCHAR* LastExt = wcsrchr(Name, L'.');
	if (LastExt)
	{
		CHAR Extension[LFExtSize] = { 0 };

		Ptr = LastExt+1;
		SIZE_T cCount = 0;
		while ((*Ptr!=L'\0') && (cCount<LFExtSize-1))
		{
			Extension[cCount++] = (*Ptr<=0xFF) ? tolower(*Ptr) & 0xFF : L'_';
			Ptr++;
		}

		SetAttribute(pItemDescriptor, LFAttrFileFormat, Extension);

		*LastExt = L'\0';
	}

	SetAttribute(pItemDescriptor, LFAttrFileName, Name);
}


void SetFromFindData(LFCoreAttributes* pCoreAttributes, WIN32_FIND_DATA* pFindData)
{
	assert(pCoreAttributes);
	assert(pFindData);

	// Set attributes
	pCoreAttributes->FileSize = (((INT64)pFindData->nFileSizeHigh) << 32) | pFindData->nFileSizeLow;
	pCoreAttributes->CreationTime = pFindData->ftCreationTime;
	pCoreAttributes->FileTime = pFindData->ftLastWriteTime;

	// Adjust files with modification time older than creation time
	ULARGE_INTEGER CreationTime;
	CreationTime.LowPart = pCoreAttributes->CreationTime.dwLowDateTime;
	CreationTime.HighPart = pCoreAttributes->CreationTime.dwHighDateTime;

	ULARGE_INTEGER FileTime;
	FileTime.LowPart = pCoreAttributes->FileTime.dwLowDateTime;
	FileTime.HighPart = pCoreAttributes->FileTime.dwHighDateTime;

	if (CreationTime.QuadPart>FileTime.QuadPart)
		pCoreAttributes->CreationTime = pCoreAttributes->FileTime;
}


void GetShellProperty(IShellFolder2* pParentFolder, LPCITEMIDLIST pidlRel, GUID Schema, UINT ID, LFItemDescriptor* pItemDescriptor, UINT Attr)
{
	SHCOLUMNID Column = { Schema, ID };
	VARIANT Value = { 0 };
	
	if (SUCCEEDED(pParentFolder->GetDetailsEx(pidlRel, &Column, &Value)))
		switch(Value.vt)
		{
		case VT_BSTR:
			if ((AttrTypes[Attr]==LFTypeUnicodeString) || (AttrTypes[Attr]==LFTypeUnicodeArray))
				SetAttribute(pItemDescriptor, Attr, Value.pbstrVal);

			break;

		case VT_I4:
			if (((AttrTypes[Attr]==LFTypeUINT) || (AttrTypes[Attr]==LFTypeBitrate)) && (Value.intVal>=0))
				SetAttribute(pItemDescriptor, Attr, &Value.intVal);

			break;

		case VT_UI4:
			if ((AttrTypes[Attr]==LFTypeUINT) || (AttrTypes[Attr]==LFTypeFourCC))
				SetAttribute(pItemDescriptor, Attr, &Value.uintVal);

			break;

		case VT_I8:
		case VT_UI8:
			switch(AttrTypes[Attr])
			{
			case LFTypeSize:
				SetAttribute(pItemDescriptor, Attr, &Value.ullVal);
				break;

			case LFTypeDuration:
				Value.ullVal /= 10000;
				UINT32 Duration = (Value.ullVal>0xFFFFFFFF) ? 0xFFFFFFFF : (UINT32)Value.ullVal;
				SetAttribute(pItemDescriptor, Attr, &Duration);
				break;
			}

			break;

		case VT_R8:
			if (AttrTypes[Attr]==LFTypeDouble)
				SetAttribute(pItemDescriptor, Attr, &Value.dblVal);

			break;

		case VT_DATE:
			if (AttrTypes[Attr]==LFTypeTime)
			{
				SYSTEMTIME st;
				FILETIME ft;

				VariantTimeToSystemTime(Value.date, &st);
				SystemTimeToFileTime(&st, &ft);
				SetAttribute(pItemDescriptor, Attr, &ft);
			}

			break;
		}
}

void GetOLEProperties(IPropertySetStorage* pPropertySetStorage, FMTID Schema, LFItemDescriptor* pItemDescriptor)
{
	IPropertyStorage *pPropertyStorage;
	if (SUCCEEDED(pPropertySetStorage->Open(Schema, STGM_READ | STGM_SHARE_EXCLUSIVE, &pPropertyStorage)))
	{
		PROPSPEC PropertySpec;
		PropertySpec.ulKind = PRSPEC_PROPID;

		for (UINT Attr=0; Attr<LFAttributeCount; Attr++)
			if (AttrProperties[Attr].Schema==Schema)
			{
				PropertySpec.propid = AttrProperties[Attr].ID;

				PROPVARIANT Value;
				if (pPropertyStorage->ReadMultiple(1, &PropertySpec, &Value)==S_OK)
					switch(Value.vt)
					{
					case VT_BSTR:
						if ((AttrTypes[Attr]==LFTypeUnicodeString) || (AttrTypes[Attr]==LFTypeUnicodeArray))
							SetAttribute(pItemDescriptor, Attr, Value.pbstrVal);

						break;

					case VT_LPWSTR:
						if ((AttrTypes[Attr]==LFTypeUnicodeString) || (AttrTypes[Attr]==LFTypeUnicodeArray))
							SetAttribute(pItemDescriptor, Attr, Value.pwszVal);

						break;

					case VT_LPSTR:
						if (AttrTypes[Attr]==LFTypeAnsiString)
							SetAttribute(pItemDescriptor, Attr, Value.pszVal);

						if ((AttrTypes[Attr]==LFTypeUnicodeString) || (AttrTypes[Attr]==LFTypeUnicodeArray))
						{
							WCHAR tmpStr[256];
							MultiByteToWideChar(CP_ACP, 0, Value.pszVal, -1, tmpStr, 256);

							SetAttribute(pItemDescriptor, Attr, tmpStr);
						}

						break;

					case VT_DATE:
						if (AttrTypes[Attr]==LFTypeTime)
						{
							SYSTEMTIME st;
							FILETIME ft;

							VariantTimeToSystemTime(Value.date, &st);
							SystemTimeToFileTime(&st, &ft);
							SetAttribute(pItemDescriptor, Attr, &ft);
						}

						break;
					}
			}

		pPropertyStorage->Release();
	}
}

void SetAttributesFromFile(LFItemDescriptor* pItemDescriptor, WCHAR* pPath, BOOL Metadata)
{
	assert(pItemDescriptor);
	assert(pPath);

	// Standard attributes
	//
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(pPath, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
		SetFromFindData(&pItemDescriptor->CoreAttributes, &FindFileData);

	FindClose(hFind);

	// Context
	SetFileContext(&pItemDescriptor->CoreAttributes);

	// Slave
	assert(pItemDescriptor->CoreAttributes.ContextID<=LFLastQueryContext);

	pItemDescriptor->CoreAttributes.SlaveID = ContextSlaves[pItemDescriptor->CoreAttributes.ContextID];

	// Exit if no additional metadata is requested
	if (!pItemDescriptor->CoreAttributes.SlaveID)
		return;

	if (Metadata)
	{
		// Shell properties
		//
		LPITEMIDLIST pidlFQ;
		if (SUCCEEDED(SHParseDisplayName(pPath, NULL, &pidlFQ, 0, NULL)))
		{
			IShellFolder2* pParentFolder;
			LPCITEMIDLIST pidlRel;
			if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder2, (void**)&pParentFolder, &pidlRel)))
			{
				for (UINT a=0; a<LFAttributeCount; a++)
					if ((AttrProperties[a].ID) && (a!=LFAttrFileName) && (a!=LFAttrFileSize) && (a!=LFAttrFileFormat) && (a!=LFAttrCreationTime) && (a!=LFAttrFileTime))
						GetShellProperty(pParentFolder, pidlRel, AttrProperties[a].Schema, AttrProperties[a].ID, pItemDescriptor, a);

				// Besondere Eigenschaften
				GetShellProperty(pParentFolder, pidlRel, PropertyAudio, 4, pItemDescriptor, LFAttrBitrate);
				GetShellProperty(pParentFolder, pidlRel, PropertyAudio, 5, pItemDescriptor, LFAttrSamplerate);
				GetShellProperty(pParentFolder, pidlRel, PropertyAudio, 7, pItemDescriptor, LFAttrChannels);
				GetShellProperty(pParentFolder, pidlRel, PropertyAudio, 10, pItemDescriptor, LFAttrAudioCodec);

				GetShellProperty(pParentFolder, pidlRel, PropertyPhoto, 36867, pItemDescriptor, LFAttrRecordingTime);

				GetShellProperty(pParentFolder, pidlRel, PropertyVideo, 3, pItemDescriptor, LFAttrWidth);
				GetShellProperty(pParentFolder, pidlRel, PropertyVideo, 4, pItemDescriptor, LFAttrHeight);
				GetShellProperty(pParentFolder, pidlRel, PropertyVideo, 8, pItemDescriptor, LFAttrBitrate);

				pParentFolder->Release();
			}

			CoTaskMemFree(pidlFQ);
		}

		// OLE structured storage
		//
		if (pItemDescriptor->CoreAttributes.ContextID==LFContextDocuments)
		{
			IPropertySetStorage* pPropertySetStorage;
			if (SUCCEEDED(StgOpenStorageEx(pPath, STGM_DIRECT | STGM_SHARE_EXCLUSIVE | STGM_READ, STGFMT_ANY, 0, NULL, NULL, IID_IPropertySetStorage, (void**)&pPropertySetStorage)))
			{
				GetOLEProperties(pPropertySetStorage, PropertyDocuments, pItemDescriptor);
				GetOLEProperties(pPropertySetStorage, PropertyMedia, pItemDescriptor);
				GetOLEProperties(pPropertySetStorage, PropertyMusic, pItemDescriptor);
				GetOLEProperties(pPropertySetStorage, PropertyPhoto, pItemDescriptor);
				GetOLEProperties(pPropertySetStorage, PropertySummary, pItemDescriptor);

				pPropertySetStorage->Release();
			}
		}

		// TODO: weitere Attribute durch eigene Metadaten-Bibliothek
	}

	// Properties from filename
	//
	if ((pItemDescriptor->CoreAttributes.ContextID>=LFContextAudio) && (pItemDescriptor->CoreAttributes.ContextID<=LFContextVideos))
	{
		WCHAR* pSeparator = wcsstr(pItemDescriptor->CoreAttributes.FileName, L" – ");
		SIZE_T SeparatorLength = 3;

		if (!pSeparator)
		{
			pSeparator = wcschr(pItemDescriptor->CoreAttributes.FileName, L'—');
			SeparatorLength = 1;
		}

		if (pSeparator)
		{
			// Artist or Roll
			WCHAR Value[256];
			wcsncpy_s(Value, 256, pItemDescriptor->CoreAttributes.FileName, pSeparator-pItemDescriptor->CoreAttributes.FileName);

			for (WCHAR* pChar=Value; pChar; pChar++)
				if (*pChar>=L'A')
				{
					const UINT Attr = (pItemDescriptor->CoreAttributes.ContextID>LFContextAudio) && LFIsNullAttribute(pItemDescriptor, LFAttrRoll) ? LFAttrRoll : LFAttrArtist;
					SetAttribute(pItemDescriptor, Attr, Value);

					break;
				}

			// Title
			WCHAR* pTitle = pSeparator+SeparatorLength;
			if (*pTitle)
				SetAttribute(pItemDescriptor, LFAttrTitle, pTitle);
		}
	}
}

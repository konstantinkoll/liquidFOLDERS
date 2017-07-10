
#include "stdafx.h"
#include "AttributeTables.h"
#include "ID3.h"
#include "IndexTables.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "ShellProperties.h"
#include <assert.h>
#include <shlobj.h>
#include <shlwapi.h>


#pragma data_seg(".shared")

#include "ContextTable.h"

static const BYTE ContextSlaves[LFLastQueryContext+1] = {
	IDXTABLE_MASTER,		// LFContextAllFiles
	IDXTABLE_MASTER,		// LFContextFavorites
	IDXTABLE_AUDIO,			// LFContextAudio
	IDXTABLE_PICTURES,		// LFContextPictures
	IDXTABLE_VIDEOS,		// LFContextVideos
	IDXTABLE_DOCUMENTS,		// LFContextDocuments
	IDXTABLE_MASTER,		// LFContextContacts
	IDXTABLE_MESSAGES,		// LFContextMessages
	IDXTABLE_MASTER,		// LFContextTasks
	IDXTABLE_MASTER,		// LFContextNew
	IDXTABLE_MASTER,		// LFContextTrash
	IDXTABLE_MASTER			// LFContextFilters
};

#pragma data_seg()


#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")


BYTE GetHardcodedContext(LPCSTR Extension)
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

BYTE GetPerceivedContext(LPCSTR Extension)
{
	WCHAR ExtensionW[17];
	ExtensionW[0] = '.';
	MultiByteToWideChar(CP_ACP, 0, Extension, -1, &ExtensionW[1], 16);

	PERCEIVED Type;
	PERCEIVEDFLAG Flag;
	if (AssocGetPerceivedType(ExtensionW, &Type, &Flag, NULL)==S_OK)
		switch (Type)
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


void SetNameExtFromFile(LFItemDescriptor* pItemDescriptor, LPCWSTR pFilename)
{
	assert(pItemDescriptor);
	assert(pFilename);

	// Type
	pItemDescriptor->Type = (pItemDescriptor->Type & ~LFTypeMask) | LFTypeFile;

	// Name
	WCHAR Name[256];
	LPCWSTR pChar = wcsrchr(pFilename, L'\\');
	wcscpy_s(Name, 256, pChar ? pChar+1 : pFilename);

	// Erweiterung
	WCHAR* pLastExt = wcsrchr(Name, L'.');
	if (pLastExt)
	{
		CHAR Extension[LFExtSize] = { 0 };

		pChar = pLastExt+1;
		SIZE_T cCount = 0;
		while ((*pChar!=L'\0') && (cCount<LFExtSize-1))
		{
			Extension[cCount++] = (*pChar<=0xFF) ? tolower(*pChar) & 0xFF : L'_';
			pChar++;
		}

		SetAttribute(pItemDescriptor, LFAttrFileFormat, Extension);

		*pLastExt = L'\0';
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
		switch (Value.vt)
		{
		case VT_BSTR:
			if ((AttrProperties[Attr].Type==LFTypeUnicodeString) || (AttrProperties[Attr].Type==LFTypeUnicodeArray))
				SetAttribute(pItemDescriptor, Attr, Value.pbstrVal);

			break;

		case 8200:
			if (Attr==LFAttrGenre)
			{
				BSTR HUGEP *pbstr;
				if (SUCCEEDED(SafeArrayAccessData(Value.parray, (void HUGEP**)&pbstr)))
				{
					const UINT Genre = FindMusicGenre(pbstr[0]);
					SetAttribute(pItemDescriptor, Attr, &Genre);
				}
			}

			break;

		case VT_I4:
			if (((AttrProperties[Attr].Type==LFTypeUINT) || (AttrProperties[Attr].Type==LFTypeBitrate)) && (Value.intVal>=0))
				SetAttribute(pItemDescriptor, Attr, &Value.intVal);

			break;

		case VT_UI4:
			if ((AttrProperties[Attr].Type==LFTypeUINT) || (AttrProperties[Attr].Type==LFTypeFourCC))
				SetAttribute(pItemDescriptor, Attr, &Value.uintVal);

			break;

		case VT_I8:
		case VT_UI8:
			switch (AttrProperties[Attr].Type)
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
			if (AttrProperties[Attr].Type==LFTypeDouble)
				SetAttribute(pItemDescriptor, Attr, &Value.dblVal);

			break;

		case VT_DATE:
			if (AttrProperties[Attr].Type==LFTypeTime)
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
			if (AttrProperties[Attr].ShPropertyMapping.Schema==Schema)
			{
				PropertySpec.propid = AttrProperties[Attr].ShPropertyMapping.ID;

				PROPVARIANT Value;
				if (pPropertyStorage->ReadMultiple(1, &PropertySpec, &Value)==S_OK)
					switch (Value.vt)
					{
					case VT_BSTR:
						if ((AttrProperties[Attr].Type==LFTypeUnicodeString) || (AttrProperties[Attr].Type==LFTypeUnicodeArray))
							SetAttribute(pItemDescriptor, Attr, Value.pbstrVal);

						break;

					case VT_LPWSTR:
						if ((AttrProperties[Attr].Type==LFTypeUnicodeString) || (AttrProperties[Attr].Type==LFTypeUnicodeArray))
							SetAttribute(pItemDescriptor, Attr, Value.pwszVal);

						break;

					case VT_LPSTR:
						if ((AttrProperties[Attr].Type==LFTypeAnsiString) || (AttrProperties[Attr].Type==LFTypeIATACode))
							SetAttribute(pItemDescriptor, Attr, Value.pszVal);

						if ((AttrProperties[Attr].Type==LFTypeUnicodeString) || (AttrProperties[Attr].Type==LFTypeUnicodeArray))
						{
							WCHAR tmpStr[256];
							MultiByteToWideChar(CP_ACP, 0, Value.pszVal, -1, tmpStr, 256);

							SetAttribute(pItemDescriptor, Attr, tmpStr);
						}

						break;

					case VT_DATE:
						if (AttrProperties[Attr].Type==LFTypeTime)
						{
							SYSTEMTIME st;
							FILETIME ft;

							VariantTimeToSystemTime(Value.date, &st);
							SystemTimeToFileTime(&st, &ft);
							SetAttribute(pItemDescriptor, Attr, &ft);
						}

						break;

					default:
						;
					}
			}

		pPropertyStorage->Release();
	}
}

void SetAttributesFromFile(LFItemDescriptor* pItemDescriptor, LPCWSTR pPath, BOOL Metadata)
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
			if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder2, (LPVOID*)&pParentFolder, &pidlRel)))
			{
				for (UINT a=0; a<LFAttributeCount; a++)
					if ((AttrProperties[a].ShPropertyMapping.ID) && (a!=LFAttrFileName) && (a!=LFAttrFileSize) && (a!=LFAttrFileFormat) && (a!=LFAttrCreationTime) && (a!=LFAttrFileTime))
						GetShellProperty(pParentFolder, pidlRel, AttrProperties[a].ShPropertyMapping.Schema, AttrProperties[a].ShPropertyMapping.ID, pItemDescriptor, a);

				// Besondere Eigenschaften
				GetShellProperty(pParentFolder, pidlRel, SHPropertyAudio, 4, pItemDescriptor, LFAttrBitrate);
				GetShellProperty(pParentFolder, pidlRel, SHPropertyAudio, 5, pItemDescriptor, LFAttrSamplerate);
				GetShellProperty(pParentFolder, pidlRel, SHPropertyAudio, 7, pItemDescriptor, LFAttrChannels);

				GetShellProperty(pParentFolder, pidlRel, SHPropertyPhoto, 36867, pItemDescriptor, LFAttrRecordingTime);

				GetShellProperty(pParentFolder, pidlRel, SHPropertyVideo, 3, pItemDescriptor, LFAttrWidth);
				GetShellProperty(pParentFolder, pidlRel, SHPropertyVideo, 4, pItemDescriptor, LFAttrHeight);
				GetShellProperty(pParentFolder, pidlRel, SHPropertyVideo, 8, pItemDescriptor, LFAttrBitrate);

				pParentFolder->Release();
			}

			CoTaskMemFree(pidlFQ);
		}

		// OLE structured storage
		//
		if (pItemDescriptor->CoreAttributes.ContextID==LFContextDocuments)
		{
			IPropertySetStorage* pPropertySetStorage;
			if (SUCCEEDED(StgOpenStorageEx(pPath, STGM_DIRECT | STGM_SHARE_EXCLUSIVE | STGM_READ, STGFMT_ANY, 0, NULL, NULL, IID_IPropertySetStorage, (LPVOID*)&pPropertySetStorage)))
			{
				GetOLEProperties(pPropertySetStorage, SHPropertyDocuments, pItemDescriptor);
				GetOLEProperties(pPropertySetStorage, SHPropertyMedia, pItemDescriptor);
				GetOLEProperties(pPropertySetStorage, SHPropertyMusic, pItemDescriptor);
				GetOLEProperties(pPropertySetStorage, SHPropertyPhoto, pItemDescriptor);
				GetOLEProperties(pPropertySetStorage, SHPropertySummary, pItemDescriptor);

				pPropertySetStorage->Release();
			}
		}

		// TODO: weitere Attribute durch eigene Metadaten-Bibliothek
	}

	// Properties from filename
	//
	if ((pItemDescriptor->CoreAttributes.ContextID>=LFContextAudio) && (pItemDescriptor->CoreAttributes.ContextID<=LFContextVideos))
	{
		LPCWSTR pSeparator = wcsstr(pItemDescriptor->CoreAttributes.FileName, L" – ");
		SIZE_T SeparatorLength = 3;

		if (!pSeparator)
		{
			pSeparator = wcschr(pItemDescriptor->CoreAttributes.FileName, L'—');
			SeparatorLength = 1;
		}

		if (!pSeparator)
		{
			if ((pSeparator=wcsrchr(pItemDescriptor->CoreAttributes.FileName, L' '))!=NULL)
			{
				// Only (and at least one) numbers following the right-most space?
				LPCWSTR pChar = pSeparator+1;

				do
				{
					if ((*pChar<L'0') || (*pChar>L'9'))
					{
						// No, so no separator!
						pSeparator = NULL;

						break;
					}
				}
				while (*(++pChar));
			}

			SeparatorLength = 1;
		}

		if (pSeparator)
		{
			// Artist or Roll
			WCHAR Value[256];
			wcsncpy_s(Value, 256, pItemDescriptor->CoreAttributes.FileName, pSeparator-pItemDescriptor->CoreAttributes.FileName);

			for (LPCWSTR pChar=Value; pChar; pChar++)
				if (*pChar>=L'A')
				{
					const UINT Attr = (pItemDescriptor->CoreAttributes.ContextID>LFContextAudio) && LFIsNullAttribute(pItemDescriptor, LFAttrRoll) ? LFAttrRoll : LFAttrArtist;
					SetAttribute(pItemDescriptor, Attr, Value);

					break;
				}

			// Title
			LPCWSTR pTitle = pSeparator+SeparatorLength;
			if (*pTitle)
				SetAttribute(pItemDescriptor, LFAttrTitle, pTitle);
		}
	}
}


#include "stdafx.h"
#include "FileProperties.h"
#include "TableIndexes.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "TableApplications.h"
#include "TableAttributes.h"
#include "TableContexts.h"
#include "TableMusicGenres.h"
#include <assert.h>
#include <shlobj.h>
#include <shlwapi.h>


#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")


// Context handling
//

void GetHardcodedContext(LPCSTR Extension, BYTE& SystemContextID, BYTE& UserContextID)
{
	INT First = 0;
	INT Last = FILEFORMATCOUNT-1;

	while (First<=Last)
	{
		const INT Mid = (First+Last)/2;

		const INT Result = strcmp(ContextRegistry[Mid].Format, Extension);
		if (Result==0)
		{
			SystemContextID = ContextRegistry[Mid].SystemContextID;
			UserContextID = ContextRegistry[Mid].UserContextID;

			assert(ContextMoveAllowed(SystemContextID, UserContextID));

			return;
		}

		if (Result<0)
		{
			First = Mid+1;
		}
		else
		{
			Last = Mid-1;
		}
	}
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

		case PERCEIVED_TYPE_APPLICATION:
			return LFContextApps;
	}

	return 0;
}

void SetFileContext(LFCoreAttributes& CoreAttributes, BOOL OnImport)
{
#ifdef _DEBUG
	// Is the context list sorted?
	for (UINT a=0; a<(sizeof(ContextRegistry)/sizeof(RegisteredFileFormat))-1; a++)
		assert(strcmp(ContextRegistry[a].Format, ContextRegistry[a+1].Format)<1);
#endif

	// Find context
	BYTE SystemContextID = 0;
	BYTE UserContextID = 0;
	GetHardcodedContext(CoreAttributes.FileFormat, SystemContextID, UserContextID);

	assert(ContextMoveAllowed(SystemContextID, UserContextID));

	if (!SystemContextID)
		SystemContextID = GetPerceivedContext(CoreAttributes.FileFormat);

	if (LFGetSystemContextID(CoreAttributes)!=SystemContextID)
	{
		// New system context: set both system and user context
		CoreAttributes.SystemContextID = SystemContextID;
		CoreAttributes.UserContextID = UserContextID;
	}
	else
		if (CoreAttributes.UserContextID && !(CtxProperties[SystemContextID].AllowMoveToContext & (1ull<<UserContextID)))
		{
			// Illegal user context: set new user context
			CoreAttributes.UserContextID = UserContextID;
		}

	// Slave
	if (OnImport)
	{
		assert(SystemContextID<=LFLastPersistentContext);
		assert(!CoreAttributes.SlaveID);

		CoreAttributes.SlaveID = ContextSlaves[SystemContextID];
	}
}


// File system handling
//

void SetAttributesFromFindFileData(LFCoreAttributes& CoreAttributes, WIN32_FIND_DATA& FindData)
{
	// Set attributes
	CoreAttributes.FileSize = (((INT64)FindData.nFileSizeHigh) << 32) | FindData.nFileSizeLow;
	CoreAttributes.CreationTime = FindData.ftCreationTime;
	CoreAttributes.FileTime = FindData.ftLastWriteTime;

	// Hidden flag
	if (FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
		CoreAttributes.Flags = (CoreAttributes.Flags & ~LFFlagNew) | LFFlagArchive;

	// Adjust files with modification time older than creation time
	ULARGE_INTEGER CreationTime;
	CreationTime.LowPart = CoreAttributes.CreationTime.dwLowDateTime;
	CreationTime.HighPart = CoreAttributes.CreationTime.dwHighDateTime;

	ULARGE_INTEGER FileTime;
	FileTime.LowPart = CoreAttributes.FileTime.dwLowDateTime;
	FileTime.HighPart = CoreAttributes.FileTime.dwHighDateTime;

	if (CreationTime.QuadPart>FileTime.QuadPart)
		CoreAttributes.CreationTime = CoreAttributes.FileTime;
}

void SetAttributesFromFindFileData(LFCoreAttributes& CoreAttributes, LPCWSTR pPath)
{
	assert(pPath);

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(pPath, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
		SetAttributesFromFindFileData(CoreAttributes, FindFileData);

	FindClose(hFind);
}


// Annotation handling
//

BOOL SetAttributesFromAnnotation(LFItemDescriptor* pItemDescriptor, LPCWSTR pAnnotation)
{
	assert(pItemDescriptor);
	assert(pAnnotation);

	for (BYTE a=0; a<APPLICATIONCOUNT; a++)
		if (_wcsicmp(ApplicationRegistry[a].Name, pAnnotation)==0)
		{
			SetAttribute(pItemDescriptor, LFAttrApplication, &ApplicationRegistry[a].ApplicationID);

			return TRUE;
		}

	return FALSE;
}


// Shell property handling
//

void GetShellProperty(IShellFolder2* pParentFolder, LPCITEMIDLIST pidlRel, LPCGUID Schema, UINT ID, LFItemDescriptor* pItemDescriptor, UINT Attr)
{
	assert(pParentFolder);

	SHCOLUMNID Column = { *Schema, ID };
	VARIANT Value = { 0 };

	if (SUCCEEDED(pParentFolder->GetDetailsEx(pidlRel, &Column, &Value)))
		switch (Value.vt)
		{
		case VT_BSTR:
			if ((AttrProperties[Attr].Type==LFTypeUnicodeString) || (AttrProperties[Attr].Type==LFTypeUnicodeArray))
				SetAttribute(pItemDescriptor, Attr, Value.pbstrVal);

			break;

		case 8200:
			if (AttrProperties[Attr].Type==LFTypeGenre)
			{
				BSTR HUGEP *pbStr;
				if (SUCCEEDED(SafeArrayAccessData(Value.parray, (void HUGEP**)&pbStr)))
				{
					const UINT Genre = FindMusicGenre(pbStr[0]);

					SetAttribute(pItemDescriptor, Attr, &Genre);
				}
			}

			break;

		case VT_I4:
			if (Value.intVal<0)
				break;

		case VT_UI4:
			if ((AttrProperties[Attr].Type==LFTypeUINT) || (AttrProperties[Attr].Type==LFTypeBitrate) || (AttrProperties[Attr].Type==LFTypeYear) || (AttrProperties[Attr].Type==LFTypeFramerate) || (AttrProperties[Attr].Type==LFTypeFourCC))
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
				const UINT32 Duration = (Value.ullVal>0xFFFFFFFF) ? 0xFFFFFFFF : (UINT32)Value.ullVal;

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
			if (*AttrProperties[Attr].ShPropertyMapping.Schema==Schema)
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
					}
			}

		pPropertyStorage->Release();
	}
}

void SetAttributesFromShell(LFItemDescriptor* pItemDescriptor, LPCWSTR pPath, BOOL OnImport)
{
	assert(pItemDescriptor);
	assert(pPath);

	// Exit if no additional metadata is requested
	if (!pItemDescriptor->CoreAttributes.SlaveID)
		return;

	// Shell properties
	//
	LPITEMIDLIST pidlFQ;
	if (SUCCEEDED(SHParseDisplayName(pPath, NULL, &pidlFQ, 0, NULL)))
	{
		IShellFolder2* pParentFolder;
		LPCITEMIDLIST pidlRel;
		if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder2, (LPVOID*)&pParentFolder, &pidlRel)))
		{
			for (UINT a=(OnImport ? 0 : LFLastCoreAttribute+1); a<LFAttributeCount; a++)
				if (AttrProperties[a].ShPropertyMapping.ID && (a!=LFAttrFileName) && (a!=LFAttrFileSize) && (a!=LFAttrFileFormat) && (a!=LFAttrCreationTime) && (a!=LFAttrFileTime))
					GetShellProperty(pParentFolder, pidlRel, AttrProperties[a].ShPropertyMapping.Schema, AttrProperties[a].ShPropertyMapping.ID, pItemDescriptor, a);

			// Secondary properties
			GetShellProperty(pParentFolder, pidlRel, &SHPropertyAudio, 4, pItemDescriptor, LFAttrBitrate);
			GetShellProperty(pParentFolder, pidlRel, &SHPropertyAudio, 5, pItemDescriptor, LFAttrSamplerate);
			GetShellProperty(pParentFolder, pidlRel, &SHPropertyAudio, 7, pItemDescriptor, LFAttrChannels);

			GetShellProperty(pParentFolder, pidlRel, &SHPropertyMedia, 32, pItemDescriptor, LFAttrURL);

			GetShellProperty(pParentFolder, pidlRel, &SHPropertyMusic, 2, pItemDescriptor, LFAttrCreator);

			GetShellProperty(pParentFolder, pidlRel, &SHPropertyPhoto, 18248, pItemDescriptor, LFAttrMediaCollection);
			GetShellProperty(pParentFolder, pidlRel, &SHPropertyPhoto, 36867, pItemDescriptor, LFAttrRecordingTime);

			GetShellProperty(pParentFolder, pidlRel, &SHPropertySummary, 4, pItemDescriptor, LFAttrCreator);

			GetShellProperty(pParentFolder, pidlRel, &SHPropertyVideo, 3, pItemDescriptor, LFAttrWidth);
			GetShellProperty(pParentFolder, pidlRel, &SHPropertyVideo, 4, pItemDescriptor, LFAttrHeight);
			GetShellProperty(pParentFolder, pidlRel, &SHPropertyVideo, 8, pItemDescriptor, LFAttrBitrate);

			pParentFolder->Release();
		}

		CoTaskMemFree(pidlFQ);
	}

	// OLE structured storage
	//
	if (LFIsDocumentFile(pItemDescriptor))
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

	// Fix broken properties
	//

	// Amazon appends " [Explicit]" to certain album names; remove it!
	LFVariantData VData;
	LFGetAttributeVariantDataEx(pItemDescriptor, LFAttrMediaCollection, VData);
	if (!LFIsNullVariantData(VData))
	{
		LPWSTR pSubstr = StrStrI(VData.UnicodeString, L" [EXPLICIT]");
		if (pSubstr)
		{
			*pSubstr = L'\0';
			LFSetAttributeVariantData(pItemDescriptor, VData);
		}
	}
}

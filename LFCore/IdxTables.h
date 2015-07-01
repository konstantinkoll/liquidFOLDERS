
#pragma once
#include "CHeapfile.h"


// Version
//

#define CurIdxVersion        5


// Structures
//

#define IDMaster             0
#define IDSlaveDocuments     1
#define IDSlaveMessages      2
#define IDSlaveAudio         3
#define IDSlavePictures      4
#define IDSlaveVideos        5

#define IdxTableCount        6


struct LFDocumentAttributes
{
	WCHAR Author[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR Responsible[256];
	FILETIME DueTime;
	FILETIME DoneTime;
	CHAR Signature[32];
	CHAR ISBN[32];
	UINT Pages;
	CHAR Language[3];
	WCHAR Customer[256];
};

struct LFMessageAttributes
{
	CHAR From[256];
	CHAR To[256];
	WCHAR Subject[256];
	CHAR Language[3];
	WCHAR Responsible[256];
	FILETIME DueTime;
	FILETIME DoneTime;
};

struct LFAudioAttributes
{
	WCHAR Artist[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR Album[256];
	UINT AudioCodec;
	UINT Channels;
	UINT Samplerate;
	UINT Duration;
	UINT Bitrate;
	FILETIME RecordingTime;
	CHAR Language[3];
};

struct LFPictureAttributes
{
	WCHAR Artist[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR Equipment[256];
	WCHAR Roll[256];
	WCHAR Exposure[32];
	UINT Height;
	UINT Width;
	LFFraction Aperture;
	LFFraction Focus;
	WCHAR Chip[32];
	FILETIME RecordingTime;
	CHAR Language[3];
	WCHAR Customer[256];
};

struct LFVideoAttributes
{
	WCHAR Artist[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR Equipment[256];
	WCHAR Roll[256];
	UINT Height;
	UINT Width;
	UINT AudioCodec;
	UINT VideoCodec;
	UINT Channels;
	UINT Samplerate;
	UINT Duration;
	UINT Bitrate;
	FILETIME RecordingTime;
	CHAR Language[3];
	WCHAR Customer[256];
};


// CIdxTableMaster
//

class CIdxTableMaster : public CHeapfile
{
public:
	CIdxTableMaster(WCHAR* Path);

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};


// CIdxTableDocuments
//

class CIdxTableDocuments : public CHeapfile
{
public:
	CIdxTableDocuments(WCHAR* Path);

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};


// CIdxTableMessages
//

class CIdxTableMessages : public CHeapfile
{
public:
	CIdxTableMessages(WCHAR* Path);

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};


// CIdxTableAudio
//

class CIdxTableAudio : public CHeapfile
{
public:
	CIdxTableAudio(WCHAR* Path);

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};


// CIdxTablePictures
//

class CIdxTablePictures : public CHeapfile
{
public:
	CIdxTablePictures(WCHAR* Path);

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};


// CIdxTableVideos
//

class CIdxTableVideos : public CHeapfile
{
public:
	CIdxTableVideos(WCHAR* Path);

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};

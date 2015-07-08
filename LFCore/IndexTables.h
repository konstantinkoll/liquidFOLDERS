
#pragma once
#include "LF.h"


// Core-ID: 0

#define IDXTABLE_MASTER     0
#define IDXTABLECOUNT       6

#define CURIDXVERSION       2


// Slave-ID: 1

#define IDXTABLE_DOCUMENTS     1

struct LFDocumentAttributes
{
	WCHAR Artist[256];
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


// Slave-ID: 2

#define IDXTABLE_MESSAGES     2

struct LFMessageAttributes
{
	CHAR From[256];
	CHAR To[256];
	WCHAR Title[256];
	CHAR Language[3];
	WCHAR Responsible[256];
	FILETIME DueTime;
	FILETIME DoneTime;
};


// Slave-ID: 3

#define IDXTABLE_AUDIO     3

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


// Slave-ID: 4

#define IDXTABLE_PICTURES     4

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


// Slave-ID: 5

#define IDXTABLE_VIDEOS     5

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


// Data structures

struct LFIdxTableEntry
{
	UINT Attr;
	INT Offset;
};

struct LFIdxTable
{
	WCHAR FileName[13];
	UINT Size;
	UINT cTableEntries;
	const LFIdxTableEntry* pTableEntries;
};

extern const LFIdxTable LFIndexTables[];
extern const LFIdxTableEntry LFCoreAttributeEntries[];

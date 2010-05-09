
#pragma once
#include "CHeapfile.h"


// Version
//

#define IdxVersion                      1


// Structures
//

#define IDMaster                        0
#define IDSlaveDocuments                1
#define IDSlaveMails                    2
#define IDSlaveAudio                    3
#define IDSlavePictures                 4
#define IDSlaveVideos                   5

#define IdxTableCount                   6


struct LFDocumentAttributes
{
	wchar_t Author[256];
	wchar_t Copyright[256];
	wchar_t Title[256];
	wchar_t Responsible[256];
	FILETIME DueTime;
	FILETIME DoneTime;
	char Signature[32];
	char ISBN[32];
	unsigned int Pages;
	char Language[3];
};

struct LFMailAttributes
{
	char From[256];
	char To[256];
	wchar_t Subject[256];
	char Language[3];
	wchar_t Responsible[256];
	FILETIME DueTime;
	FILETIME DoneTime;
};

struct LFAudioAttributes
{
	wchar_t Artist[256];
	wchar_t Copyright[256];
	wchar_t Title[256];
	wchar_t Album[256];
	unsigned int AudioCodec;
	unsigned int Channels;
	unsigned int Samplerate;
	unsigned int Duration;
	unsigned int Bitrate;
	FILETIME RecordingTime;
	char Language[3];
};

struct LFPictureAttributes
{
	wchar_t Artist[256];
	wchar_t Copyright[256];
	wchar_t Title[256];
	wchar_t Equipment[256];
	wchar_t Roll[256];
	wchar_t Exposure[32];
	unsigned int Height;
	unsigned int Width;
	LFFraction Aperture;
	LFFraction Focus;
	wchar_t Chip[32];
	FILETIME RecordingTime;
	char Language[3];
};

struct LFVideoAttributes
{
	wchar_t Artist[256];
	wchar_t Copyright[256];
	wchar_t Title[256];
	wchar_t Equipment[256];
	wchar_t Roll[256];
	unsigned int Height;
	unsigned int Width;
	unsigned int AudioCodec;
	unsigned int VideoCodec;
	unsigned int Channels;
	unsigned int Samplerate;
	unsigned int Duration;
	unsigned int Bitrate;
	FILETIME RecordingTime;
	char Language[3];
};


// CIdxTableMaster
//

class CIdxTableMaster : public CHeapfile
{
public:
	CIdxTableMaster(char* Path, char* Filename);
	~CIdxTableMaster();

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};


// CIdxTableDocuments
//

class CIdxTableDocuments : public CHeapfile
{
public:
	CIdxTableDocuments(char* Path, char* Filename);
	~CIdxTableDocuments();

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};


// CIdxTableMails
//

class CIdxTableMails : public CHeapfile
{
public:
	CIdxTableMails(char* Path, char* Filename);
	~CIdxTableMails();

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};


// CIdxTableAudio
//

class CIdxTableAudio : public CHeapfile
{
public:
	CIdxTableAudio(char* Path, char* Filename);
	~CIdxTableAudio();

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};


// CIdxTablePictures
//

class CIdxTablePictures : public CHeapfile
{
public:
	CIdxTablePictures(char* Path, char* Filename);
	~CIdxTablePictures();

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};


// CIdxTableVideos
//

class CIdxTableVideos : public CHeapfile
{
public:
	CIdxTableVideos(char* Path, char* Filename);
	~CIdxTableVideos();

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);
};

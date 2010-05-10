
#pragma once
#include "stdafx.h"
#include "IdxTables.h"
#include "LFCore.h"
#include <assert.h>
#include <malloc.h>
#include <stddef.h>


inline void ZeroCopy(void* _Dst, rsize_t _DstSize, void* _Src, rsize_t _SrcSize)
{
	memcpy_s(_Dst, _DstSize, _Src, _SrcSize);

	if (_DstSize>_SrcSize)
	{
		char* P = (char*)_Dst+_SrcSize;
		ZeroMemory(P, _DstSize-_SrcSize);
	}
}

// CIdxTableMaster
//

CIdxTableMaster::CIdxTableMaster(char* Path, char* Filename)
	: CHeapfile(Path, Filename, sizeof(LFCoreAttributes), offsetof(LFCoreAttributes, FileID))
{
}

CIdxTableMaster::~CIdxTableMaster()
{
}

void CIdxTableMaster::GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i)
{
	assert(i);
	assert(PtrDst);

	ZeroCopy(PtrDst, Hdr.ElementSize, &i->CoreAttributes, sizeof(LFCoreAttributes));
}

void CIdxTableMaster::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(PtrSrc);

	ZeroCopy(&i->CoreAttributes, sizeof(LFCoreAttributes), PtrSrc, Hdr.ElementSize);
}


// CIdxTableDocuments
//

CIdxTableDocuments::CIdxTableDocuments(char* Path, char* Filename)
	: CHeapfile(Path, Filename, sizeof(LFDocumentAttributes))
{
}

CIdxTableDocuments::~CIdxTableDocuments()
{
}

void CIdxTableDocuments::GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i)
{
	assert(i);
	assert(PtrDst);

	if ((i->CoreAttributes.SlaveID==IDSlaveDocuments) && (i->Slave))
	{
		ZeroCopy(PtrDst, Hdr.ElementSize-LFKeySize, i->Slave, sizeof(LFDocumentAttributes));
	}
	else
	{
		GetAttribute(PtrDst, offsetof(LFDocumentAttributes, Author), LFAttrArtist, i);
		GetAttribute(PtrDst, offsetof(LFDocumentAttributes, Copyright), LFAttrCopyright, i);
		GetAttribute(PtrDst, offsetof(LFDocumentAttributes, Title), LFAttrTitle, i);
		GetAttribute(PtrDst, offsetof(LFDocumentAttributes, Responsible), LFAttrResponsible, i);
		GetAttribute(PtrDst, offsetof(LFDocumentAttributes, DueTime), LFAttrDueTime, i);
		GetAttribute(PtrDst, offsetof(LFDocumentAttributes, DoneTime), LFAttrDoneTime, i);
		GetAttribute(PtrDst, offsetof(LFDocumentAttributes, Signature), LFAttrSignature, i);
		GetAttribute(PtrDst, offsetof(LFDocumentAttributes, ISBN), LFAttrISBN, i);
		GetAttribute(PtrDst, offsetof(LFDocumentAttributes, Pages), LFAttrPages, i);
		GetAttribute(PtrDst, offsetof(LFDocumentAttributes, Language), LFAttrLanguage, i);
	}
}

void CIdxTableDocuments::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(!i->Slave);
	assert(i->CoreAttributes.SlaveID==IDSlaveDocuments);
	assert(PtrSrc);

	i->Slave = malloc(sizeof(LFDocumentAttributes));
	ZeroCopy(i->Slave, sizeof(LFDocumentAttributes), PtrSrc, Hdr.ElementSize-LFKeySize);

	i->AttributeValues[LFAttrArtist] = ((LFDocumentAttributes*)i->Slave)->Author;
	i->AttributeValues[LFAttrCopyright] = ((LFDocumentAttributes*)i->Slave)->Copyright;
	i->AttributeValues[LFAttrTitle] = ((LFDocumentAttributes*)i->Slave)->Title;
	i->AttributeValues[LFAttrResponsible] = ((LFDocumentAttributes*)i->Slave)->Responsible;
	i->AttributeValues[LFAttrDueTime] = &((LFDocumentAttributes*)i->Slave)->DueTime;
	i->AttributeValues[LFAttrDoneTime] = &((LFDocumentAttributes*)i->Slave)->DoneTime;
	i->AttributeValues[LFAttrSignature] = ((LFDocumentAttributes*)i->Slave)->Signature;
	i->AttributeValues[LFAttrISBN] = ((LFDocumentAttributes*)i->Slave)->ISBN;
	i->AttributeValues[LFAttrPages] = &((LFDocumentAttributes*)i->Slave)->Pages;
	i->AttributeValues[LFAttrLanguage] = ((LFDocumentAttributes*)i->Slave)->Language;
}


// CIdxTableMails
//

CIdxTableMails::CIdxTableMails(char* Path, char* Filename)
	: CHeapfile(Path, Filename, sizeof(LFMailAttributes))
{
}

CIdxTableMails::~CIdxTableMails()
{
}

void CIdxTableMails::GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i)
{
	assert(i);
	assert(PtrDst);

	if ((i->CoreAttributes.SlaveID==IDSlaveMails) && (i->Slave))
	{
		ZeroCopy(PtrDst, Hdr.ElementSize-LFKeySize, i->Slave, sizeof(LFMailAttributes));
	}
	else
	{
		GetAttribute(PtrDst, offsetof(LFMailAttributes, From), LFAttrFrom, i);
		GetAttribute(PtrDst, offsetof(LFMailAttributes, To), LFAttrTo, i);
		GetAttribute(PtrDst, offsetof(LFMailAttributes, Subject), LFAttrTitle, i);
		GetAttribute(PtrDst, offsetof(LFMailAttributes, Language), LFAttrLanguage, i);
		GetAttribute(PtrDst, offsetof(LFMailAttributes, Responsible), LFAttrResponsible, i);
		GetAttribute(PtrDst, offsetof(LFMailAttributes, DueTime), LFAttrDueTime, i);
		GetAttribute(PtrDst, offsetof(LFMailAttributes, DoneTime), LFAttrDoneTime, i);
	}
}

void CIdxTableMails::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(!i->Slave);
	assert(i->CoreAttributes.SlaveID==IDSlaveMails);
	assert(PtrSrc);

	i->Slave = malloc(sizeof(LFMailAttributes));
	ZeroCopy(i->Slave, sizeof(LFMailAttributes), PtrSrc, Hdr.ElementSize-LFKeySize);

	i->AttributeValues[LFAttrFrom] = ((LFMailAttributes*)i->Slave)->From;
	i->AttributeValues[LFAttrTo] = ((LFMailAttributes*)i->Slave)->To;
	i->AttributeValues[LFAttrTitle] = ((LFMailAttributes*)i->Slave)->Subject;
	i->AttributeValues[LFAttrLanguage] = ((LFMailAttributes*)i->Slave)->Language;
	i->AttributeValues[LFAttrResponsible] = ((LFMailAttributes*)i->Slave)->Responsible;
	i->AttributeValues[LFAttrDueTime] = &((LFMailAttributes*)i->Slave)->DueTime;
	i->AttributeValues[LFAttrDoneTime] = &((LFMailAttributes*)i->Slave)->DoneTime;
}


// CIdxTableAudio
//

CIdxTableAudio::CIdxTableAudio(char* Path, char* Filename)
	: CHeapfile(Path, Filename, sizeof(LFAudioAttributes))
{
}

CIdxTableAudio::~CIdxTableAudio()
{
}

void CIdxTableAudio::GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i)
{
	assert(i);
	assert(PtrDst);

	if ((i->CoreAttributes.SlaveID==IDSlaveAudio) && (i->Slave))
	{
		ZeroCopy(PtrDst, Hdr.ElementSize-LFKeySize, i->Slave, sizeof(LFAudioAttributes));
	}
	else
	{
		GetAttribute(PtrDst, offsetof(LFAudioAttributes, Artist), LFAttrArtist, i);
		GetAttribute(PtrDst, offsetof(LFAudioAttributes, Copyright), LFAttrCopyright, i);
		GetAttribute(PtrDst, offsetof(LFAudioAttributes, Title), LFAttrTitle, i);
		GetAttribute(PtrDst, offsetof(LFAudioAttributes, Album), LFAttrAlbum, i);
		GetAttribute(PtrDst, offsetof(LFAudioAttributes, AudioCodec), LFAttrAudioCodec, i);
		GetAttribute(PtrDst, offsetof(LFAudioAttributes, Channels), LFAttrChannels, i);
		GetAttribute(PtrDst, offsetof(LFAudioAttributes, Samplerate), LFAttrSamplerate, i);
		GetAttribute(PtrDst, offsetof(LFAudioAttributes, Duration), LFAttrDuration, i);
		GetAttribute(PtrDst, offsetof(LFAudioAttributes, Bitrate), LFAttrBitrate, i);
		GetAttribute(PtrDst, offsetof(LFAudioAttributes, RecordingTime), LFAttrRecordingTime, i);
		GetAttribute(PtrDst, offsetof(LFAudioAttributes, Language), LFAttrLanguage, i);
	}
}

void CIdxTableAudio::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(!i->Slave);
	assert(i->CoreAttributes.SlaveID==IDSlaveAudio);
	assert(PtrSrc);

	i->Slave = malloc(sizeof(LFAudioAttributes));
	ZeroCopy(i->Slave, sizeof(LFAudioAttributes), PtrSrc, Hdr.ElementSize-LFKeySize);

	i->AttributeValues[LFAttrArtist] = ((LFAudioAttributes*)i->Slave)->Artist;
	i->AttributeValues[LFAttrCopyright] = ((LFAudioAttributes*)i->Slave)->Copyright;
	i->AttributeValues[LFAttrTitle] = ((LFAudioAttributes*)i->Slave)->Title;
	i->AttributeValues[LFAttrAlbum] = ((LFAudioAttributes*)i->Slave)->Album;
	i->AttributeValues[LFAttrAudioCodec] = &((LFAudioAttributes*)i->Slave)->AudioCodec;
	i->AttributeValues[LFAttrChannels] = &((LFAudioAttributes*)i->Slave)->Channels;
	i->AttributeValues[LFAttrSamplerate] = &((LFAudioAttributes*)i->Slave)->Samplerate;
	i->AttributeValues[LFAttrDuration] = &((LFAudioAttributes*)i->Slave)->Duration;
	i->AttributeValues[LFAttrBitrate] = &((LFAudioAttributes*)i->Slave)->Bitrate;
	i->AttributeValues[LFAttrRecordingTime] = &((LFAudioAttributes*)i->Slave)->RecordingTime;
	i->AttributeValues[LFAttrLanguage] = ((LFAudioAttributes*)i->Slave)->Language;
}


// CIdxTablePictures
//

CIdxTablePictures::CIdxTablePictures(char* Path, char* Filename)
	: CHeapfile(Path, Filename, sizeof(LFPictureAttributes))
{
}

CIdxTablePictures::~CIdxTablePictures()
{
}

void CIdxTablePictures::GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i)
{
	assert(i);
	assert(PtrDst);

	if ((i->CoreAttributes.SlaveID==IDSlavePictures) && (i->Slave))
	{
		ZeroCopy(PtrDst, Hdr.ElementSize-LFKeySize, i->Slave, sizeof(LFPictureAttributes));
	}
	else
	{
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Artist), LFAttrArtist, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Copyright), LFAttrCopyright, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Title), LFAttrTitle, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Equipment), LFAttrRecordingEquipment, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Roll), LFAttrRoll, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Exposure), LFAttrExposure, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Height), LFAttrHeight, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Width), LFAttrWidth, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Aperture), LFAttrAperture, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Focus), LFAttrFocus, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Chip), LFAttrChip, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, RecordingTime), LFAttrRecordingTime, i);
		GetAttribute(PtrDst, offsetof(LFPictureAttributes, Language), LFAttrLanguage, i);
	}
}

void CIdxTablePictures::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(!i->Slave);
	assert(i->CoreAttributes.SlaveID==IDSlavePictures);
	assert(PtrSrc);

	i->Slave = malloc(sizeof(LFPictureAttributes));
	ZeroCopy(i->Slave, sizeof(LFPictureAttributes), PtrSrc, Hdr.ElementSize-LFKeySize);

	i->AttributeValues[LFAttrArtist] = ((LFPictureAttributes*)i->Slave)->Artist;
	i->AttributeValues[LFAttrCopyright] = ((LFPictureAttributes*)i->Slave)->Copyright;
	i->AttributeValues[LFAttrTitle] = ((LFPictureAttributes*)i->Slave)->Title;
	i->AttributeValues[LFAttrRecordingEquipment] = ((LFPictureAttributes*)i->Slave)->Equipment;
	i->AttributeValues[LFAttrRoll] = ((LFPictureAttributes*)i->Slave)->Roll;
	i->AttributeValues[LFAttrExposure] = &((LFPictureAttributes*)i->Slave)->Exposure;
	i->AttributeValues[LFAttrHeight] = &((LFPictureAttributes*)i->Slave)->Height;
	i->AttributeValues[LFAttrWidth] = &((LFPictureAttributes*)i->Slave)->Width;
	i->AttributeValues[LFAttrAperture] = &((LFPictureAttributes*)i->Slave)->Aperture;
	i->AttributeValues[LFAttrFocus] = &((LFPictureAttributes*)i->Slave)->Focus;
	i->AttributeValues[LFAttrChip] = ((LFPictureAttributes*)i->Slave)->Chip;
	i->AttributeValues[LFAttrRecordingTime] = &((LFPictureAttributes*)i->Slave)->RecordingTime;
	i->AttributeValues[LFAttrLanguage] = ((LFPictureAttributes*)i->Slave)->Language;
}


// CIdxTableVideos
//

CIdxTableVideos::CIdxTableVideos(char* Path, char* Filename)
	: CHeapfile(Path, Filename, sizeof(LFVideoAttributes))
{
}

CIdxTableVideos::~CIdxTableVideos()
{
}

void CIdxTableVideos::GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i)
{
	assert(i);
	assert(PtrDst);

	if ((i->CoreAttributes.SlaveID==IDSlaveVideos) && (i->Slave))
	{
		ZeroCopy(PtrDst, Hdr.ElementSize-LFKeySize, i->Slave, sizeof(LFVideoAttributes));
	}
	else
	{
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Artist), LFAttrArtist, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Copyright), LFAttrCopyright, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Title), LFAttrTitle, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Equipment), LFAttrRecordingEquipment, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Roll), LFAttrRoll, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, VideoCodec), LFAttrVideoCodec, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, VideoCodec), LFAttrVideoCodec, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Height), LFAttrHeight, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Width), LFAttrWidth, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Channels), LFAttrChannels, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Samplerate), LFAttrSamplerate, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Duration), LFAttrDuration, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Bitrate), LFAttrBitrate, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, RecordingTime), LFAttrRecordingTime, i);
		GetAttribute(PtrDst, offsetof(LFVideoAttributes, Language), LFAttrLanguage, i);
	}
}

void CIdxTableVideos::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(!i->Slave);
	assert(i->CoreAttributes.SlaveID==IDSlaveVideos);
	assert(PtrSrc);

	i->Slave = malloc(sizeof(LFVideoAttributes));
	ZeroCopy(i->Slave, sizeof(LFVideoAttributes), PtrSrc, Hdr.ElementSize-LFKeySize);

	i->AttributeValues[LFAttrArtist] = ((LFVideoAttributes*)i->Slave)->Artist;
	i->AttributeValues[LFAttrCopyright] = ((LFVideoAttributes*)i->Slave)->Copyright;
	i->AttributeValues[LFAttrTitle] = ((LFVideoAttributes*)i->Slave)->Title;
	i->AttributeValues[LFAttrRecordingEquipment] = ((LFVideoAttributes*)i->Slave)->Equipment;
	i->AttributeValues[LFAttrRoll] = ((LFVideoAttributes*)i->Slave)->Roll;
	i->AttributeValues[LFAttrHeight] = &((LFVideoAttributes*)i->Slave)->Height;
	i->AttributeValues[LFAttrWidth] = &((LFVideoAttributes*)i->Slave)->Width;
	i->AttributeValues[LFAttrVideoCodec] = &((LFVideoAttributes*)i->Slave)->VideoCodec;
	i->AttributeValues[LFAttrVideoCodec] = &((LFVideoAttributes*)i->Slave)->VideoCodec;
	i->AttributeValues[LFAttrChannels] = &((LFVideoAttributes*)i->Slave)->Channels;
	i->AttributeValues[LFAttrSamplerate] = &((LFVideoAttributes*)i->Slave)->Samplerate;
	i->AttributeValues[LFAttrDuration] = &((LFVideoAttributes*)i->Slave)->Duration;
	i->AttributeValues[LFAttrBitrate] = &((LFVideoAttributes*)i->Slave)->Bitrate;
	i->AttributeValues[LFAttrRecordingTime] = &((LFVideoAttributes*)i->Slave)->RecordingTime;
	i->AttributeValues[LFAttrLanguage] = ((LFVideoAttributes*)i->Slave)->Language;
}

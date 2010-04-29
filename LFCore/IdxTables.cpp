
#pragma once
#include "stdafx.h"
#include "IdxTables.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include <assert.h>
#include <malloc.h>
#include <stddef.h>


inline void GetAttribute(void* PtrDst, unsigned int attr, LFItemDescriptor* i)
{
	assert(PtrDst);
	assert(attr<LFAttributeCount);

	if (i->AttributeValues[attr])
	{
		size_t sz = GetAttributeSize(attr, i->AttributeValues[attr]);
		memcpy_s(PtrDst, sz, i->AttributeValues[attr], sz);
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

	memcpy_s(PtrDst, sizeof(LFCoreAttributes), &i->CoreAttributes, sizeof(LFCoreAttributes));
}

void CIdxTableMaster::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(PtrSrc);

	memcpy_s(&i->CoreAttributes, sizeof(LFCoreAttributes), PtrSrc, sizeof(LFCoreAttributes));
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
		memcpy_s(PtrDst, sizeof(LFDocumentAttributes), i->Slave, sizeof(LFDocumentAttributes));
	}
	else
	{
		ZeroMemory(PtrDst, sizeof(LFDocumentAttributes));

		GetAttribute(((LFDocumentAttributes*)PtrDst)->Author, LFAttrArtist, i);
		GetAttribute(((LFDocumentAttributes*)PtrDst)->Copyright, LFAttrCopyright, i);
		GetAttribute(((LFDocumentAttributes*)PtrDst)->Title, LFAttrTitle, i);
		GetAttribute(((LFDocumentAttributes*)PtrDst)->Responsible, LFAttrResponsible, i);
		GetAttribute(&((LFDocumentAttributes*)PtrDst)->DueTime, LFAttrDueTime, i);
		GetAttribute(&((LFDocumentAttributes*)PtrDst)->DoneTime, LFAttrDoneTime, i);
		GetAttribute(((LFDocumentAttributes*)PtrDst)->Signature, LFAttrSignature, i);
		GetAttribute(((LFDocumentAttributes*)PtrDst)->ISBN, LFAttrISBN, i);
		GetAttribute(&((LFDocumentAttributes*)PtrDst)->Pages, LFAttrPages, i);
		GetAttribute(((LFDocumentAttributes*)PtrDst)->Language, LFAttrLanguage, i);
	}
}

void CIdxTableDocuments::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(!i->Slave);
	assert(i->CoreAttributes.SlaveID==IDSlaveDocuments);
	assert(PtrSrc);

	i->Slave = malloc(sizeof(LFDocumentAttributes));
	memcpy_s(i->Slave, sizeof(LFDocumentAttributes), PtrSrc, sizeof(LFDocumentAttributes));

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
		memcpy_s(PtrDst, sizeof(LFMailAttributes), i->Slave, sizeof(LFMailAttributes));
	}
	else
	{
		ZeroMemory(PtrDst, sizeof(LFMailAttributes));

		GetAttribute(((LFMailAttributes*)PtrDst)->From, LFAttrFrom, i);
		GetAttribute(((LFMailAttributes*)PtrDst)->To, LFAttrTo, i);
		GetAttribute(((LFMailAttributes*)PtrDst)->Subject, LFAttrTitle, i);
		GetAttribute(((LFMailAttributes*)PtrDst)->Language, LFAttrLanguage, i);
		GetAttribute(((LFMailAttributes*)PtrDst)->Responsible, LFAttrResponsible, i);
		GetAttribute(&((LFMailAttributes*)PtrDst)->DueTime, LFAttrDueTime, i);
		GetAttribute(&((LFMailAttributes*)PtrDst)->DoneTime, LFAttrDoneTime, i);
	}
}

void CIdxTableMails::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(!i->Slave);
	assert(i->CoreAttributes.SlaveID==IDSlaveMails);
	assert(PtrSrc);

	i->Slave = malloc(sizeof(LFMailAttributes));
	memcpy_s(i->Slave, sizeof(LFMailAttributes), PtrSrc, sizeof(LFMailAttributes));

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
		memcpy_s(PtrDst, sizeof(LFAudioAttributes), i->Slave, sizeof(LFAudioAttributes));
	}
	else
	{
		ZeroMemory(PtrDst, sizeof(LFAudioAttributes));

		GetAttribute(((LFAudioAttributes*)PtrDst)->Artist, LFAttrArtist, i);
		GetAttribute(((LFAudioAttributes*)PtrDst)->Copyright, LFAttrCopyright, i);
		GetAttribute(((LFAudioAttributes*)PtrDst)->Title, LFAttrTitle, i);
		GetAttribute(((LFAudioAttributes*)PtrDst)->Album, LFAttrAlbum, i);
		GetAttribute(&((LFAudioAttributes*)PtrDst)->AudioCodec, LFAttrAudioCodec, i);
		GetAttribute(&((LFAudioAttributes*)PtrDst)->Channels, LFAttrChannels, i);
		GetAttribute(&((LFAudioAttributes*)PtrDst)->Samplerate, LFAttrSamplerate, i);
		GetAttribute(&((LFAudioAttributes*)PtrDst)->Duration, LFAttrDuration, i);
		GetAttribute(&((LFAudioAttributes*)PtrDst)->Bitrate, LFAttrBitrate, i);
		GetAttribute(&((LFAudioAttributes*)PtrDst)->RecordingTime, LFAttrRecordingTime, i);
		GetAttribute(((LFAudioAttributes*)PtrDst)->Language, LFAttrLanguage, i);
	}
}

void CIdxTableAudio::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(!i->Slave);
	assert(i->CoreAttributes.SlaveID==IDSlaveAudio);
	assert(PtrSrc);

	i->Slave = malloc(sizeof(LFAudioAttributes));
	memcpy_s(i->Slave, sizeof(LFAudioAttributes), PtrSrc, sizeof(LFAudioAttributes));

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
		memcpy_s(PtrDst, sizeof(LFPictureAttributes), i->Slave, sizeof(LFPictureAttributes));
	}
	else
	{
		ZeroMemory(PtrDst, sizeof(LFPictureAttributes));

		GetAttribute(((LFPictureAttributes*)PtrDst)->Artist, LFAttrArtist, i);
		GetAttribute(((LFPictureAttributes*)PtrDst)->Copyright, LFAttrCopyright, i);
		GetAttribute(((LFPictureAttributes*)PtrDst)->Title, LFAttrTitle, i);
		GetAttribute(((LFPictureAttributes*)PtrDst)->Equipment, LFAttrRecordingEquipment, i);
		GetAttribute(((LFPictureAttributes*)PtrDst)->Roll, LFAttrRoll, i);
		GetAttribute(((LFPictureAttributes*)PtrDst)->Exposure, LFAttrExposure, i);
		GetAttribute(&((LFPictureAttributes*)PtrDst)->Height, LFAttrHeight, i);
		GetAttribute(&((LFPictureAttributes*)PtrDst)->Width, LFAttrWidth, i);
		GetAttribute(&((LFPictureAttributes*)PtrDst)->Aperture, LFAttrAperture, i);
		GetAttribute(&((LFPictureAttributes*)PtrDst)->Focus, LFAttrFocus, i);
		GetAttribute(((LFPictureAttributes*)PtrDst)->Chip, LFAttrChip, i);
		GetAttribute(&((LFPictureAttributes*)PtrDst)->RecordingTime, LFAttrRecordingTime, i);
		GetAttribute(((LFPictureAttributes*)PtrDst)->Language, LFAttrLanguage, i);
	}
}

void CIdxTablePictures::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(!i->Slave);
	assert(i->CoreAttributes.SlaveID==IDSlavePictures);
	assert(PtrSrc);

	i->Slave = malloc(sizeof(LFPictureAttributes));
	memcpy_s(i->Slave, sizeof(LFPictureAttributes), PtrSrc, sizeof(LFPictureAttributes));

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
		memcpy_s(PtrDst, sizeof(LFVideoAttributes), i->Slave, sizeof(LFVideoAttributes));
	}
	else
	{
		ZeroMemory(PtrDst, sizeof(LFVideoAttributes));

		GetAttribute(((LFVideoAttributes*)PtrDst)->Artist, LFAttrArtist, i);
		GetAttribute(((LFVideoAttributes*)PtrDst)->Copyright, LFAttrCopyright, i);
		GetAttribute(((LFVideoAttributes*)PtrDst)->Title, LFAttrTitle, i);
		GetAttribute(((LFVideoAttributes*)PtrDst)->Equipment, LFAttrRecordingEquipment, i);
		GetAttribute(((LFVideoAttributes*)PtrDst)->Roll, LFAttrRoll, i);
		GetAttribute(&((LFVideoAttributes*)PtrDst)->VideoCodec, LFAttrVideoCodec, i);
		GetAttribute(&((LFVideoAttributes*)PtrDst)->VideoCodec, LFAttrVideoCodec, i);
		GetAttribute(&((LFVideoAttributes*)PtrDst)->Height, LFAttrHeight, i);
		GetAttribute(&((LFVideoAttributes*)PtrDst)->Width, LFAttrWidth, i);
		GetAttribute(&((LFVideoAttributes*)PtrDst)->Channels, LFAttrChannels, i);
		GetAttribute(&((LFVideoAttributes*)PtrDst)->Samplerate, LFAttrSamplerate, i);
		GetAttribute(&((LFVideoAttributes*)PtrDst)->Duration, LFAttrDuration, i);
		GetAttribute(&((LFVideoAttributes*)PtrDst)->Bitrate, LFAttrBitrate, i);
		GetAttribute(&((LFVideoAttributes*)PtrDst)->RecordingTime, LFAttrRecordingTime, i);
		GetAttribute(((LFVideoAttributes*)PtrDst)->Language, LFAttrLanguage, i);
	}
}

void CIdxTableVideos::WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc)
{
	assert(i);
	assert(!i->Slave);
	assert(i->CoreAttributes.SlaveID==IDSlaveVideos);
	assert(PtrSrc);

	i->Slave = malloc(sizeof(LFVideoAttributes));
	memcpy_s(i->Slave, sizeof(LFVideoAttributes), PtrSrc, sizeof(LFVideoAttributes));

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

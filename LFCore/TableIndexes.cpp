
#include "stdafx.h"
#include "TableIndexes.h"


#pragma data_seg(".shared")

extern const IdxTableEntry CoreAttributeEntries[] = {
	{ LFAttrFileName, offsetof(LFCoreAttributes, FileName) },
	{ LFAttrPriority, offsetof(LFCoreAttributes, Priority) },
	{ LFAttrCreationTime, offsetof(LFCoreAttributes, CreationTime) },
	{ LFAttrFileTime, offsetof(LFCoreAttributes, FileTime) },
	{ LFAttrAddTime, offsetof(LFCoreAttributes, AddTime) },
	{ LFAttrDueTime, offsetof(LFCoreAttributes, DueTime) },
	{ LFAttrDoneTime, offsetof(LFCoreAttributes, DoneTime) },
	{ LFAttrArchiveTime, offsetof(LFCoreAttributes, ArchiveTime) },
	{ LFAttrDeleteTime, offsetof(LFCoreAttributes, DeleteTime) },
	{ LFAttrFileFormat, offsetof(LFCoreAttributes, FileFormat) },
	{ LFAttrFileSize, offsetof(LFCoreAttributes, FileSize) },
	{ LFAttrColor, offsetof(LFCoreAttributes, Color) },
	{ LFAttrHashtags, offsetof(LFCoreAttributes, Hashtags) },
	{ LFAttrRating, offsetof(LFCoreAttributes, Rating) },
	{ LFAttrComments, offsetof(LFCoreAttributes, Comments) },
	{ LFAttrLocationName, offsetof(LFCoreAttributes, LocationName) },
	{ LFAttrLocationIATA, offsetof(LFCoreAttributes, LocationIATA) },
	{ LFAttrLocationGPS, offsetof(LFCoreAttributes, LocationGPS) },
	{ LFAttrURL, offsetof(LFCoreAttributes, URL) }
};

const IdxTableEntry DocumentAttributeEntries[] = {
	{ LFAttrCreator, offsetof(DocumentAttributes, Creator) },
	{ LFAttrCopyright, offsetof(DocumentAttributes, Copyright) },
	{ LFAttrTitle, offsetof(DocumentAttributes, Title) },
	{ LFAttrResponsible, offsetof(DocumentAttributes, Responsible) },
	{ LFAttrApplication, offsetof(DocumentAttributes, Application) },
	{ LFAttrSignature, offsetof(DocumentAttributes, Signature) },
	{ LFAttrISBN, offsetof(DocumentAttributes, ISBN) },
	{ LFAttrPages, offsetof(DocumentAttributes, Pages) },
	{ LFAttrLanguage, offsetof(DocumentAttributes, Language) },
	{ LFAttrCustomer, offsetof(DocumentAttributes, Customer) },
	{ LFAttrReleased, offsetof(DocumentAttributes, PublishedYear) },
	{ LFAttrMediaCollection, offsetof(DocumentAttributes, MediaCollection) }
};

const IdxTableEntry MessageAttributeEntries[] = {
	{ LFAttrFrom, offsetof(MessageAttributes, From) },
	{ LFAttrTo, offsetof(MessageAttributes, To) },
	{ LFAttrTitle, offsetof(MessageAttributes, Title) },
	{ LFAttrLanguage, offsetof(MessageAttributes, Language) },
	{ LFAttrResponsible, offsetof(MessageAttributes, Responsible) }
};

const IdxTableEntry AudioAttributeEntries[] = {
	{ LFAttrCreator, offsetof(AudioAttributes, Creator) },
	{ LFAttrCopyright, offsetof(AudioAttributes, Copyright) },
	{ LFAttrTitle, offsetof(AudioAttributes, Title) },
	{ LFAttrMediaCollection, offsetof(AudioAttributes, MediaCollection) },
	{ LFAttrAudioCodec, offsetof(AudioAttributes, AudioCodec) },
	{ LFAttrChannels, offsetof(AudioAttributes, Channels) },
	{ LFAttrSamplerate, offsetof(AudioAttributes, Samplerate) },
	{ LFAttrLength, offsetof(AudioAttributes, Duration) },
	{ LFAttrBitrate, offsetof(AudioAttributes, Bitrate) },
	{ LFAttrRecordingTime, offsetof(AudioAttributes, RecordingTime) },
	{ LFAttrLanguage, offsetof(AudioAttributes, Language) },
	{ LFAttrGenre, offsetof(AudioAttributes, Genre) },
	{ LFAttrSequenceInCollection, offsetof(AudioAttributes, SequenceInCollection) },
	{ LFAttrReleased, offsetof(AudioAttributes, PublishedYear) }
};

const IdxTableEntry PictureAttributeEntries[] = {
	{ LFAttrCreator, offsetof(PictureAttributes, Creator) },
	{ LFAttrCopyright, offsetof(PictureAttributes, Copyright) },
	{ LFAttrTitle, offsetof(PictureAttributes, Title) },
	{ LFAttrRecordingEquipment, offsetof(PictureAttributes, RecordingEquipment) },
	{ LFAttrMediaCollection, offsetof(PictureAttributes, MediaCollection) },
	{ LFAttrExposure, offsetof(PictureAttributes, Exposure) },
	{ LFAttrWidth, offsetof(PictureAttributes, Width) },
	{ LFAttrHeight, offsetof(PictureAttributes, Height) },
	{ LFAttrAperture, offsetof(PictureAttributes, Aperture) },
	{ LFAttrFocus, offsetof(PictureAttributes, Focus) },
	{ LFAttrChip, offsetof(PictureAttributes, Chip) },
	{ LFAttrRecordingTime, offsetof(PictureAttributes, RecordingTime) },
	{ LFAttrLanguage, offsetof(PictureAttributes, Language) },
	{ LFAttrApplication, offsetof(PictureAttributes, Application) },
	{ LFAttrCustomer, offsetof(PictureAttributes, Customer) }
};

const IdxTableEntry VideoAttributeEntries[] = {
	{ LFAttrCreator, offsetof(VideoAttributes, Creator) },
	{ LFAttrCopyright, offsetof(VideoAttributes, Copyright) },
	{ LFAttrTitle, offsetof(VideoAttributes, Title) },
	{ LFAttrRecordingEquipment, offsetof(VideoAttributes, RecordingEquipment) },
	{ LFAttrMediaCollection, offsetof(VideoAttributes, MediaCollection) },
	{ LFAttrHeight, offsetof(VideoAttributes, Height) },
	{ LFAttrWidth, offsetof(VideoAttributes, Width) },
	{ LFAttrAudioCodec, offsetof(VideoAttributes, AudioCodec) },
	{ LFAttrVideoCodec, offsetof(VideoAttributes, VideoCodec) },
	{ LFAttrChannels, offsetof(VideoAttributes, Channels) },
	{ LFAttrSamplerate, offsetof(VideoAttributes, Samplerate) },
	{ LFAttrLength, offsetof(VideoAttributes, Duration) },
	{ LFAttrBitrate, offsetof(VideoAttributes, Bitrate) },
	{ LFAttrRecordingTime, offsetof(VideoAttributes, RecordingTime) },
	{ LFAttrLanguage, offsetof(VideoAttributes, Language) },
	{ LFAttrApplication, offsetof(VideoAttributes, Application) },
	{ LFAttrCustomer, offsetof(VideoAttributes, Customer) },
	{ LFAttrSequenceInCollection, offsetof(VideoAttributes, SequenceInCollection) },
	{ LFAttrReleased, offsetof(VideoAttributes, PublishedYear) },
	{ LFAttrFramerate, offsetof(VideoAttributes, Framerate) }
};

extern const IdxTable IndexTables[IDXTABLECOUNT] = {
	{ L"Master.idx", sizeof(LFCoreAttributes), sizeof(CoreAttributeEntries)/sizeof(IdxTableEntry), CoreAttributeEntries },
	{ L"Docs.idx", sizeof(DocumentAttributes), sizeof(DocumentAttributeEntries)/sizeof(IdxTableEntry), DocumentAttributeEntries },
	{ L"Mails.idx", sizeof(MessageAttributes), sizeof(MessageAttributeEntries)/sizeof(IdxTableEntry), MessageAttributeEntries },
	{ L"Audio.idx", sizeof(AudioAttributes), sizeof(AudioAttributeEntries)/sizeof(IdxTableEntry), AudioAttributeEntries },
	{ L"Pictures.idx", sizeof(PictureAttributes), sizeof(PictureAttributeEntries)/sizeof(IdxTableEntry), PictureAttributeEntries },
	{ L"Video.idx", sizeof(VideoAttributes), sizeof(VideoAttributeEntries)/sizeof(IdxTableEntry), VideoAttributeEntries }
};

#pragma data_seg()

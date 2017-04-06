
#include "stdafx.h"
#include "IndexTables.h"


#pragma data_seg(".shared")

extern const IdxTableEntry CoreAttributeEntries[] = {
	{ LFAttrFileName, offsetof(LFCoreAttributes, FileName) },
	{ LFAttrFileID, offsetof(LFCoreAttributes, FileID) },
	{ LFAttrComments, offsetof(LFCoreAttributes, Comments) },
	{ LFAttrCreationTime, offsetof(LFCoreAttributes, CreationTime) },
	{ LFAttrFileTime, offsetof(LFCoreAttributes, FileTime) },
	{ LFAttrAddTime, offsetof(LFCoreAttributes, AddTime) },
	{ LFAttrDeleteTime, offsetof(LFCoreAttributes, DeleteTime) },
	{ LFAttrArchiveTime, offsetof(LFCoreAttributes, ArchiveTime) },
	{ LFAttrFileFormat, offsetof(LFCoreAttributes, FileFormat) },
	{ LFAttrFileSize, offsetof(LFCoreAttributes, FileSize) },
	{ LFAttrFlags, offsetof(LFCoreAttributes, Flags) },
	{ LFAttrURL, offsetof(LFCoreAttributes, URL) },
	{ LFAttrHashtags, offsetof(LFCoreAttributes, Hashtags) },
	{ LFAttrRating, offsetof(LFCoreAttributes, Rating) },
	{ LFAttrPriority, offsetof(LFCoreAttributes, Priority) },
	{ LFAttrLocationName, offsetof(LFCoreAttributes, LocationName) },
	{ LFAttrLocationIATA, offsetof(LFCoreAttributes, LocationIATA) },
	{ LFAttrLocationGPS, offsetof(LFCoreAttributes, LocationGPS) }
};

const IdxTableEntry DocumentAttributeEntries[] = {
	{ LFAttrArtist, offsetof(DocumentAttributes, Artist) },
	{ LFAttrCopyright, offsetof(DocumentAttributes, Copyright) },
	{ LFAttrTitle, offsetof(DocumentAttributes, Title) },
	{ LFAttrResponsible, offsetof(DocumentAttributes, Responsible) },
	{ LFAttrDueTime, offsetof(DocumentAttributes, DueTime) },
	{ LFAttrDoneTime, offsetof(DocumentAttributes, DoneTime) },
	{ LFAttrSignature, offsetof(DocumentAttributes, Signature) },
	{ LFAttrISBN, offsetof(DocumentAttributes, ISBN) },
	{ LFAttrPages, offsetof(DocumentAttributes, Pages) },
	{ LFAttrLanguage, offsetof(DocumentAttributes, Language) },
	{ LFAttrCustomer, offsetof(DocumentAttributes, Customer) }
};

const IdxTableEntry MessageAttributeEntries[] = {
	{ LFAttrFrom, offsetof(MessageAttributes, From) },
	{ LFAttrTo, offsetof(MessageAttributes, To) },
	{ LFAttrTitle, offsetof(MessageAttributes, Title) },
	{ LFAttrLanguage, offsetof(MessageAttributes, Language) },
	{ LFAttrResponsible, offsetof(MessageAttributes, Responsible) },
	{ LFAttrDueTime, offsetof(MessageAttributes, DueTime) },
	{ LFAttrDoneTime, offsetof(MessageAttributes, DoneTime) }
};

const IdxTableEntry AudioAttributeEntries[] = {
	{ LFAttrArtist, offsetof(AudioAttributes, Artist) },
	{ LFAttrCopyright, offsetof(AudioAttributes, Copyright) },
	{ LFAttrTitle, offsetof(AudioAttributes, Title) },
	{ LFAttrAlbum, offsetof(AudioAttributes, Album) },
	{ LFAttrAudioCodec, offsetof(AudioAttributes, AudioCodec) },
	{ LFAttrChannels, offsetof(AudioAttributes, Channels) },
	{ LFAttrSamplerate, offsetof(AudioAttributes, Samplerate) },
	{ LFAttrDuration, offsetof(AudioAttributes, Duration) },
	{ LFAttrBitrate, offsetof(AudioAttributes, Bitrate) },
	{ LFAttrRecordingTime, offsetof(AudioAttributes, RecordingTime) },
	{ LFAttrLanguage, offsetof(AudioAttributes, Language) },
	{ LFAttrGenre, offsetof(AudioAttributes, Genre) }
};

const IdxTableEntry PictureAttributeEntries[] = {
	{ LFAttrArtist, offsetof(PictureAttributes, Artist) },
	{ LFAttrCopyright, offsetof(PictureAttributes, Copyright) },
	{ LFAttrTitle, offsetof(PictureAttributes, Title) },
	{ LFAttrEquipment, offsetof(PictureAttributes, Equipment) },
	{ LFAttrRoll, offsetof(PictureAttributes, Roll) },
	{ LFAttrExposure, offsetof(PictureAttributes, Exposure) },
	{ LFAttrHeight, offsetof(PictureAttributes, Height) },
	{ LFAttrWidth, offsetof(PictureAttributes, Width) },
	{ LFAttrAperture, offsetof(PictureAttributes, Aperture) },
	{ LFAttrFocus, offsetof(PictureAttributes, Focus) },
	{ LFAttrChip, offsetof(PictureAttributes, Chip) },
	{ LFAttrRecordingTime, offsetof(PictureAttributes, RecordingTime) },
	{ LFAttrLanguage, offsetof(PictureAttributes, Language) },
	{ LFAttrCustomer, offsetof(PictureAttributes, Customer) }
};

const IdxTableEntry VideoAttributeEntries[] = {
	{ LFAttrArtist, offsetof(VideoAttributes, Artist) },
	{ LFAttrCopyright, offsetof(VideoAttributes, Copyright) },
	{ LFAttrTitle, offsetof(VideoAttributes, Title) },
	{ LFAttrEquipment, offsetof(VideoAttributes, Equipment) },
	{ LFAttrRoll, offsetof(VideoAttributes, Roll) },
	{ LFAttrHeight, offsetof(VideoAttributes, Height) },
	{ LFAttrWidth, offsetof(VideoAttributes, Width) },
	{ LFAttrVideoCodec, offsetof(VideoAttributes, VideoCodec) },
	{ LFAttrVideoCodec, offsetof(VideoAttributes, VideoCodec) },
	{ LFAttrChannels, offsetof(VideoAttributes, Channels) },
	{ LFAttrSamplerate, offsetof(VideoAttributes, Samplerate) },
	{ LFAttrDuration, offsetof(VideoAttributes, Duration) },
	{ LFAttrBitrate, offsetof(VideoAttributes, Bitrate) },
	{ LFAttrRecordingTime, offsetof(VideoAttributes, RecordingTime) },
	{ LFAttrLanguage, offsetof(VideoAttributes, Language) },
	{ LFAttrCustomer, offsetof(VideoAttributes, Customer) }
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

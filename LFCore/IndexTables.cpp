
#include "stdafx.h"
#include "IndexTables.h"


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
	{ LFAttrAuthor, offsetof(DocumentAttributes, Author) },
	{ LFAttrCopyright, offsetof(DocumentAttributes, Copyright) },
	{ LFAttrTitle, offsetof(DocumentAttributes, Title) },
	{ LFAttrISBN, offsetof(DocumentAttributes, ISBN) },
	{ LFAttrLanguage, offsetof(DocumentAttributes, Language) },
	{ LFAttrPages, offsetof(DocumentAttributes, Pages) },
	{ LFAttrSignature, offsetof(DocumentAttributes, Signature) },
	{ LFAttrResponsible, offsetof(DocumentAttributes, Responsible) },
	{ LFAttrCustomer, offsetof(DocumentAttributes, Customer) }
};

const IdxTableEntry MessageAttributeEntries[] = {
	{ LFAttrFrom, offsetof(MessageAttributes, From) },
	{ LFAttrTo, offsetof(MessageAttributes, To) },
	{ LFAttrTitle, offsetof(MessageAttributes, Title) },
	{ LFAttrLanguage, offsetof(MessageAttributes, Language) },
	{ LFAttrResponsible, offsetof(MessageAttributes, Responsible) }
};

const IdxTableEntry AudioAttributeEntries[] = {
	{ LFAttrArtist, offsetof(AudioAttributes, Artist) },
	{ LFAttrAlbum, offsetof(AudioAttributes, Album) },
	{ LFAttrGenre, offsetof(AudioAttributes, Genre) },
	{ LFAttrChannels, offsetof(AudioAttributes, Channels) },
	{ LFAttrSamplerate, offsetof(AudioAttributes, Samplerate) },
	{ LFAttrAudioCodec, offsetof(AudioAttributes, AudioCodec) },
	{ LFAttrCopyright, offsetof(AudioAttributes, Copyright) },
	{ LFAttrDuration, offsetof(AudioAttributes, Duration) },
	{ LFAttrBitrate, offsetof(AudioAttributes, Bitrate) },
	{ LFAttrRecordingTime, offsetof(AudioAttributes, RecordingTime) },
	{ LFAttrTitle, offsetof(AudioAttributes, Title) },
	{ LFAttrLanguage, offsetof(AudioAttributes, Language) }
};

const IdxTableEntry PictureAttributeEntries[] = {
	{ LFAttrRoll, offsetof(PictureAttributes, Roll) },
	{ LFAttrWidth, offsetof(PictureAttributes, Width) },
	{ LFAttrHeight, offsetof(PictureAttributes, Height) },
	{ LFAttrExposure, offsetof(PictureAttributes, Exposure) },
	{ LFAttrAperture, offsetof(PictureAttributes, Aperture) },
	{ LFAttrFocus, offsetof(PictureAttributes, Focus) },
	{ LFAttrChip, offsetof(PictureAttributes, Chip) },
	{ LFAttrRecordingTime, offsetof(PictureAttributes, RecordingTime) },
	{ LFAttrRecordingEquipment, offsetof(PictureAttributes, Equipment) },
	{ LFAttrAuthor, offsetof(PictureAttributes, Author) },
	{ LFAttrCopyright, offsetof(PictureAttributes, Copyright) },
	{ LFAttrTitle, offsetof(PictureAttributes, Title) },
	{ LFAttrLanguage, offsetof(PictureAttributes, Language) },
	{ LFAttrCustomer, offsetof(PictureAttributes, Customer) }
};

const IdxTableEntry VideoAttributeEntries[] = {
	{ LFAttrRoll, offsetof(PictureAttributes, Roll) },
	{ LFAttrWidth, offsetof(PictureAttributes, Width) },
	{ LFAttrHeight, offsetof(PictureAttributes, Height) },
	{ LFAttrVideoCodec, offsetof(VideoAttributes, VideoCodec) },
	{ LFAttrArtist, offsetof(VideoAttributes, Artist) },
	{ LFAttrChannels, offsetof(VideoAttributes, Channels) },
	{ LFAttrSamplerate, offsetof(VideoAttributes, Samplerate) },
	{ LFAttrAudioCodec, offsetof(VideoAttributes, AudioCodec) },
	{ LFAttrDuration, offsetof(VideoAttributes, Duration) },
	{ LFAttrBitrate, offsetof(VideoAttributes, Bitrate) },
	{ LFAttrRecordingTime, offsetof(VideoAttributes, RecordingTime) },
	{ LFAttrRecordingEquipment, offsetof(VideoAttributes, Equipment) },
	{ LFAttrCopyright, offsetof(VideoAttributes, Copyright) },
	{ LFAttrTitle, offsetof(VideoAttributes, Title) },
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


#include "stdafx.h"
#include "IndexTables.h"


#pragma data_seg(".shared")

extern const LFIdxTableEntry LFCoreAttributeEntries[] = {
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

const LFIdxTableEntry LFDocumentAttributeEntries[] = {
	{ LFAttrArtist, offsetof(LFDocumentAttributes, Artist) },
	{ LFAttrCopyright, offsetof(LFDocumentAttributes, Copyright) },
	{ LFAttrTitle, offsetof(LFDocumentAttributes, Title) },
	{ LFAttrResponsible, offsetof(LFDocumentAttributes, Responsible) },
	{ LFAttrDueTime, offsetof(LFDocumentAttributes, DueTime) },
	{ LFAttrDoneTime, offsetof(LFDocumentAttributes, DoneTime) },
	{ LFAttrSignature, offsetof(LFDocumentAttributes, Signature) },
	{ LFAttrISBN, offsetof(LFDocumentAttributes, ISBN) },
	{ LFAttrPages, offsetof(LFDocumentAttributes, Pages) },
	{ LFAttrLanguage, offsetof(LFDocumentAttributes, Language) },
	{ LFAttrCustomer, offsetof(LFDocumentAttributes, Customer) }
};

const LFIdxTableEntry LFMessageAttributeEntries[] = {
	{ LFAttrFrom, offsetof(LFMessageAttributes, From) },
	{ LFAttrTo, offsetof(LFMessageAttributes, To) },
	{ LFAttrTitle, offsetof(LFMessageAttributes, Title) },
	{ LFAttrLanguage, offsetof(LFMessageAttributes, Language) },
	{ LFAttrResponsible, offsetof(LFMessageAttributes, Responsible) },
	{ LFAttrDueTime, offsetof(LFMessageAttributes, DueTime) },
	{ LFAttrDoneTime, offsetof(LFMessageAttributes, DoneTime) }
};

const LFIdxTableEntry LFAudioAttributeEntries[] = {
	{ LFAttrArtist, offsetof(LFAudioAttributes, Artist) },
	{ LFAttrCopyright, offsetof(LFAudioAttributes, Copyright) },
	{ LFAttrTitle, offsetof(LFAudioAttributes, Title) },
	{ LFAttrAlbum, offsetof(LFAudioAttributes, Album) },
	{ LFAttrAudioCodec, offsetof(LFAudioAttributes, AudioCodec) },
	{ LFAttrChannels, offsetof(LFAudioAttributes, Channels) },
	{ LFAttrSamplerate, offsetof(LFAudioAttributes, Samplerate) },
	{ LFAttrDuration, offsetof(LFAudioAttributes, Duration) },
	{ LFAttrBitrate, offsetof(LFAudioAttributes, Bitrate) },
	{ LFAttrRecordingTime, offsetof(LFAudioAttributes, RecordingTime) },
	{ LFAttrLanguage, offsetof(LFAudioAttributes, Language) },
	{ LFAttrGenre, offsetof(LFAudioAttributes, Genre) }
};

const LFIdxTableEntry LFPictureAttributeEntries[] = {
	{ LFAttrArtist, offsetof(LFPictureAttributes, Artist) },
	{ LFAttrCopyright, offsetof(LFPictureAttributes, Copyright) },
	{ LFAttrTitle, offsetof(LFPictureAttributes, Title) },
	{ LFAttrEquipment, offsetof(LFPictureAttributes, Equipment) },
	{ LFAttrRoll, offsetof(LFPictureAttributes, Roll) },
	{ LFAttrExposure, offsetof(LFPictureAttributes, Exposure) },
	{ LFAttrHeight, offsetof(LFPictureAttributes, Height) },
	{ LFAttrWidth, offsetof(LFPictureAttributes, Width) },
	{ LFAttrAperture, offsetof(LFPictureAttributes, Aperture) },
	{ LFAttrFocus, offsetof(LFPictureAttributes, Focus) },
	{ LFAttrChip, offsetof(LFPictureAttributes, Chip) },
	{ LFAttrRecordingTime, offsetof(LFPictureAttributes, RecordingTime) },
	{ LFAttrLanguage, offsetof(LFPictureAttributes, Language) },
	{ LFAttrCustomer, offsetof(LFPictureAttributes, Customer) }
};

const LFIdxTableEntry LFVideoAttributeEntries[] = {
	{ LFAttrArtist, offsetof(LFVideoAttributes, Artist) },
	{ LFAttrCopyright, offsetof(LFVideoAttributes, Copyright) },
	{ LFAttrTitle, offsetof(LFVideoAttributes, Title) },
	{ LFAttrEquipment, offsetof(LFVideoAttributes, Equipment) },
	{ LFAttrRoll, offsetof(LFVideoAttributes, Roll) },
	{ LFAttrHeight, offsetof(LFVideoAttributes, Height) },
	{ LFAttrWidth, offsetof(LFVideoAttributes, Width) },
	{ LFAttrVideoCodec, offsetof(LFVideoAttributes, VideoCodec) },
	{ LFAttrVideoCodec, offsetof(LFVideoAttributes, VideoCodec) },
	{ LFAttrChannels, offsetof(LFVideoAttributes, Channels) },
	{ LFAttrSamplerate, offsetof(LFVideoAttributes, Samplerate) },
	{ LFAttrDuration, offsetof(LFVideoAttributes, Duration) },
	{ LFAttrBitrate, offsetof(LFVideoAttributes, Bitrate) },
	{ LFAttrRecordingTime, offsetof(LFVideoAttributes, RecordingTime) },
	{ LFAttrLanguage, offsetof(LFVideoAttributes, Language) },
	{ LFAttrCustomer, offsetof(LFVideoAttributes, Customer) }
};

extern const LFIdxTable LFIndexTables[IDXTABLECOUNT] = {
	{ L"Master.idx", sizeof(LFCoreAttributes), sizeof(LFCoreAttributeEntries)/sizeof(LFIdxTableEntry), LFCoreAttributeEntries },
	{ L"Docs.idx", sizeof(LFDocumentAttributes), sizeof(LFDocumentAttributeEntries)/sizeof(LFIdxTableEntry), LFDocumentAttributeEntries },
	{ L"Mails.idx", sizeof(LFMessageAttributes), sizeof(LFMessageAttributeEntries)/sizeof(LFIdxTableEntry), LFMessageAttributeEntries },
	{ L"Audio.idx", sizeof(LFAudioAttributes), sizeof(LFAudioAttributeEntries)/sizeof(LFIdxTableEntry), LFAudioAttributeEntries },
	{ L"Pictures.idx", sizeof(LFPictureAttributes), sizeof(LFPictureAttributeEntries)/sizeof(LFIdxTableEntry), LFPictureAttributeEntries },
	{ L"Video.idx", sizeof(LFVideoAttributes), sizeof(LFVideoAttributeEntries)/sizeof(LFIdxTableEntry), LFVideoAttributeEntries }
};

#pragma data_seg()

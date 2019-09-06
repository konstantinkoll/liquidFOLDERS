
#include "stdafx.h"
#include "TableContexts.h"
#include "TableIndexes.h"


#pragma data_seg(".shared")

extern const RegisteredFileFormat ContextRegistry[FILEFORMATCOUNT] = {
	{ "3dl",     LFContextColorTables, 0 },
	{ "8bsc",    LFContextColorTables, 0 },
	{ "8svx",    LFContextAudio, 0 },
	{ "aa",      LFContextAudio, 0 },
	{ "aac",     LFContextAudio, 0 },
	{ "aax",     LFContextAudio, 0 },
	{ "aco",     LFContextColorTables, 0 },
	{ "act",     LFContextAudio, 0 },
	{ "acv",     LFContextColorTables, 0 },
	{ "aiff",    LFContextAudio, 0 },
	{ "air",     LFContextDocuments, 0 },
	{ "airx",    LFContextDocuments, 0 },
	{ "amr",     LFContextAudio, 0 },
	{ "ans",     LFContextPictures, 0 },
	{ "apk",     LFContextApps, 0 },
	{ "app",     LFContextApps, 0 },
	{ "appx",    LFContextApps, 0 },
	{ "arw",     LFContextPictures, 0 },						// RAW format (digital camera)
	{ "asc",     LFContextDocuments, 0 },
	{ "ase",     LFContextColorTables, 0 },
	{ "asm",     LFContextDocuments, 0 },
	{ "atf",     LFContextColorTables, 0 },
	{ "au",      LFContextAudio, 0 },
	{ "audio",   LFContextAudio, 0 },
	{ "avi",     LFContextVideos, 0 },
	{ "avif",    LFContextPictures, 0 },
	{ "awb",     LFContextAudio, 0 },
	{ "azw",     LFContextDocuments, LFContextBooks },
	{ "bm",      LFContextPictures, 0 },
	{ "bmp",     LFContextPictures, 0 },
	{ "c",       LFContextDocuments, 0 },
	{ "caf",     LFContextAudio, 0 },
	{ "cb7",     LFContextDocuments, LFContextBooks },
	{ "cba",     LFContextDocuments, LFContextBooks },
	{ "cbf",     LFContextColorTables, 0 },
	{ "cbr",     LFContextDocuments, LFContextBooks },
	{ "cbt",     LFContextDocuments, LFContextBooks },
	{ "cbz",     LFContextDocuments, LFContextBooks },
	{ "cda",     LFContextAudio, 0 },
	{ "chm",     LFContextDocuments, 0 },
	{ "cpp",     LFContextDocuments, 0 },
	{ "cr2",     LFContextPictures, 0 },						// RAW format (digital camera)
	{ "crv",     LFContextColorTables, 0 },
	{ "crw",     LFContextPictures, 0 },						// RAW format (digital camera)
	{ "cs",      LFContextDocuments, 0 },
	{ "css",     LFContextDocuments, 0 },
	{ "csv",     LFContextDocuments, 0 },
	{ "cube",    LFContextColorTables, 0 },
	{ "cur",     LFContextPictures, 0 },
	{ "dct",     LFContextAudio, 0 },
	{ "dib",     LFContextPictures, 0 },
	{ "div",     LFContextVideos, 0 },
	{ "divx",    LFContextVideos, 0 },
	{ "djvu",    LFContextDocuments, LFContextBooks },
	{ "dll",     LFContextApps, 0 },
	{ "dng",     LFContextPictures, 0 },						// RAW format (digital camera)
	{ "doc",     LFContextDocuments, 0 },
	{ "docm",    LFContextDocuments, 0 },
	{ "docx",    LFContextDocuments, 0 },
	{ "dot",     LFContextDocuments, 0 },
	{ "dotm",    LFContextDocuments, 0 },
	{ "dotx",    LFContextDocuments, 0 },
	{ "dsf",     LFContextPictures, 0 },
	{ "dss",     LFContextAudio, 0 },
	{ "dv",      LFContextVideos, 0 },
	{ "dvf",     LFContextAudio, 0 },
	{ "dvi",     LFContextVideos, 0 },
	{ "elfo",    LFContextDocuments, 0 },
	{ "eml",     LFContextMessages, 0 },
	{ "eot",     LFContextFonts, 0 },
	{ "epa",     LFContextPictures, 0 },
	{ "epr",     LFContextApps, 0 },
	{ "epub",    LFContextDocuments, LFContextBooks },
	{ "exe",     LFContextApps, 0 },
	{ "fb2",     LFContextDocuments, LFContextBooks },
	{ "filter",  LFContextFilters, 0 },
	{ "flac",    LFContextAudio, 0 },
	{ "flc",     LFContextVideos, 0 },
	{ "fli",     LFContextVideos, 0 },
	{ "flic",    LFContextVideos, 0 },
	{ "flv",     LFContextVideos, 0 },
	{ "fnt",     LFContextFonts, 0 },
	{ "fon",     LFContextFonts, 0 },
	{ "gfa",     LFContextPictures, 0 },
	{ "gft",     LFContextFonts, 0 },
	{ "gif",     LFContextPictures, 0 },
	{ "gsm",     LFContextAudio, 0 },
	{ "h",       LFContextDocuments, 0 },
	{ "h264",    LFContextVideos, 0 },
	{ "h265",    LFContextVideos, 0 },
	{ "heic",    LFContextPictures, 0 },
	{ "heif",    LFContextPictures, 0 },
	{ "hh",      LFContextDocuments, 0 },
	{ "hlp",     LFContextDocuments, 0 },
	{ "htc",     LFContextDocuments, 0 },
	{ "htm",     LFContextDocuments, 0 },
	{ "html",    LFContextDocuments, 0 },
	{ "htx",     LFContextDocuments, 0 },
	{ "iba",     LFContextDocuments, LFContextBooks },
	{ "ibook",   LFContextDocuments, LFContextBooks },
	{ "ibooks",  LFContextDocuments, LFContextBooks },
	{ "icc",     LFContextColorTables, 0 },
	{ "icl",     LFContextPictures, 0 },
	{ "ico",     LFContextPictures, 0 },
	{ "iff",     LFContextAudio, 0 },
	{ "iklax",   LFContextAudio, LFContextMusic },
	{ "inc",     LFContextDocuments, 0 },
	{ "ipa",     LFContextApps, 0 },
	{ "ipsw",    LFContextApps, 0 },
	{ "ivs",     LFContextAudio, LFContextMusic },
	{ "jar",     LFContextApps, 0 },
	{ "jav",     LFContextDocuments, 0 },
	{ "java",    LFContextDocuments, 0 },
	{ "jfi",     LFContextPictures, 0 },
	{ "jfif",    LFContextPictures, 0 },
	{ "jpe",     LFContextPictures, 0 },
	{ "jpeg",    LFContextPictures, 0 },
	{ "jpg",     LFContextPictures, 0 },
	{ "js",      LFContextDocuments, 0 },
	{ "key",     LFContextDocuments, 0 },
	{ "key-tef", LFContextDocuments, 0 },
	{ "kml",     LFContextDocuments, 0 },
	{ "kmz",     LFContextDocuments, 0 },
	{ "lbm",     LFContextPictures, 0 },
	{ "lib",     LFContextApps, 0 },
	{ "lit",     LFContextDocuments, LFContextBooks },
	{ "log",     LFContextDocuments, 0 },
	{ "look",    LFContextColorTables, 0 },
	{ "m1v",     LFContextVideos, 0 },
	{ "m2a",     LFContextAudio, 0 },
	{ "m2v",     LFContextVideos, 0 },
	{ "m4a",     LFContextAudio, LFContextPodcasts },
	{ "m4b",     LFContextAudio, LFContextPodcasts },
	{ "m4p",     LFContextAudio, LFContextMusic },
	{ "m4v",     LFContextVideos, 0 },
	{ "maff",    LFContextDocuments, 0 },
	{ "mht",     LFContextDocuments, 0 },
	{ "mid",     LFContextAudio, LFContextMusic },
	{ "midi",    LFContextAudio, LFContextMusic },
	{ "mjp",     LFContextVideos, 0 },
	{ "mjpeg",   LFContextVideos, 0 },
	{ "mjpg",    LFContextVideos, 0 },
	{ "mmap",    LFContextDocuments, 0 },
	{ "mmf",     LFContextAudio, 0 },
	{ "mobi",    LFContextDocuments, LFContextBooks },
	{ "mod",     LFContextAudio, LFContextMusic },
	{ "mogg",    LFContextAudio, LFContextMusic },
	{ "mov",     LFContextVideos, 0 },
	{ "mp1",     LFContextAudio, 0 },
	{ "mp2",     LFContextAudio, 0 },
	{ "mp3",     LFContextAudio, LFContextMusic },
	{ "mp4",     LFContextVideos, 0 },
	{ "mpa",     LFContextAudio, 0 },
	{ "mpc",     LFContextAudio, 0 },
	{ "mpe",     LFContextVideos, 0 },
	{ "mpeg",    LFContextVideos, 0 },
	{ "mpg",     LFContextVideos, 0 },
	{ "mpv",     LFContextVideos, 0 },
	{ "mrw",     LFContextPictures, 0 },						// RAW format (digital camera)
	{ "msv",     LFContextAudio, 0 },
	{ "mxf",     LFContextVideos, LFContextTVShows },			// RAW format (TV production, only I-frames at high bitrate)
	{ "nef",     LFContextPictures, 0 },						// RAW format (digital camera)
	{ "nsm",     LFContextDocuments, 0 },
	{ "numbers", LFContextDocuments, 0 },
	{ "odg",     LFContextPictures, 0 },
	{ "odp",     LFContextDocuments, 0 },
	{ "ods",     LFContextDocuments, 0 },
	{ "odt",     LFContextDocuments, 0 },
	{ "odttf",   LFContextFonts, 0 },
	{ "oga",     LFContextAudio, 0 },
	{ "ogg",     LFContextAudio, 0 },
	{ "one",     LFContextDocuments, 0 },
	{ "opus",    LFContextAudio, 0 },
	{ "otf",     LFContextFonts, 0 },
	{ "oxps",    LFContextDocuments, 0 },
	{ "p",       LFContextDocuments, 0 },
	{ "pages",   LFContextDocuments, 0 },
	{ "pal",     LFContextColorTables, 0 },
	{ "pas",     LFContextDocuments, 0 },
	{ "pcd",     LFContextPictures, 0 },
	{ "pcx",     LFContextPictures, 0 },
	{ "pdb",     LFContextDocuments, LFContextBooks },
	{ "pdf",     LFContextDocuments, 0 },
	{ "pef",     LFContextPictures, 0 },						// RAW format (digital camera)
	{ "php",     LFContextDocuments, 0 },
	{ "pht",     LFContextDocuments, 0 },
	{ "phtm",    LFContextDocuments, 0 },
	{ "phtml",   LFContextDocuments, 0 },
	{ "pkg",     LFContextApps, 0 },
	{ "pkpass" , LFContextDocuments, 0 },
	{ "png",     LFContextPictures, 0 },
	{ "pot",     LFContextDocuments, 0 },
	{ "potm",    LFContextDocuments, 0 },
	{ "potx",    LFContextDocuments, 0 },
	{ "pps",     LFContextDocuments, 0 },
	{ "ppsm",    LFContextDocuments, 0 },
	{ "ppsx",    LFContextDocuments, 0 },
	{ "ppt",     LFContextDocuments, 0 },
	{ "pptm",    LFContextDocuments, 0 },
	{ "pptx",    LFContextDocuments, 0 },
	{ "prc",     LFContextDocuments, LFContextBooks },
	{ "prn",     LFContextDocuments, 0 },
	{ "ps",      LFContextDocuments, 0 },
	{ "psd",     LFContextPictures, 0 },
	{ "qt",      LFContextVideos, 0 },
	{ "qtm",     LFContextVideos, 0 },
	{ "ra",      LFContextAudio, 0 },
	{ "raf",     LFContextPictures, 0 },						// RAW format (digital camera)
	{ "ram",     LFContextAudio, 0 },
	{ "ras",     LFContextPictures, 0 },
	{ "raw",     LFContextPictures, 0 },
	{ "rl4",     LFContextPictures, 0 },
	{ "rle",     LFContextPictures, 0 },
	{ "rm",      LFContextVideos, 0 },
	{ "rss",     LFContextDocuments, 0 },
	{ "rt",      LFContextDocuments, 0 },
	{ "rtf",     LFContextDocuments, 0 },
	{ "rtx",     LFContextDocuments, 0 },
	{ "rv",      LFContextVideos, 0 },
	{ "s3m",     LFContextAudio, LFContextMusic },
	{ "sht",     LFContextDocuments, 0 },
	{ "shtm",    LFContextDocuments, 0 },
	{ "shtml",   LFContextDocuments, 0 },
	{ "snd",     LFContextAudio, 0 },
	{ "spi1d",   LFContextColorTables, 0 },
	{ "spi3d",   LFContextColorTables, 0 },
	{ "sr2",     LFContextPictures, 0 },						// RAW format (digital camera)
	{ "srt",     LFContextAudio, 0 },
	{ "svg",     LFContextPictures, 0 },
	{ "swf",     LFContextDocuments, 0 },
	{ "tex",     LFContextDocuments, 0 },
	{ "text",    LFContextDocuments, 0 },
	{ "tga",     LFContextPictures, 0 },
	{ "tif",     LFContextPictures, 0 },
	{ "tiff",    LFContextPictures, 0 },
	{ "tr2",     LFContextDocuments, LFContextBooks },
	{ "tr3",     LFContextDocuments, LFContextBooks },
	{ "tta",     LFContextAudio, 0 },
	{ "ttc",     LFContextFonts, 0 },
	{ "ttf",     LFContextFonts, 0 },
	{ "txt",     LFContextDocuments, 0 },
	{ "vb",      LFContextDocuments, 0 },
	{ "vcard",   LFContextContacts, 0 },
	{ "vcf",     LFContextContacts, 0 },
	{ "vob",     LFContextVideos, 0 },
	{ "voc",     LFContextAudio, 0 },
	{ "vox",     LFContextAudio, 0 },
	{ "wav",     LFContextAudio, 0 },
	{ "wave",    LFContextAudio, 0 },
	{ "webm",    LFContextVideos, 0 },
	{ "webp",    LFContextPictures, 0 },
	{ "wm",      LFContextAudio, 0 },
	{ "wma",     LFContextAudio, 0 },
	{ "wmv",     LFContextVideos, 0 },
	{ "woff",    LFContextFonts, 0 },
	{ "woff2",   LFContextFonts, 0 },
	{ "wv",      LFContextAudio, 0 },
	{ "xcf",     LFContextPictures, 0 },
	{ "xlb",     LFContextDocuments, 0 },
	{ "xls",     LFContextDocuments, 0 },
	{ "xlsb",    LFContextDocuments, 0 },
	{ "xlsm",    LFContextDocuments, 0 },
	{ "xlsx",    LFContextDocuments, 0 },
	{ "xlt",     LFContextDocuments, 0 },
	{ "xltb",    LFContextDocuments, 0 },
	{ "xltm",    LFContextDocuments, 0 },
	{ "xltx",    LFContextDocuments, 0 },
	{ "xm",      LFContextAudio, 0 },
	{ "xml",     LFContextDocuments, 0 },
	{ "xps",     LFContextDocuments, 0 },
	{ "xray",    LFContextColorTables, 0 },
	{ "xsl",     LFContextDocuments, 0 }
};

extern const BYTE ContextSlaves[LFLastPersistentContext+1] = {
	IDXTABLE_MASTER,		// LFContextAllFiles
	IDXTABLE_MASTER,		// LFContextFilters
	IDXTABLE_AUDIO,			// LFContextAudio
	IDXTABLE_PICTURES,		// LFContextPictures
	IDXTABLE_VIDEOS,		// LFContextVideos
	IDXTABLE_DOCUMENTS,		// LFContextDocuments
	IDXTABLE_MASTER,		// LFContextContacts
	IDXTABLE_MESSAGES,		// LFContextMessages
	IDXTABLE_MASTER,		// LFContextFonts
	IDXTABLE_MASTER,		// LFContextApps
	IDXTABLE_DOCUMENTS,		// LFContextBooks
	IDXTABLE_VIDEOS,		// LFContextMovies
	IDXTABLE_AUDIO,			// LFContextMusic
	IDXTABLE_MASTER,		// LFContextPodcasts
	IDXTABLE_VIDEOS,		// LFContextTVShows
	IDXTABLE_DOCUMENTS		// LFContextColorTables
};

#pragma data_seg()
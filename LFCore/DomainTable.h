struct RegisteredFile
{
	char Format[16];
	unsigned char DomainID;
};

RegisteredFile Registry[] = {
	{ "7z",     LFDomainArchives },
	{ "air",    LFDomainGeodata },
	{ "ans",    LFDomainPictures },
	{ "arc",    LFDomainArchives },
	{ "arj",    LFDomainArchives },
	{ "asc",    LFDomainDocuments },
	{ "asm",    LFDomainDocuments },
	{ "au",     LFDomainAudio },
	{ "avi",    LFDomainVideos },
	{ "bm",     LFDomainPictures },
	{ "bmp",    LFDomainPictures },
	{ "c",      LFDomainDocuments },
	{ "cab",    LFDomainArchives },
	{ "cda",    LFDomainAudio },
	{ "chm",    LFDomainDocuments },
	{ "cpp",    LFDomainDocuments },
	{ "cr2",    LFDomainPhotos },
	{ "cs",     LFDomainDocuments },
	{ "css",    LFDomainWeb },
	{ "csv",    LFDomainSpreadsheets },
	{ "cur",    LFDomainPictures },
	{ "dib",    LFDomainPictures },
	{ "div",    LFDomainVideos },
	{ "divx",   LFDomainVideos },
	{ "doc",    LFDomainDocuments },
	{ "docm",   LFDomainDocuments },
	{ "docx",   LFDomainDocuments },
	{ "dot",    LFDomainDocuments },
	{ "dotm",   LFDomainDocuments },
	{ "dotx",   LFDomainDocuments },
	{ "dsf",    LFDomainPictures },
	{ "dv",     LFDomainVideos },
	{ "dvi",    LFDomainVideos },
	{ "eml",    LFDomainMessages },
	{ "eot",    LFDomainFonts },
	{ "epa",    LFDomainPictures },
	{ "filter", LFDomainFilters },
	{ "flc",    LFDomainVideos },
	{ "fli",    LFDomainVideos },
	{ "flic",   LFDomainVideos },
	{ "flv",    LFDomainVideos },
	{ "fnt",    LFDomainFonts },
	{ "fon",    LFDomainFonts },
	{ "gfa",    LFDomainPictures },
	{ "gft",    LFDomainFonts },
	{ "gif",    LFDomainPictures },
	{ "gz",     LFDomainArchives },
	{ "h",      LFDomainDocuments },
	{ "hh",     LFDomainWeb },
	{ "hlp",    LFDomainDocuments },
	{ "htc",    LFDomainWeb },
	{ "htm",    LFDomainWeb },
	{ "html",   LFDomainWeb },
	{ "htx",    LFDomainWeb },
	{ "icl",    LFDomainPictures },
	{ "ico",    LFDomainPictures },
	{ "ics",    LFDomainEvents },
	{ "iff",    LFDomainAudio },
	{ "inc",    LFDomainDocuments },
	{ "jar",    LFDomainArchives },
	{ "jav",    LFDomainDocuments },
	{ "java",   LFDomainDocuments },
	{ "jfi",    LFDomainPhotos },
	{ "jfif",   LFDomainPhotos },
	{ "jpe",    LFDomainPhotos },
	{ "jpeg",   LFDomainPhotos },
	{ "jpg",    LFDomainPhotos },
	{ "js",     LFDomainWeb },
	{ "kml",    LFDomainGeodata },
	{ "kmz",    LFDomainGeodata },
	{ "lbm",    LFDomainPictures },
	{ "lha",    LFDomainArchives },
	{ "lhz",    LFDomainArchives },
	{ "log",    LFDomainDocuments },
	{ "m1v",    LFDomainVideos },
	{ "m2a",    LFDomainAudio },
	{ "m2v",    LFDomainVideos },
	{ "m4v",    LFDomainVideos },
	{ "mht",    LFDomainWeb },
	{ "mjp",    LFDomainVideos },
	{ "mjpeg",  LFDomainVideos },
	{ "mjpg",   LFDomainVideos },
	{ "mmap",   LFDomainDocuments },
	{ "mod",    LFDomainAudio },
	{ "mov",    LFDomainVideos },
	{ "mp1",    LFDomainAudio },
	{ "mp2",    LFDomainAudio },
	{ "mp3",    LFDomainAudio },
	{ "mp4",    LFDomainVideos },
	{ "mpa",    LFDomainAudio },
	{ "mpe",    LFDomainVideos },
	{ "mpeg",   LFDomainVideos },
	{ "mpg",    LFDomainVideos },
	{ "mpv",    LFDomainVideos },
	{ "nef",    LFDomainPictures },
	{ "nsm",    LFDomainDocuments },
	{ "odg",    LFDomainPictures },
	{ "odp",    LFDomainPresentations },
	{ "ods",    LFDomainSpreadsheets },
	{ "odt",    LFDomainDocuments },
	{ "ogg",    LFDomainAudio },
	{ "one",    LFDomainDocuments },
	{ "otf",    LFDomainFonts },
	{ "p",      LFDomainDocuments },
	{ "pas",    LFDomainDocuments },
	{ "pcd",    LFDomainPhotos },
	{ "pcx",    LFDomainPictures },
	{ "pdf",    LFDomainDocuments },
	{ "php",    LFDomainWeb },
	{ "pht",    LFDomainWeb },
	{ "phtm",   LFDomainWeb },
	{ "phtml",  LFDomainWeb },
	{ "png",    LFDomainPictures },
	{ "pot",    LFDomainPresentations },
	{ "potm",   LFDomainPresentations },
	{ "potx",   LFDomainPresentations },
	{ "pps",    LFDomainPresentations },
	{ "ppsm",   LFDomainPresentations },
	{ "ppsx",   LFDomainPresentations },
	{ "ppt",    LFDomainPresentations },
	{ "pptm",   LFDomainPresentations },
	{ "pptx",   LFDomainPresentations },
	{ "prn",    LFDomainDocuments },
	{ "ps",     LFDomainDocuments },
	{ "psd",    LFDomainPictures },
	{ "qt",     LFDomainVideos },
	{ "qtm",    LFDomainVideos },
	{ "ra",     LFDomainAudio },
	{ "raf",    LFDomainPhotos },
	{ "ram",    LFDomainAudio },
	{ "rar",    LFDomainArchives },
	{ "ras",    LFDomainPictures },
	{ "raw",    LFDomainPhotos },
	{ "rl4",    LFDomainPictures },
	{ "rle",    LFDomainPictures },
	{ "rm",     LFDomainVideos },
	{ "rss",    LFDomainWeb },
	{ "rt",     LFDomainDocuments },
	{ "rtf",    LFDomainDocuments },
	{ "rtx",    LFDomainDocuments },
	{ "rv",     LFDomainVideos },
	{ "s3m",    LFDomainAudio },
	{ "sht",    LFDomainWeb },
	{ "shtm",   LFDomainWeb },
	{ "shtml",  LFDomainWeb },
	{ "snd",    LFDomainAudio },
	{ "srt",    LFDomainAudio },
	{ "svg",    LFDomainPictures },
	{ "swf",    LFDomainWeb },
	{ "tar",    LFDomainArchives },
	{ "tex",    LFDomainDocuments },
	{ "text",   LFDomainDocuments },
	{ "tga",    LFDomainPictures },
	{ "tgz",    LFDomainArchives },
	{ "tif",    LFDomainPictures },
	{ "tiff",   LFDomainPictures },
	{ "ttf",    LFDomainFonts },
	{ "txt",    LFDomainDocuments },
	{ "vb",     LFDomainDocuments },
	{ "vcard",  LFDomainContacts },
	{ "vcf",    LFDomainContacts },
	{ "vob",    LFDomainVideos },
	{ "voc",    LFDomainAudio },
	{ "wav",    LFDomainAudio },
	{ "wave",   LFDomainAudio },
	{ "wm",     LFDomainAudio },
	{ "wmv",    LFDomainVideos },
	{ "wof",    LFDomainFonts },
	{ "woff",   LFDomainFonts },
	{ "xlb",    LFDomainSpreadsheets },
	{ "xls",    LFDomainSpreadsheets },
	{ "xlsb",   LFDomainSpreadsheets },
	{ "xlsm",   LFDomainSpreadsheets },
	{ "xlsx",   LFDomainSpreadsheets },
	{ "xlt",    LFDomainSpreadsheets },
	{ "xltb",   LFDomainSpreadsheets },
	{ "xltm",   LFDomainSpreadsheets },
	{ "xltx",   LFDomainSpreadsheets },
	{ "xm",     LFDomainAudio },
	{ "xml",    LFDomainWeb },
	{ "xps",    LFDomainDocuments },
	{ "xsl",    LFDomainWeb },
	{ "zip",    LFDomainArchives }
};

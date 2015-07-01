
struct RegisteredFile
{
	CHAR Format[7];
	BYTE ContextID;
};

static const RegisteredFile Registry[] = {
	{ "air",    LFContextDocuments },
	{ "ans",    LFContextPictures },
	{ "asc",    LFContextDocuments },
	{ "asm",    LFContextDocuments },
	{ "au",     LFContextAudio },
	{ "avi",    LFContextVideos },
	{ "bm",     LFContextPictures },
	{ "bmp",    LFContextPictures },
	{ "c",      LFContextDocuments },
	{ "cda",    LFContextAudio },
	{ "chm",    LFContextDocuments },
	{ "cpp",    LFContextDocuments },
	{ "cr2",    LFContextPictures },
	{ "cs",     LFContextDocuments },
	{ "css",    LFContextDocuments },
	{ "csv",    LFContextDocuments },
	{ "cur",    LFContextPictures },
	{ "dib",    LFContextPictures },
	{ "div",    LFContextVideos },
	{ "divx",   LFContextVideos },
	{ "doc",    LFContextDocuments },
	{ "docm",   LFContextDocuments },
	{ "docx",   LFContextDocuments },
	{ "dot",    LFContextDocuments },
	{ "dotm",   LFContextDocuments },
	{ "dotx",   LFContextDocuments },
	{ "dsf",    LFContextPictures },
	{ "dv",     LFContextVideos },
	{ "dvi",    LFContextVideos },
	{ "eml",    LFContextMessages },
	{ "eot",    LFContextAllFiles },
	{ "epa",    LFContextPictures },
	{ "filter", LFContextFilters },
	{ "flc",    LFContextVideos },
	{ "fli",    LFContextVideos },
	{ "flic",   LFContextVideos },
	{ "flv",    LFContextVideos },
	{ "fnt",    LFContextAllFiles },
	{ "fon",    LFContextAllFiles },
	{ "gfa",    LFContextPictures },
	{ "gft",    LFContextAllFiles },
	{ "gif",    LFContextPictures },
	{ "h",      LFContextDocuments },
	{ "hh",     LFContextDocuments },
	{ "hlp",    LFContextDocuments },
	{ "htc",    LFContextDocuments },
	{ "htm",    LFContextDocuments },
	{ "html",   LFContextDocuments },
	{ "htx",    LFContextDocuments },
	{ "icl",    LFContextPictures },
	{ "ico",    LFContextPictures },
	{ "ics",    LFContextEvents },
	{ "iff",    LFContextAudio },
	{ "inc",    LFContextDocuments },
	{ "jav",    LFContextDocuments },
	{ "java",   LFContextDocuments },
	{ "jfi",    LFContextPictures },
	{ "jfif",   LFContextPictures },
	{ "jpe",    LFContextPictures },
	{ "jpeg",   LFContextPictures },
	{ "jpg",    LFContextPictures },
	{ "js",     LFContextDocuments },
	{ "kml",    LFContextDocuments },
	{ "kmz",    LFContextDocuments },
	{ "lbm",    LFContextPictures },
	{ "log",    LFContextDocuments },
	{ "m1v",    LFContextVideos },
	{ "m2a",    LFContextAudio },
	{ "m2v",    LFContextVideos },
	{ "m4v",    LFContextVideos },
	{ "mht",    LFContextDocuments },
	{ "mjp",    LFContextVideos },
	{ "mjpeg",  LFContextVideos },
	{ "mjpg",   LFContextVideos },
	{ "mmap",   LFContextDocuments },
	{ "mod",    LFContextAudio },
	{ "mov",    LFContextVideos },
	{ "mp1",    LFContextAudio },
	{ "mp2",    LFContextAudio },
	{ "mp3",    LFContextAudio },
	{ "mp4",    LFContextVideos },
	{ "mpa",    LFContextAudio },
	{ "mpe",    LFContextVideos },
	{ "mpeg",   LFContextVideos },
	{ "mpg",    LFContextVideos },
	{ "mpv",    LFContextVideos },
	{ "nef",    LFContextPictures },
	{ "nsm",    LFContextDocuments },
	{ "odg",    LFContextPictures },
	{ "odp",    LFContextDocuments },
	{ "ods",    LFContextDocuments },
	{ "odt",    LFContextDocuments },
	{ "ogg",    LFContextAudio },
	{ "one",    LFContextDocuments },
	{ "otf",    LFContextAllFiles },
	{ "p",      LFContextDocuments },
	{ "pas",    LFContextDocuments },
	{ "pcd",    LFContextPictures },
	{ "pcx",    LFContextPictures },
	{ "pdf",    LFContextDocuments },
	{ "php",    LFContextDocuments },
	{ "pht",    LFContextDocuments },
	{ "phtm",   LFContextDocuments },
	{ "phtml",  LFContextDocuments },
	{ "png",    LFContextPictures },
	{ "pot",    LFContextDocuments },
	{ "potm",   LFContextDocuments },
	{ "potx",   LFContextDocuments },
	{ "pps",    LFContextDocuments },
	{ "ppsm",   LFContextDocuments },
	{ "ppsx",   LFContextDocuments },
	{ "ppt",    LFContextDocuments },
	{ "pptm",   LFContextDocuments },
	{ "pptx",   LFContextDocuments },
	{ "prn",    LFContextDocuments },
	{ "ps",     LFContextDocuments },
	{ "psd",    LFContextPictures },
	{ "qt",     LFContextVideos },
	{ "qtm",    LFContextVideos },
	{ "ra",     LFContextAudio },
	{ "raf",    LFContextPictures },
	{ "ram",    LFContextAudio },
	{ "ras",    LFContextPictures },
	{ "raw",    LFContextPictures },
	{ "rl4",    LFContextPictures },
	{ "rle",    LFContextPictures },
	{ "rm",     LFContextVideos },
	{ "rss",    LFContextDocuments },
	{ "rt",     LFContextDocuments },
	{ "rtf",    LFContextDocuments },
	{ "rtx",    LFContextDocuments },
	{ "rv",     LFContextVideos },
	{ "s3m",    LFContextAudio },
	{ "sht",    LFContextDocuments },
	{ "shtm",   LFContextDocuments },
	{ "shtml",  LFContextDocuments },
	{ "snd",    LFContextAudio },
	{ "srt",    LFContextAudio },
	{ "svg",    LFContextPictures },
	{ "swf",    LFContextDocuments },
	{ "tex",    LFContextDocuments },
	{ "text",   LFContextDocuments },
	{ "tga",    LFContextPictures },
	{ "tif",    LFContextPictures },
	{ "tiff",   LFContextPictures },
	{ "ttf",    LFContextAllFiles },
	{ "txt",    LFContextDocuments },
	{ "vb",     LFContextDocuments },
	{ "vcard",  LFContextContacts },
	{ "vcf",    LFContextContacts },
	{ "vob",    LFContextVideos },
	{ "voc",    LFContextAudio },
	{ "wav",    LFContextAudio },
	{ "wave",   LFContextAudio },
	{ "wm",     LFContextAudio },
	{ "wmv",    LFContextVideos },
	{ "wof",    LFContextAllFiles },
	{ "woff",   LFContextAllFiles },
	{ "xlb",    LFContextDocuments },
	{ "xls",    LFContextDocuments },
	{ "xlsb",   LFContextDocuments },
	{ "xlsm",   LFContextDocuments },
	{ "xlsx",   LFContextDocuments },
	{ "xlt",    LFContextDocuments },
	{ "xltb",   LFContextDocuments },
	{ "xltm",   LFContextDocuments },
	{ "xltx",   LFContextDocuments },
	{ "xm",     LFContextAudio },
	{ "xml",    LFContextDocuments },
	{ "xps",    LFContextDocuments },
	{ "xsl",    LFContextDocuments },
};

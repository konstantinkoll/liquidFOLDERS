
#pragma once
#include "LF.h"


extern const FMTID SHPropertyStorage;
extern const FMTID SHPropertyQuery;
extern const FMTID SHPropertySummary;
extern const FMTID SHPropertyDocuments;
extern const FMTID SHPropertyImage;
extern const FMTID SHPropertyAudio;
extern const FMTID SHPropertyVideo;
extern const FMTID SHPropertyMedia;
extern const FMTID SHPropertyPhoto;
extern const FMTID SHPropertyMusic;
extern const FMTID SHPropertyVersion;
extern const FMTID SHPropertyUnnamed1;
extern const FMTID SHPropertyUnnamed2;
extern const FMTID SHPropertyUnnamed3;
extern const FMTID SHPropertyUnnamed4;
extern const FMTID SHPropertyUnnamed5;
extern const FMTID SHPropertyUnnamed6;
extern const FMTID SHPropertyUnnamed7;


extern const LFContextProperties CtxProperties[LFContextCount];
extern const LFAttributeProperties AttrProperties[LFAttributeCount];
extern const LFTypeProperties TypeProperties[LFTypeCount];


struct SpecialAttributeName
{
	UINT Attr;
	UINT nID;
	UINT IconID;
	BOOL SortDescending;
	UINT64 ContextSet;
};

#define ALLCONTEXTSSET                 ((1ull<<LFContextCount)-1)
#define SPECIALATTRIBUTENAMESCOUNT     11

extern const SpecialAttributeName SpecialAttributeNames[SPECIALATTRIBUTENAMESCOUNT];


inline BOOL ContextMoveAllowed(BYTE SystemContextID, BYTE UserContextID)
{
	assert(SystemContextID<LFContextCount);
	assert(UserContextID<LFContextCount);

	return !UserContextID || (SystemContextID==UserContextID) || (CtxProperties[SystemContextID].AllowMoveToContext & (1ull<<UserContextID));
}

inline BOOL IsAttributeSortDescending(UINT Context, UINT Attr)
{
	assert(Context<LFContextCount);
	assert(Attr<LFAttributeCount);

	for (UINT a=0; a<SPECIALATTRIBUTENAMESCOUNT; a++)
		if ((SpecialAttributeNames[a].Attr==Attr) && (SpecialAttributeNames[a].ContextSet & (1ull<<Context)))
			return SpecialAttributeNames[a].SortDescending;

	return (TypeProperties[AttrProperties[Attr].Type].DataFlags & LFDataSortDescending);
}

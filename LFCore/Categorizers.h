
#pragma once
#include "LF.h"
#include "LFItemDescriptor.h"



// CCategorizer
//

class CCategorizer
{
public:
	CCategorizer(ATTRIBUTE Attr);

	LFItemDescriptor* GetFolder(LFItemDescriptor* pItemDescriptor, LFFilter* pFilter, LFFileSummary& FileSummary, INT FirstAggregate=-1, INT LastAggregate=-1) const;
	BOOL IsEqual(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const;

protected:
	virtual BOOL CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const;
	virtual void GetFilterValue(LFVariantData& VData, LFItemDescriptor* pItemDescriptor) const;
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const;

	ATTRIBUTE m_Attr;
};


// CNameCategorizer
//

class CNameCategorizer : public CCategorizer
{
public:
	CNameCategorizer();

	static BOOL GetNamePrefix(LPCWSTR FullName, LPWSTR pStr, SIZE_T cCount);

protected:
	virtual BOOL CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const;
	virtual void GetFilterValue(LFVariantData& VData, LFItemDescriptor* pItemDescriptor) const;
};


// CURLCategorizer
//

class CURLCategorizer : public CCategorizer
{
public:
	CURLCategorizer();

	static void GetServer(LPCSTR URL, LPSTR pStr, SIZE_T cCount);

protected:
	virtual BOOL CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const;
	virtual void GetFilterValue(LFVariantData& VData, LFItemDescriptor* pItemDescriptor) const;
};


// CIATACategorizer
//

class CIATACategorizer : public CCategorizer
{
public:
	CIATACategorizer();

protected:
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const;

	LPCWSTR m_Separator;
};


// CChannelsCategorizer
//

class CChannelsCategorizer : public CCategorizer
{
public:
	CChannelsCategorizer();

	static UINT GetChannelsCategory(const UINT Channels);

protected:
	virtual BOOL CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const;
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const;
};


// CRatingCategorizer
//

class CRatingCategorizer : public CCategorizer
{
public:
	CRatingCategorizer(ATTRIBUTE Attr);

	static BYTE GetRatingCategory(const BYTE Rating);

protected:
	virtual BOOL CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const;
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const;
};


// CSizeCategorizer
//

class CSizeCategorizer : public CCategorizer
{
public:
	CSizeCategorizer(ATTRIBUTE Attr);

	static UINT GetSizeCategory(const INT64 Size);

protected:
	virtual BOOL CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const;
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const;
};


// CDateCategorizer
//

class CDateCategorizer : public CCategorizer
{
public:
	CDateCategorizer(ATTRIBUTE Attr);

	static void GetDate(const LPFILETIME Time, LPSYSTEMTIME Date);
	static void GetDay(const LPFILETIME Time, LPFILETIME Day);

protected:
	virtual BOOL CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const;
	virtual void GetFilterValue(LFVariantData& VData, LFItemDescriptor* pItemDescriptor) const;
};


// CDurationCategorizer
//

class CDurationCategorizer : public CCategorizer
{
public:
	CDurationCategorizer(ATTRIBUTE Attr);

	static UINT GetDurationCategory(const UINT Duration);

protected:
	virtual BOOL CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const;
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const;
};


// CMegapixelCategorizer
//

class CMegapixelCategorizer : public CCategorizer
{
public:
	CMegapixelCategorizer(ATTRIBUTE Attr);

protected:
	virtual BOOL CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const;
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const;
};


#pragma once
#include "LF.h"
#include "LFItemDescriptor.h"


// CCategorizer
//

class CCategorizer
{
public:
	CCategorizer(UINT Attr);

	LFItemDescriptor* GetFolder(LFItemDescriptor* i, LFFilter* pFilter);
	BOOL IsEqual(LFItemDescriptor* i1, LFItemDescriptor* i2);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i, LFFilterCondition* pNext);
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* i);

	UINT m_Attr;
};


// CNameCategorizer
//

class CNameCategorizer : public CCategorizer
{
public:
	CNameCategorizer();

	static BOOL GetNamePrefix(WCHAR* FullName, WCHAR* pBuffer);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i, LFFilterCondition* pNext);
};


// CURLCategorizer
//

class CURLCategorizer : public CCategorizer
{
public:
	CURLCategorizer();

	static void GetServer(CHAR* URL, CHAR* Server, SIZE_T cCount);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i, LFFilterCondition* pNext);
};


// CIATACategorizer
//

class CIATACategorizer : public CCategorizer
{
public:
	CIATACategorizer();

protected:
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* i);
};


// CRatingCategorizer
//

class CRatingCategorizer : public CCategorizer
{
public:
	CRatingCategorizer(UINT Attr);

	static BYTE GetRatingCategory(const BYTE Rating);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* i);
};


// CSizeCategorizer
//

class CSizeCategorizer : public CCategorizer
{
public:
	CSizeCategorizer(UINT Attr);

	static UINT GetSizeCategory(const INT64 Size);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* i);
};


// CDateCategorizer
//

class CDateCategorizer : public CCategorizer
{
public:
	CDateCategorizer(UINT Attr);

	static void GetDate(const FILETIME* Time, LPSYSTEMTIME Date);
	static void GetDay(const FILETIME* Time, LPFILETIME Day);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i, LFFilterCondition* pNext);
};


// CDurationCategorizer
//

class CDurationCategorizer : public CCategorizer
{
public:
	CDurationCategorizer(UINT Attr);

	static UINT GetDurationCategory(const UINT Duration);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* i);
};


// CMegapixelCategorizer
//

class CMegapixelCategorizer : public CCategorizer
{
public:
	CMegapixelCategorizer(UINT Attr);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* i);
};

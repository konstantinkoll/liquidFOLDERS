#pragma once
#include "LF.h"
#include "LFItemDescriptor.h"


// CCategorizer
//

class CCategorizer
{
public:
	CCategorizer(UINT Attr);

	LFItemDescriptor* GetFolder(LFItemDescriptor* i, LFFilter* f);
	BOOL IsEqual(LFItemDescriptor* i1, LFItemDescriptor* i2);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i);

	UINT m_Attr;
};


// CDateCategorizer
//

class CDateCategorizer : public CCategorizer
{
public:
	CDateCategorizer(UINT Attr);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i);
};


// CRatingCategorizer
//

class CRatingCategorizer : public CCategorizer
{
public:
	CRatingCategorizer(UINT Attr);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
};


// IATACategorizer
//

class IATACategorizer : public CCategorizer
{
public:
	IATACategorizer(UINT Attr);

protected:
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
};


// SizeCategorizer
//

class SizeCategorizer : public CCategorizer
{
public:
	SizeCategorizer(UINT Attr);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
};


// DurationCategorizer
//

class DurationCategorizer : public CCategorizer
{
public:
	DurationCategorizer(UINT Attr);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
};


// CNameCategorizer
//

class CNameCategorizer : public CCategorizer
{
public:
	CNameCategorizer(UINT Attr);

protected:
	static BOOL GetNamePrefix(WCHAR* FullName, WCHAR* Buffer);

	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i);
};


// MegapixelCategorizer
//

class MegapixelCategorizer : public CCategorizer
{
public:
	MegapixelCategorizer(UINT Attr);

protected:
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i);
};


// URLCategorizer
//

class URLCategorizer : public CCategorizer
{
public:
	URLCategorizer(UINT Attr);

protected:
	virtual BOOL CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i);
};

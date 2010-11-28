#pragma once
#include "liquidFOLDERS.h"
#include "LFItemDescriptor.h"


// CCategorizer
//

class CCategorizer
{
public:
	CCategorizer(unsigned int _attr);

	LFItemDescriptor* GetFolder(LFItemDescriptor* i, LFFilter* f);
	bool IsEqual(LFItemDescriptor* i1, LFItemDescriptor* i2);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i);

	unsigned int attr;
};


// DateCategorizer
//

class DateCategorizer : public CCategorizer
{
public:
	DateCategorizer(unsigned int _attr);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i);
};


// RatingCategorizer
//

class RatingCategorizer : public CCategorizer
{
public:
	RatingCategorizer(unsigned int _attr);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
};


// UnicodeCategorizer
//

class UnicodeCategorizer : public CCategorizer
{
public:
	UnicodeCategorizer(unsigned int _attr);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i);
};


// AnsiCategorizer
//

class AnsiCategorizer : public CCategorizer
{
public:
	AnsiCategorizer(unsigned int _attr);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
};


// UINTCategorizer
//

class UINTCategorizer : public CCategorizer
{
public:
	UINTCategorizer(unsigned int _attr);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
};


// IATACategorizer
//

class IATACategorizer : public AnsiCategorizer
{
public:
	IATACategorizer(unsigned int _attr);

protected:
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
};


// CoordCategorizer
//

class CoordCategorizer : public CCategorizer
{
public:
	CoordCategorizer(unsigned int _attr);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
};


// SizeCategorizer
//

class SizeCategorizer : public CCategorizer
{
public:
	SizeCategorizer(unsigned int _attr);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
};


// NameCategorizer
//

class NameCategorizer : public CCategorizer
{
public:
	NameCategorizer(unsigned int _attr);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i);
};


// DurationBitrateCategorizer
//

class DurationBitrateCategorizer : public CCategorizer
{
public:
	DurationBitrateCategorizer(unsigned int _attr);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
};


// MegapixelCategorizer
//

class MegapixelCategorizer : public CCategorizer
{
public:
	MegapixelCategorizer(unsigned int _attr);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i);
};


// URLCategorizer
//

class URLCategorizer : public CCategorizer
{
public:
	URLCategorizer(unsigned int _attr);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual void CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i);
	virtual LFFilterCondition* GetCondition(LFItemDescriptor* i);
};

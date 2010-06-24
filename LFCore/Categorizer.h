#pragma once
#include "liquidFOLDERS.h"
#include "LFItemDescriptor.h"


// CCategorizer
//

class CCategorizer
{
public:
	CCategorizer(unsigned int _attr, unsigned int _icon);

	virtual LFItemDescriptor* GetFolder(LFItemDescriptor* i);

	bool IsEqual(LFItemDescriptor* i1, LFItemDescriptor* i2);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);

	unsigned int attr;
	unsigned int icon;
};


// DateCategorizer
//

class DateCategorizer : public CCategorizer
{
public:
	DateCategorizer(unsigned int _attr);

	virtual LFItemDescriptor* GetFolder(LFItemDescriptor* i);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
};


// RatingCategorizer
//

class RatingCategorizer : public CCategorizer
{
public:
	RatingCategorizer(unsigned int _attr);

	virtual LFItemDescriptor* GetFolder(LFItemDescriptor* i);

protected:
	virtual bool Compare(LFItemDescriptor* i1, LFItemDescriptor* i2);
};

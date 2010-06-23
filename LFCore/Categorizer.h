#pragma once
#include "liquidFOLDERS.h"
#include "LFItemDescriptor.h"


// CCategorizer
//

class CCategorizer
{
public:
	CCategorizer(unsigned int _attr, unsigned int _icon);

	virtual bool IsEqual(LFItemDescriptor* i1, LFItemDescriptor* i2)=0;
	virtual LFItemDescriptor* GetFolder(LFItemDescriptor* i);

protected:
	unsigned int attr;
	unsigned int icon;
};


// DateCategorizer
//

class DateCategorizer : public CCategorizer
{
public:
	DateCategorizer(unsigned int _attr);

	virtual bool IsEqual(LFItemDescriptor* i1, LFItemDescriptor* i2);
	virtual LFItemDescriptor* GetFolder(LFItemDescriptor* i);
};

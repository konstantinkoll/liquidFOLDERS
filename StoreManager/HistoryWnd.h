
#pragma once
#include "liquidFOLDERS.h"
#include "CFileView.h"


// Breadcrumbs
//

struct BreadcrumbItem
{
	BreadcrumbItem* next;
	LFFilter* filter;
	FVPersistentData data;
};

void AddBreadcrumbItem(BreadcrumbItem** bi, LFFilter* f, FVPersistentData& data);
void ConsumeBreadcrumbItem(BreadcrumbItem** bi, LFFilter** f, FVPersistentData* data);
void DeleteBreadcrumbs(BreadcrumbItem** bi);


#include "stdafx.h"
#include "HistoryWnd.h"
#include "Resource.h"
#include "liquidFOLDERS.h"
#include "LFCore.h"


// Breadcrumbs
//

void AddBreadcrumbItem(BreadcrumbItem** bi, LFFilter* f, FVPersistentData& data)
{
	BreadcrumbItem* add = new BreadcrumbItem;
	add->next = *bi;
	add->filter = f;
	add->data = data;
	*bi = add;
}

void ConsumeBreadcrumbItem(BreadcrumbItem** bi, LFFilter** f, FVPersistentData* data)
{
	*f = NULL;
	ZeroMemory(data, sizeof(FVPersistentData));

	if (*bi)
	{
		*f = (*bi)->filter;
		*data = (*bi)->data;
		BreadcrumbItem* victim = *bi;
		*bi = (*bi)->next;
		delete victim;
	}
}

void DeleteBreadcrumbs(BreadcrumbItem** bi)
{
	while (*bi)
	{
		BreadcrumbItem* victim = *bi;
		*bi = (*bi)->next;
		LFFreeFilter(victim->filter);
		delete victim;
	}
}

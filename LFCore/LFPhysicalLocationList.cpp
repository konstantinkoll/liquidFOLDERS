#include "stdafx.h"
#include "CIndex.h"
#include "LFCore.h"
#include "LFPhysicalLocationList.h"
#include "Mutex.h"
#include "Stores.h"
#include <assert.h>
#include <malloc.h>


LFPhysicalLocationList::LFPhysicalLocationList()
	: DynArray()
{
}

LFPhysicalLocationList::~LFPhysicalLocationList()
{
	if (m_Items)
		for (unsigned int a=0; a<m_ItemCount; a++)
			if (m_Items[a].pidlFQ)
				FreePIDL(m_Items[a].pidlFQ);
}

bool LFPhysicalLocationList::AddItemDescriptor(LFItemDescriptor* i)
{
	assert(i);

	LFPLL_Item item;
	ZeroMemory(&item, sizeof(item));

	strcpy_s(item.StoreID, LFKeySize, i->StoreID);
	strcpy_s(item.FileID, LFKeySize, i->CoreAttributes.FileID);
	wcscpy_s(item.FileName, 256, i->CoreAttributes.FileName);

	return DynArray::AddItem(item);
}

void LFPhysicalLocationList::SetError(char* key, unsigned int res)
{
	for (unsigned int a=0; a<m_ItemCount; a++)
		if (!m_Items[a].Processed)
			if (strcmp(m_Items[a].StoreID, key)==0)
			{
				m_Items[a].LastError = m_LastError = res;
				m_Items[a].Processed = true;
			}
}

void LFPhysicalLocationList::Resolve(bool IncludePIDL)
{
	// Reset
	for (unsigned int a=0; a<m_ItemCount; a++)
	{
		m_Items[a].LastError = LFOk;
		m_Items[a].Processed = false;
	}

	// Retrieve physical paths
	for (unsigned int a=0; a<m_ItemCount; a++)
		if ((m_Items[a].LastError==LFOk) && (!m_Items[a].Processed))
		{
			CIndex* idx1;
			CIndex* idx2;
			HANDLE StoreLock = NULL;
			unsigned int res = OpenStore(m_Items[a].StoreID, true, idx1, idx2, NULL, &StoreLock);

			if (res==LFOk)
			{
				if (idx1)
				{
					idx1->ResolvePhysicalLocation(this);
					delete idx1;
				}
				if (idx2)
					delete idx2;

				ReleaseMutexForStore(StoreLock);
			}
			else
			{
				// Cannot open index, so mark all subsequent files in the same store as processed
				for (unsigned int b=a; b<m_ItemCount; b++)
					if ((strcmp(m_Items[b].StoreID, m_Items[a].StoreID)==0) && (!m_Items[b].Processed))
					{
						m_Items[b].LastError = m_LastError = res;
						m_Items[b].Processed = true;
					}
			}
		}

	// PIDLs
	if (IncludePIDL)
	{
		IShellFolder* pDesktop = NULL;
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
		{
			for (unsigned int a=0; a<m_ItemCount; a++)
				if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
					if (FAILED(pDesktop->ParseDisplayName(NULL, NULL, &m_Items[a].Path[4], NULL, &m_Items[a].pidlFQ, NULL)))
						m_Items[a].LastError = m_LastError = LFIllegalPhysicalPath;

			pDesktop->Release();
		}
	}
}

LPITEMIDLIST LFPhysicalLocationList::DetachPIDL(unsigned int idx)
{
	assert(idx<m_ItemCount);

	LPITEMIDLIST pidl = m_Items[idx].pidlFQ;
	m_Items[idx].pidlFQ = NULL;

	return pidl;
}

HGLOBAL LFPhysicalLocationList::CreateDropFiles()
{
	unsigned int cChars = 0;
	for (unsigned int a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
			cChars += (unsigned int)wcslen(&m_Items[a].Path[4])+1;

	unsigned int szBuffer = sizeof(DROPFILES)+sizeof(wchar_t)*(cChars+1);
	HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE, szBuffer);
	if (!hG)
		return NULL;

	DROPFILES* pDrop = (DROPFILES*)GlobalLock(hG);
	if (!pDrop)
	{
		GlobalFree(hG);
		return NULL;
	}

	pDrop->pFiles = sizeof(DROPFILES);
	pDrop->fNC = TRUE;
	pDrop->pt.x = pDrop->pt.y = 0;
	pDrop->fWide = TRUE;

	wchar_t* ptr = (wchar_t*)(((unsigned char*)pDrop)+sizeof(DROPFILES));
	for (unsigned int a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
		{
#pragma warning(push)
#pragma warning(disable: 4996)
			wcscpy(ptr, &m_Items[a].Path[4]);
#pragma warning(pop)
			ptr += wcslen(&m_Items[a].Path[4])+1;
		}

	*ptr = L'\0';

	GlobalUnlock(hG);
	return hG;
}

HGLOBAL LFPhysicalLocationList::CreateLiquidFiles()
{
	unsigned int cFiles = 0;
	for (unsigned int a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
			cFiles++;

	unsigned int szBuffer = sizeof(LIQUIDFILES)+sizeof(char)*cFiles*LFKeySize*2;
	HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE, szBuffer);
	if (!hG)
		return NULL;

	LIQUIDFILES* pFiles = (LIQUIDFILES*)GlobalLock(hG);
	if (!pFiles)
	{
		GlobalFree(hG);
		return NULL;
	}

	pFiles->pFiles = sizeof(LIQUIDFILES);
	pFiles->cFiles = cFiles;

	char* ptr = (char*)(((unsigned char*)pFiles)+sizeof(LIQUIDFILES));
	for (unsigned int a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
		{
			strcpy_s(ptr, LFKeySize, m_Items[a].StoreID);
			ptr += LFKeySize;
			strcpy_s(ptr, LFKeySize, m_Items[a].FileID);
			ptr += LFKeySize;
		}

	GlobalUnlock(hG);
	return hG;
}

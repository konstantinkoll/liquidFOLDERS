#pragma once
#include "LF.h"


struct ThumbnailData
{
	CHAR StoreID[LFKeySize];
	CHAR FileID[LFKeySize];
	HBITMAP hBmp;
};


template <UINT C>
class ThumbnailList
{
public:
	ThumbnailList();
	~ThumbnailList();

	void AddItem(ThumbnailData& td);
	BOOL Lookup(LFItemDescriptor* i, ThumbnailData& td);

protected:
	void FreeItem(UINT idx);

	ThumbnailData m_Items[C];
	INT m_NextPtr;
};


template <UINT C>
ThumbnailList<C>::ThumbnailList()
{
	ZeroMemory(&m_Items, sizeof(m_Items));
	m_NextPtr = 0;
}

template <UINT C>
ThumbnailList<C>::~ThumbnailList()
{
	for (UINT a=0; a<C; a++)
		FreeItem(a);
}

template <UINT C>
__forceinline void ThumbnailList<C>::FreeItem(UINT idx)
{
	if (m_Items[idx].hBmp)
		DeleteObject(m_Items[idx].hBmp);
}

template <UINT C>
void ThumbnailList<C>::AddItem(ThumbnailData& td)
{
	FreeItem(m_NextPtr);

	m_Items[m_NextPtr] = td;
	if (++m_NextPtr>=C)
		m_NextPtr = 0;
}

template <UINT C>
BOOL ThumbnailList<C>::Lookup(LFItemDescriptor* i, ThumbnailData& td)
{
	INT Ptr = m_NextPtr;

	for (UINT a=0; a<C; a++)
	{
		if (--Ptr<0)
			Ptr = C-1;

		if ((strcmp(m_Items[Ptr].StoreID, i->StoreID)==0) && (strcmp(m_Items[Ptr].FileID, i->CoreAttributes.FileID)==0))
		{
			td = m_Items[Ptr];

			if (Ptr!=m_NextPtr)
				std::swap(m_Items[m_NextPtr], m_Items[Ptr]);

			if (++m_NextPtr>=C)
				m_NextPtr = 0;

			return TRUE;
		}
	}

	return FALSE;
}


#pragma once
#include "LF.h"


struct ThumbnailData
{
	STOREID StoreID;
	FILEID FileID;
	HBITMAP hBitmap;
	BOOL HasFrame;
	BOOL HasBackground;
};


template <UINT C>
class ThumbnailList
{
public:
	ThumbnailList();
	~ThumbnailList();

	void AddItem(ThumbnailData& Thumbnail);
	BOOL Lookup(const LFItemDescriptor* pItemDescriptor, ThumbnailData& Thumbnail);

protected:
	void FreeItem(UINT Index);

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
__forceinline void ThumbnailList<C>::FreeItem(UINT Index)
{
	if (m_Items[Index].hBitmap)
		DeleteObject(m_Items[Index].hBitmap);
}

template <UINT C>
void ThumbnailList<C>::AddItem(ThumbnailData& Thumbnail)
{
	FreeItem(m_NextPtr);

	m_Items[m_NextPtr] = Thumbnail;
	if (++m_NextPtr>=C)
		m_NextPtr = 0;
}

template <UINT C>
BOOL ThumbnailList<C>::Lookup(const LFItemDescriptor* pItemDescriptor, ThumbnailData& Thumbnail)
{
	INT pChar = m_NextPtr;

	for (UINT a=0; a<C; a++)
	{
		if (--pChar<0)
			pChar = C-1;

		if ((strcmp(m_Items[pChar].StoreID, pItemDescriptor->StoreID)==0) && (strcmp(m_Items[pChar].FileID, pItemDescriptor->CoreAttributes.FileID)==0))
		{
			Thumbnail = m_Items[pChar];

			if (pChar!=m_NextPtr)
			{
				ThumbnailData Temp = m_Items[m_NextPtr];
				m_Items[m_NextPtr] = m_Items[pChar];
				m_Items[pChar] = Temp;
			}

			if (++m_NextPtr>=C)
				m_NextPtr = 0;

			return TRUE;
		}
	}

	return FALSE;
}

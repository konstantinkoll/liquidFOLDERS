
#pragma once
#include "LFMemorySort.h"
#include <assert.h>


template <typename T, UINT FirstAlloc, UINT SubsequentAlloc>
class LFDynArray
{
public:
	LFDynArray();
	LFDynArray(UINT AllocItems);
	~LFDynArray();

	BOOL AddItem(const T& Item);
	BOOL InsertEmpty(UINT Pos, UINT Count=1, BOOL ZeroOut=TRUE);
	void DeleteItems(UINT Pos, UINT Count=1);
	void SortItems(PFNCOMPARE zCompare, ATTRIBUTE Attr=0, BOOL Descending=FALSE, BOOL Parameter1=FALSE, BOOL Parameter2=FALSE);

	const T& operator[](const SIZE_T Index) const;
	T& operator[](const SIZE_T Index);

	UINT m_ItemCount;

protected:
	UINT m_ItemDataAllocated;
	T* m_Items;

private:
	BOOL Allocate(UINT Count=1);
};


template <typename T, UINT FirstAlloc, UINT SubsequentAlloc>
LFDynArray<T, FirstAlloc, SubsequentAlloc>::LFDynArray()
{
	m_Items = (T*)malloc((m_ItemDataAllocated=FirstAlloc)*sizeof(T));
	m_ItemCount = 0;
}

template <typename T, UINT FirstAlloc, UINT SubsequentAlloc>
LFDynArray<T, FirstAlloc, SubsequentAlloc>::LFDynArray(UINT AllocItems)
{
	if (AllocItems)
	{
		m_Items = (T*)malloc((m_ItemDataAllocated=max(FirstAlloc, AllocItems))*sizeof(T));
	}
	else
	{
		m_Items = NULL;
		m_ItemDataAllocated = 0;
	}

	m_ItemCount = 0;
}

template <typename T, UINT FirstAlloc, UINT SubsequentAlloc>
LFDynArray<T, FirstAlloc, SubsequentAlloc>::~LFDynArray()
{
	free(m_Items);
}

template <typename T, UINT FirstAlloc, UINT SubsequentAlloc>
inline BOOL LFDynArray<T, FirstAlloc, SubsequentAlloc>::Allocate(UINT Count=1)
{
	assert(m_ItemCount<=m_ItemDataAllocated);

	if (m_ItemCount+Count>m_ItemDataAllocated)
		if ((m_Items=(T*)realloc(m_Items, (m_ItemDataAllocated=(m_ItemDataAllocated ? m_ItemDataAllocated+SubsequentAlloc : max(FirstAlloc, Count)))*sizeof(T)))==NULL)
			return FALSE;

	return TRUE;
}

template <typename T, UINT FirstAlloc, UINT SubsequentAlloc>
BOOL LFDynArray<T, FirstAlloc, SubsequentAlloc>::AddItem(const T& Item)
{
	if (!Allocate())
		return FALSE;

	m_Items[m_ItemCount++] = Item;

	return TRUE;
}

template <typename T, UINT FirstAlloc, UINT SubsequentAlloc>
BOOL LFDynArray<T, FirstAlloc, SubsequentAlloc>::InsertEmpty(UINT Pos, UINT Count=1, BOOL ZeroOut=TRUE)
{
	if (!Count)
		return TRUE;

	if ((Pos>m_ItemCount+1) || !Allocate(Count))
		return FALSE;

	if (!Allocate(Count))
		return FALSE;

	for (INT a=((INT)(m_ItemCount))-1; a>=(INT)Pos; a--)
		m_Items[a+Count] = m_Items[a];

	if (ZeroOut)
		ZeroMemory(&m_Items[Pos], Count*sizeof(T));

	m_ItemCount += Count;

	return TRUE;
}

template <typename T, UINT FirstAlloc, UINT SubsequentAlloc>
void LFDynArray<T, FirstAlloc, SubsequentAlloc>::DeleteItems(UINT Pos, UINT Count=1)
{
	if (!Count)
		return;

	for (UINT a=Pos; a<m_ItemCount-Count; a++)
		m_Items[a] = m_Items[a+Count];

	m_ItemCount -= Count;
}

template <typename T, UINT FirstAlloc, UINT SubsequentAlloc>
void LFDynArray<T, FirstAlloc, SubsequentAlloc>::SortItems(PFNCOMPARE zCompare, ATTRIBUTE Attr, BOOL Descending, BOOL Parameter1, BOOL Parameter2)
{
	LFSortMemory(m_Items, m_ItemCount, sizeof(T), zCompare, Attr, Descending, Parameter1, Parameter2);
}

template <typename T, UINT FirstAlloc, UINT SubsequentAlloc>
const T& LFDynArray<T, FirstAlloc, SubsequentAlloc>::operator[](const SIZE_T Index) const
{
	assert(Index<m_ItemCount);
	assert(m_Items);

	return m_Items[Index];
}

template <typename T, UINT FirstAlloc, UINT SubsequentAlloc>
T& LFDynArray<T, FirstAlloc, SubsequentAlloc>::operator[](const SIZE_T Index)
{
	assert(Index<m_ItemCount);
	assert(m_Items);

	return m_Items[Index];
}

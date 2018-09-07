
#pragma once
#include <assert.h>


template <typename T, SIZE_T FirstAlloc, SIZE_T SubsequentAlloc>
class LFDynArray
{
public:
	LFDynArray();
	~LFDynArray();

	BOOL AddItem(const T& Item);
	BOOL InsertEmpty(UINT Pos, UINT Count=1, BOOL ZeroOut=TRUE);
	void DeleteItems(UINT Pos, UINT Count=1);

	const T& operator[](const SIZE_T Index) const;
	T& operator[](const SIZE_T Index);

	UINT m_ItemCount;

protected:
	UINT m_Allocated;
	T* m_Items;
};


template <typename T, SIZE_T FirstAlloc, SIZE_T SubsequentAlloc>
LFDynArray<T, FirstAlloc, SubsequentAlloc>::LFDynArray()
{
	m_Items = NULL;
	m_ItemCount = m_Allocated = 0;
}

template <typename T, SIZE_T FirstAlloc, SIZE_T SubsequentAlloc>
LFDynArray<T, FirstAlloc, SubsequentAlloc>::~LFDynArray()
{
	free(m_Items);
}

template <typename T, SIZE_T FirstAlloc, SIZE_T SubsequentAlloc>
BOOL LFDynArray<T, FirstAlloc, SubsequentAlloc>::AddItem(const T& Item)
{
	if (m_ItemCount==m_Allocated)
	{
		if (!m_Items)
		{
			m_Items = (T*)malloc((m_Allocated=FirstAlloc)*sizeof(T));
		}
		else
		{
			m_Items = (T*)realloc(m_Items, (m_Allocated+=SubsequentAlloc)*sizeof(T));
		}

		if (!m_Items)
			return FALSE;
	}

	m_Items[m_ItemCount++] = Item;

	return TRUE;
}

template <typename T, SIZE_T FirstAlloc, SIZE_T SubsequentAlloc>
BOOL LFDynArray<T, FirstAlloc, SubsequentAlloc>::InsertEmpty(UINT Pos, UINT Count=1, BOOL ZeroOut=TRUE)
{
	if (!Count)
		return TRUE;

	if (Pos>m_ItemCount+1)
		return FALSE;

	if (m_ItemCount<m_Allocated+Count)
	{
		if (!m_Items)
		{
			m_Items = (T*)malloc((m_Allocated=max(FirstAlloc, Count))*sizeof(T));
		}
		else
		{
			m_Items = (T*)realloc(m_Items, (m_Allocated+=SubsequentAlloc)*sizeof(T));
		}

		if (!m_Items)
			return FALSE;
	}

	for (INT a=((INT)(m_ItemCount))-1; a>=(INT)Pos; a--)
		m_Items[a+Count] = m_Items[a];

	if (ZeroOut)
		ZeroMemory(&m_Items[Pos], Count*sizeof(T));

	m_ItemCount += Count;

	return TRUE;
}

template <typename T, SIZE_T FirstAlloc, SIZE_T SubsequentAlloc>
void LFDynArray<T, FirstAlloc, SubsequentAlloc>::DeleteItems(UINT Pos, UINT Count=1)
{
	if (!Count)
		return;

	for (UINT a=Pos; a<m_ItemCount-Count; a++)
		m_Items[a] = m_Items[a+Count];

	m_ItemCount -= Count;
}

template <typename T, SIZE_T FirstAlloc, SIZE_T SubsequentAlloc>
const T& LFDynArray<T, FirstAlloc, SubsequentAlloc>::operator[](const SIZE_T Index) const
{
	assert(Index<m_ItemCount);
	assert(m_Items);

	return m_Items[Index];
}

template <typename T, SIZE_T FirstAlloc, SIZE_T SubsequentAlloc>
T& LFDynArray<T, FirstAlloc, SubsequentAlloc>::operator[](const SIZE_T Index)
{
	assert(Index<m_ItemCount);
	assert(m_Items);

	return m_Items[Index];
}

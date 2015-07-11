
#pragma once
#include <assert.h>

#define DYN_FIRSTALLOC          1024
#define DYN_SUBSEQUENTALLOC     1024


template <typename T>
class LFDynArray
{
public:
	LFDynArray();
	~LFDynArray();

	BOOL AddItem(T i);
	BOOL InsertEmpty(UINT Pos, UINT Count=1, BOOL ZeroOut=TRUE);
	BOOL InsertItems(UINT Pos, T* pData, UINT Count=1);
	void DeleteItems(UINT Pos, UINT Count=1);

	T* m_Items;
	UINT m_ItemCount;
	UINT m_LastError;

protected:
	UINT m_Allocated;
};


#define INITLFDYNARRAY() \
	if (!m_Items) \
	{ \
		m_Items = (T*)malloc(DYN_FIRSTALLOC*sizeof(T)); \
		if (!m_Items) \
			return FALSE; \
		m_Allocated = DYN_FIRSTALLOC; \
	}


template <typename T>
LFDynArray<T>::LFDynArray()
{
	m_Items = NULL;
	m_ItemCount = m_Allocated = 0;
	m_LastError = LFOk;
}

template <typename T>
LFDynArray<T>::~LFDynArray()
{
	if (m_Items)
		free(m_Items);
}

template <typename T>
BOOL LFDynArray<T>::AddItem(T i)
{
	INITLFDYNARRAY()

	if (m_ItemCount==m_Allocated)
	{
		m_Items = (T*)realloc(m_Items, (m_Allocated+DYN_SUBSEQUENTALLOC)*sizeof(T));
		if (!m_Items)
			return FALSE;

		m_Allocated += DYN_SUBSEQUENTALLOC;
	}

	m_Items[m_ItemCount++] = i;

	return TRUE;
}

template <typename T>
BOOL LFDynArray<T>::InsertEmpty(UINT Pos, UINT Count=1, BOOL ZeroOut=TRUE)
{
	if (!Count)
		return TRUE;

	if (Pos>m_ItemCount+1)
		return FALSE;

	INITLFDYNARRAY()

	if (m_ItemCount<m_Allocated+Count)
	{
		const UINT Add = (Count+DYN_SUBSEQUENTALLOC-1) & ~(DYN_SUBSEQUENTALLOC-1);
		m_Items = (T*)realloc(m_Items, (m_Allocated+Add)*sizeof(T));
		if (!m_Items)
			return FALSE;

		m_Allocated += Add;
	}

	for (INT a=((INT)(m_ItemCount))-1; a>=(INT)Pos; a--)
		m_Items[a+Count] = m_Items[a];

	if (ZeroOut)
		ZeroMemory(&m_Items[Pos], Count*sizeof(T));

	m_ItemCount += Count;

	return TRUE;
}

template <typename T>
BOOL LFDynArray<T>::InsertItems(UINT Pos, T* pData, UINT Count=1)
{
	assert(pData);

	if (!InsertEmpty(Pos, Count, FALSE))
		return FALSE;

	memcpy_s(m_Items[Pos], sizeof(T)*Count, pData, sizeof(T)*Count);

	return TRUE;
}

template <typename T>
void LFDynArray<T>::DeleteItems(UINT Pos, UINT Count=1)
{
	if (!Count)
		return;

	for (UINT a=Pos; a<m_ItemCount-Count; a++)
		m_Items[a] = m_Items[a+Count];

	m_ItemCount -= Count;
}

#pragma once

#define Dyn_FirstAlloc          1024
#define Dyn_SubsequentAlloc     1024

#define Dyn_MemoryAlignment     8


template <typename T>
class DynArray
{
public:
	DynArray();
	~DynArray();

	bool AddItem(T i);

	T* m_Items;
	unsigned int m_ItemCount;
	unsigned int m_LastError;

protected:
	unsigned int m_Allocated;
};


template <typename T>
DynArray<T>::DynArray()
{
	m_Items = NULL;
	m_ItemCount = m_Allocated = 0;
	m_LastError = LFOk;
}

template <typename T>
DynArray<T>::~DynArray()
{
	if (m_Items)
		_aligned_free(m_Items);
}

template <typename T>
bool DynArray<T>::AddItem(T i)
{
	if (!m_Items)
	{
		m_Items = (T*)_aligned_malloc(Dyn_FirstAlloc*sizeof(T), Dyn_MemoryAlignment);
		if (!m_Items)
		{
			m_LastError = LFMemoryError;
			return false;
		}

		m_Allocated = Dyn_FirstAlloc;
	}

	if (m_ItemCount==m_Allocated)
	{
		m_Items = (T*)_aligned_realloc(m_Items, (m_Allocated+Dyn_SubsequentAlloc)*sizeof(T), Dyn_MemoryAlignment);
		if (!m_Items)
		{
			m_LastError = LFMemoryError;
			return false;
		}

		m_Allocated += Dyn_SubsequentAlloc;
	}

	m_Items[m_ItemCount++] = i;
	return true;
}

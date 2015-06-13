
#pragma once
#include <assert.h>

#define DYN_FIRSTALLOC          1024
#define DYN_SUBSEQUENTALLOC     1024

#define DYN_MEMORYALIGNMENT     8


template <typename T>
class LFDynArray
{
public:
	LFDynArray();
	~LFDynArray();

	bool AddItem(T i);
	bool InsertEmpty(unsigned int Pos, unsigned int Count=1, bool ZeroOut=true);
	bool InsertItems(unsigned int Pos, T* pData, unsigned int Count=1);
	void DeleteItems(unsigned int Pos, unsigned int Count=1);

	T* m_Items;
	unsigned int m_ItemCount;
	unsigned int m_LastError;

protected:
	unsigned int m_Allocated;
};


#define INITLFDynARRAY() \
	if (!m_Items) \
	{ \
		m_Items = (T*)_aligned_malloc(DYN_FIRSTALLOC*sizeof(T), DYN_MEMORYALIGNMENT); \
		if (!m_Items) \
			return false; \
		m_Allocated = DYN_FIRSTALLOC; \
	} \


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
		_aligned_free(m_Items);
}

template <typename T>
bool LFDynArray<T>::AddItem(T i)
{
	INITLFDynARRAY()

	if (m_ItemCount==m_Allocated)
	{
		m_Items = (T*)_aligned_realloc(m_Items, (m_Allocated+DYN_SUBSEQUENTALLOC)*sizeof(T), DYN_MEMORYALIGNMENT);
		if (!m_Items)
			return false;

		m_Allocated += DYN_SUBSEQUENTALLOC;
	}

	m_Items[m_ItemCount++] = i;
	return true;
}

template <typename T>
bool LFDynArray<T>::InsertEmpty(unsigned int Pos, unsigned int Count=1, bool ZeroOut=true)
{
	if (!Count)
		return true;
	if (Pos>m_ItemCount+1)
		return false;

	INITLFDynARRAY()

	if (m_ItemCount<m_Allocated+Count)
	{
		const unsigned int Add = (Count+DYN_SUBSEQUENTALLOC-1) & ~(DYN_SUBSEQUENTALLOC-1);
		m_Items = static_cast<T*>(_aligned_realloc(m_Items, (m_Allocated+Add)*sizeof(T), DYN_MEMORYALIGNMENT));
		if (!m_Items)
			return false;

		m_Allocated += Add;
	}

	for (int a=((int)(m_ItemCount))-1; a>=(int)Pos; a--)
		m_Items[a+Count] = m_Items[a];

	if (ZeroOut)
		ZeroMemory(&m_Items[Pos], Count*sizeof(T));

	m_ItemCount += Count;
	return true;
}

template <typename T>
bool LFDynArray<T>::InsertItems(unsigned int Pos, T* pData, unsigned int Count=1)
{
	assert(pData);

	if (!InsertEmpty(Pos, Count, false))
		return false;

	memcpy_s(m_Items[Pos], sizeof(T)*Count, pData, sizeof(T)*Count);
	return true;
}

template <typename T>
void LFDynArray<T>::DeleteItems(unsigned int Pos, unsigned int Count=1)
{
	if (!Count)
		return;

	for (unsigned int a=Pos; a<m_ItemCount-Count; a++)
		m_Items[a] = m_Items[a+Count];

	m_ItemCount -= Count;
}


#include "stdafx.h"
#include "LFBitArray.h"
#include <assert.h>
#include <malloc.h>


LFBitArray::LFBitArray(unsigned int size)
{
	m_nSize = size;
	m_nTAllocated = (m_nSize+TSize-1)/TSize;

	m_pData = (T*)malloc(m_nTAllocated*sizeof(T));
	assert(m_pData);

	Clear();
}

LFBitArray::~LFBitArray(void)
{
	free(m_pData);
}

void LFBitArray::Clear()
{
	memset(m_pData, 0, m_nTAllocated*sizeof(T));
}

void LFBitArray::Set()
{
	memset(m_pData, -1, m_nTAllocated*sizeof(T));
}

bool LFBitArray::IsSet(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	return (m_pData[index/TSize] & ((T)1 << (index % TSize))) ? true : false;
}

bool LFBitArray::ContainsOneOf(LFBitArray* B)
{
	assert(m_nSize==B->m_nSize);
	assert(m_pData);

	T* ptr=m_pData;
	for (unsigned int a=0; a<m_nTAllocated; a++)
		if (ptr[a] & B->m_pData[a])
			return true;

	return false;
}

bool LFBitArray::ContainsAll(LFBitArray* B)
{
	assert(m_nSize==B->m_nSize);
	assert(m_pData);

	for (unsigned int a=0; a<m_nTAllocated; a++)
		if ((B->m_pData[a] & m_pData[a]) != B->m_pData[a])
			return false;

	return true;
}

LFBitArray& LFBitArray::operator~()
{
	assert(m_pData);

	LFBitArray* temp = CreateCopy(*this);
	for (unsigned int a=0; a<m_nTAllocated; a++)
		temp->m_pData[a] = ~m_pData[a];

	return *temp;
}

LFBitArray& LFBitArray::operator+=(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	m_pData[index/TSize] |= (T)1 << (index % TSize);

	return *this;
}

LFBitArray& LFBitArray::operator-=(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	m_pData[index/TSize] &= ~((T)1 << (index % TSize));

	return *this;
}

LFBitArray& LFBitArray::operator^=(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	m_pData[index/TSize] ^= (T)1 << (index % TSize);

	return *this;
}

LFBitArray& LFBitArray::operator&=(const LFBitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	for (unsigned int a=0; a<m_nTAllocated; a++)
		m_pData[a] &= B.m_pData[a];

	return *this;
}

LFBitArray& LFBitArray::operator|=(const LFBitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	for (unsigned int a=0; a<m_nTAllocated; a++)
		m_pData[a] |= B.m_pData[0];

	return *this;
}

LFBitArray& LFBitArray::operator^=(const LFBitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	for (unsigned int a=0;a <m_nTAllocated; a++)
		m_pData[a] ^= B.m_pData[a];

	return *this;
}

LFBitArray& LFBitArray::operator+(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	LFBitArray* temp = CreateCopy(*this);
	temp->m_pData[index/TSize] |= (T)1 << (index % TSize);

	return *temp;
}

LFBitArray& LFBitArray::operator-(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	LFBitArray* temp = CreateCopy(*this);
	temp->m_pData[index/TSize] &= ~(T)1 << (index % TSize);

	return *temp;
}

LFBitArray& LFBitArray::operator^(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	LFBitArray* temp = CreateCopy(*this);
	temp->m_pData[index/TSize] ^= (T)1 << (index % TSize);

	return *temp;
}

LFBitArray& LFBitArray::operator&(const LFBitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	LFBitArray* temp = CreateCopy(*this);
	for (unsigned int a=0; a<m_nTAllocated; a++)
		temp->m_pData[a] &= B.m_pData[a];

	return *temp;
}

LFBitArray& LFBitArray::operator|(const LFBitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	LFBitArray* temp = CreateCopy(*this);
	for (unsigned int a=0; a<m_nTAllocated; a++)
		temp->m_pData[a] |= B.m_pData[a];

	return *temp;
}

LFBitArray& LFBitArray::operator^(const LFBitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	LFBitArray* temp = CreateCopy(*this);
	for (unsigned int a=0; a<m_nTAllocated; a++)
		temp->m_pData[a] ^= B.m_pData[a];

	return *temp;
}

LFBitArray* LFBitArray::CreateCopy(const LFBitArray& B)
{
	LFBitArray* temp = new LFBitArray(B.m_nSize);
	memcpy(temp->m_pData, B.m_pData, temp->m_nTAllocated*sizeof(T));
	return temp;
}

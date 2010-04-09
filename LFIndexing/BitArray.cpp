
#include "stdafx.h"
#include "LFIndexing.h"
#include <malloc.h>


BitArray::BitArray(unsigned int size)
{
	m_nSize = size;
	m_nTAllocated = (m_nSize+TSize-1)/TSize;
	m_pData = static_cast<T*>(malloc(m_nTAllocated*sizeof(T)));
	assert(m_pData);
	clear();
}

BitArray::~BitArray(void)
{
	free(m_pData);
}

void BitArray::clear()
{
	memset(m_pData, 0, m_nTAllocated*sizeof(T));
}

void BitArray::set()
{
	memset(m_pData, -1, m_nTAllocated*sizeof(T));
}

bool BitArray::IsSet(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	return (m_pData[index/TSize] & (static_cast<T>(1) << (index % TSize))) ? true : false;
}

bool BitArray::ContainsOneOf(BitArray* B)
{
	assert(m_nSize==B->m_nSize);
	assert(m_pData);

	T* ptr=m_pData;
	for (unsigned int a=0; a<m_nTAllocated; a++)
		if (ptr[a] & B->m_pData[a]) return true;

	return false;
}

bool BitArray::ContainsAll(BitArray* B)
{
	assert(m_nSize==B->m_nSize);
	assert(m_pData);

	for (unsigned int a=0; a<m_nTAllocated; a++)
		if ((B->m_pData[a] & m_pData[a]) != B->m_pData[a]) return false;

	return true;
}

BitArray& BitArray::operator~()
{
	assert(m_pData);

	BitArray* temp = CreateCopy(*this);
	for (unsigned int a=0; a<m_nTAllocated; a++)
		temp->m_pData[a] = ~m_pData[a];

	return *temp;
}

BitArray& BitArray::operator+=(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	m_pData[index/TSize] |= static_cast<T>(1) << (index % TSize);

	return *this;
}

BitArray& BitArray::operator-=(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	m_pData[index/TSize] &= ~(static_cast<T>(1) << (index % TSize));

	return *this;
}

BitArray& BitArray::operator^=(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	m_pData[index/TSize] ^= static_cast<T>(1) << (index % TSize);

	return *this;
}

BitArray& BitArray::operator&=(const BitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	for (unsigned int a=0; a<m_nTAllocated; a++)
		m_pData[a] &= B.m_pData[a];

	return *this;
}

BitArray& BitArray::operator|=(const BitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	for (unsigned int a=0; a<m_nTAllocated; a++)
		m_pData[a] |= B.m_pData[0];

	return *this;
}

BitArray& BitArray::operator^=(const BitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	for (unsigned int a=0;a <m_nTAllocated; a++)
		m_pData[a] ^= B.m_pData[a];

	return *this;
}

BitArray& BitArray::operator+(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	BitArray* temp = CreateCopy(*this);
	temp->m_pData[index/TSize] |= static_cast<T>(1) << (index % TSize);

	return *temp;
}

BitArray& BitArray::operator-(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	BitArray* temp = CreateCopy(*this);
	temp->m_pData[index/TSize] &= ~(static_cast<T>(1) << (index % TSize));

	return *temp;
}

BitArray& BitArray::operator^(unsigned int index)
{
	assert(index < m_nSize);
	assert(m_pData);

	BitArray* temp = CreateCopy(*this);
	temp->m_pData[index/TSize] ^= static_cast<T>(1) << (index % TSize);

	return *temp;
}

BitArray& BitArray::operator&(const BitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	BitArray* temp = CreateCopy(*this);
	for (unsigned int a=0; a<m_nTAllocated; a++)
		temp->m_pData[a] &= B.m_pData[a];

	return *temp;
}

BitArray& BitArray::operator|(const BitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	BitArray* temp = CreateCopy(*this);
	for (unsigned int a=0; a<m_nTAllocated; a++)
		temp->m_pData[a] |= B.m_pData[a];

	return *temp;
}

BitArray& BitArray::operator^(const BitArray& B)
{
	assert(m_nSize==B.m_nSize);
	assert(m_pData);

	BitArray* temp = CreateCopy(*this);
	for (unsigned int a=0; a<m_nTAllocated; a++)
		temp->m_pData[a] ^= B.m_pData[a];

	return *temp;
}

BitArray* BitArray::CreateCopy(const BitArray& B)
{
	BitArray* temp = new BitArray(B.m_nSize);
	memcpy(temp->m_pData, B.m_pData, temp->m_nTAllocated*sizeof(T));
	return temp;
}

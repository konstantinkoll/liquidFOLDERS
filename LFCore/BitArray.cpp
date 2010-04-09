#include "stdafx.h"
#include "BFCore.h"
#include <malloc.h>

BitArray::BitArray(unsigned int size)
{
	m_nSize = size;
	m_nTAllocated = (m_nSize+TSize-1)/TSize;
	m_pData = static_cast<T*>(malloc(m_nTAllocated*sizeof(T)));
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

bool BitArray::IsSet(unsigned int index)
{
	return (m_pData[index/TSize] & (static_cast<T>(1) << (index % TSize)));
}

bool BitArray::ContainsOneOf(BitArray* B)
{
	T* ptr=m_pData;
	for (unsigned int a=0;a<m_nTAllocated;a++)
		if (ptr[a] & B->m_pData[a]) return true;

	return false;
}

bool BitArray::ContainsAll(BitArray* B)
{
	T* ptr=m_pData;
	for (unsigned int a=0;a<m_nTAllocated;a++)
		if ((B->m_pData[a] & ptr[a]) != B->m_pData[a]) return false;

	return true;
}

BitArray& BitArray::operator!()
{
	return *this;
}

BitArray& BitArray::operator+=(unsigned int index)
{	
	m_pData[index/TSize] |= static_cast<T>(1) << (index % TSize);

	return *this;
}

BitArray& BitArray::operator-=(unsigned int index)
{
	m_pData[index/TSize] &= ~(static_cast<T>(1) << (index % TSize));

	return *this;
}

BitArray& BitArray::operator^=(unsigned int index)
{
	m_pData[index/TSize] ^= static_cast<T>(1) << (index % TSize);

	return *this;
}

BitArray& BitArray::operator&=(const BitArray& B)
{
	T* ptr=m_pData;
	for (unsigned int a=0;a<m_nTAllocated;a++)
		ptr[a] &= B.m_pData[a];		

	return *this;
}

BitArray& BitArray::operator|=(const BitArray& B)
{
	T* ptr=m_pData;
	for (unsigned int a=0;a<m_nTAllocated;a++)
		ptr[a] |= B.m_pData[0];

	return *this;
}

BitArray& BitArray::operator^=(const BitArray& B)
{
	T* ptr=m_pData;
	for (unsigned int a=0;a<m_nTAllocated;a++)
		ptr[a] ^= B.m_pData[a];

	return *this;
}

BitArray& BitArray::operator+(unsigned int index)
{
	BitArray* temp=CreateCopy(*this);
	temp->m_pData[index/TSize] |= static_cast<T>(1) << (index % TSize);

	return *temp;
}

BitArray& BitArray::operator-(unsigned int index)
{
	BitArray* temp=CreateCopy(*this);
	temp->m_pData[index/TSize] &= ~(static_cast<T>(1) << (index % TSize));	

	return *temp;
}

BitArray& BitArray::operator^(unsigned int index)
{
	BitArray* temp=CreateCopy(*this);
	temp->m_pData[index/TSize] ^= static_cast<T>(1) << (index % TSize);		

	return *temp;
}

BitArray& BitArray::operator&(const BitArray& B)
{
	BitArray* temp=CreateCopy(*this);
	T* ptr=temp->m_pData;
	for (unsigned int a=0;a<m_nTAllocated;a++)
		ptr[a] &= B.m_pData[a];		

	return *temp;
}

BitArray& BitArray::operator|(const BitArray& B)
{
	BitArray* temp=CreateCopy(*this);
	T* ptr=temp->m_pData;
	for (unsigned int a=0;a<m_nTAllocated;a++)
		ptr[a] |= B.m_pData[a];		

	return *temp;
}

BitArray& BitArray::operator^(const BitArray& B)
{
	BitArray* temp=CreateCopy(*this);
	T* ptr=temp->m_pData;
	for (unsigned int a=0;a<m_nTAllocated;a++)
		ptr[a] ^= B.m_pData[a];		

	return *temp;
}

BitArray* BitArray::CreateCopy(const BitArray& B)
{
	BitArray* temp=new BitArray(B.m_nSize);
	memcpy(temp->m_pData, B.m_pData, temp->m_nTAllocated*sizeof(T));
	return temp;
}

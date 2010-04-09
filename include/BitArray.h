
#pragma once


// BitArray
// Klasse, die eine (fast) beliebig große Menge von bool-Werten abspeichert und über
// Operator-Überladungen zugänglich macht. Die Größe eines BitArray muss vorher festgelegt
// werden und kann nicht mehr geändert werden. Nur gleich große BitArrays können verrechnet
// werden.
//

class LFIndexing_API BitArray
{
	typedef long T;
	#define TSize (sizeof(T)*8)

public:
	unsigned int m_nSize;
	unsigned int m_nTAllocated;
	T* m_pData;

	BitArray(unsigned int size);
	~BitArray(void);

	void clear();
	void set();
	bool IsSet(unsigned int index);
	bool ContainsOneOf(BitArray* B);
	bool ContainsAll(BitArray* B);

	BitArray& operator~();						// Alle Bits toggeln
	
	BitArray& operator+=(unsigned int index);	// Bit setzen
	BitArray& operator-=(unsigned int index);	// Bit löschen
	BitArray& operator^=(unsigned int index);	// Bit toggeln
	BitArray& operator&=(const BitArray& B);	// Schnittmenge bilden
	BitArray& operator|=(const BitArray& B);	// Vereinigungsmenge bilden
	BitArray& operator^=(const BitArray& B);	// Bit aus this toggeln, wenn Bit in B gesetzt ist

	BitArray& operator+(unsigned int index);	// Bit setzen
	BitArray& operator-(unsigned int index);	// Bit löschen
	BitArray& operator^(unsigned int index);	// Bit toggeln
	BitArray& operator&(const BitArray& B);		// Schnittmenge bilden
	BitArray& operator|(const BitArray& B);		// Vereinigungsmenge bilden
	BitArray& operator^(const BitArray& B);		// Bit aus this toggeln, wenn Bit in B gesetzt ist

private:
	BitArray* CreateCopy(const BitArray& B);
};

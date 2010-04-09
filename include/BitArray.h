
#pragma once


// BitArray
// Klasse, die eine (fast) beliebig gro�e Menge von bool-Werten abspeichert und �ber
// Operator-�berladungen zug�nglich macht. Die Gr��e eines BitArray muss vorher festgelegt
// werden und kann nicht mehr ge�ndert werden. Nur gleich gro�e BitArrays k�nnen verrechnet
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
	BitArray& operator-=(unsigned int index);	// Bit l�schen
	BitArray& operator^=(unsigned int index);	// Bit toggeln
	BitArray& operator&=(const BitArray& B);	// Schnittmenge bilden
	BitArray& operator|=(const BitArray& B);	// Vereinigungsmenge bilden
	BitArray& operator^=(const BitArray& B);	// Bit aus this toggeln, wenn Bit in B gesetzt ist

	BitArray& operator+(unsigned int index);	// Bit setzen
	BitArray& operator-(unsigned int index);	// Bit l�schen
	BitArray& operator^(unsigned int index);	// Bit toggeln
	BitArray& operator&(const BitArray& B);		// Schnittmenge bilden
	BitArray& operator|(const BitArray& B);		// Vereinigungsmenge bilden
	BitArray& operator^(const BitArray& B);		// Bit aus this toggeln, wenn Bit in B gesetzt ist

private:
	BitArray* CreateCopy(const BitArray& B);
};

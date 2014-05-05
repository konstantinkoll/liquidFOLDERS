
#pragma once

#ifdef LFCore_EXPORTS
#define LFCore_API __declspec(dllexport)
#else
#define LFCore_API __declspec(dllimport)
#endif


// LFBitArray
// Klasse, die eine (fast) beliebig große Menge von bool-Werten abspeichert und über
// Operator-Überladungen zugänglich macht. Die Größe eines BitArray muss vorher festgelegt
// werden und kann nicht mehr geändert werden. Nur gleich große BitArrays können verrechnet
// werden.
//

class LFCore_API LFBitArray
{
	typedef long T;
	#define TSize (sizeof(T)*8)

public:
	LFBitArray(unsigned int size);
	~LFBitArray(void);

	void Clear();
	void Set();
	bool IsSet(unsigned int index);
	bool ContainsOneOf(LFBitArray* B);
	bool ContainsAll(LFBitArray* B);

	LFBitArray& operator~();						// Alle Bits toggeln

	LFBitArray& operator+=(unsigned int index);		// Bit setzen
	LFBitArray& operator-=(unsigned int index);		// Bit löschen
	LFBitArray& operator^=(unsigned int index);		// Bit toggeln
	LFBitArray& operator&=(const LFBitArray& B);	// Schnittmenge bilden
	LFBitArray& operator|=(const LFBitArray& B);	// Vereinigungsmenge bilden
	LFBitArray& operator^=(const LFBitArray& B);	// Bit aus this toggeln, wenn Bit in B gesetzt ist

	LFBitArray& operator+(unsigned int index);		// Bit setzen
	LFBitArray& operator-(unsigned int index);		// Bit löschen
	LFBitArray& operator^(unsigned int index);		// Bit toggeln
	LFBitArray& operator&(const LFBitArray& B);		// Schnittmenge bilden
	LFBitArray& operator|(const LFBitArray& B);		// Vereinigungsmenge bilden
	LFBitArray& operator^(const LFBitArray& B);		// Bit aus this toggeln, wenn Bit in B gesetzt ist

protected:
	unsigned int m_nSize;
	unsigned int m_nTAllocated;
	T* m_pData;

private:
	LFBitArray* CreateCopy(const LFBitArray& B);
};


#pragma once

#ifdef LFCore_EXPORTS
#define LFCore_API __declspec(dllexport)
#else
#define LFCore_API __declspec(dllimport)
#endif


// LFBitArray
// Klasse, die eine (fast) beliebig gro�e Menge von bool-Werten abspeichert und �ber
// Operator-�berladungen zug�nglich macht. Die Gr��e eines BitArray muss vorher festgelegt
// werden und kann nicht mehr ge�ndert werden. Nur gleich gro�e BitArrays k�nnen verrechnet
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
	LFBitArray& operator-=(unsigned int index);		// Bit l�schen
	LFBitArray& operator^=(unsigned int index);		// Bit toggeln
	LFBitArray& operator&=(const LFBitArray& B);	// Schnittmenge bilden
	LFBitArray& operator|=(const LFBitArray& B);	// Vereinigungsmenge bilden
	LFBitArray& operator^=(const LFBitArray& B);	// Bit aus this toggeln, wenn Bit in B gesetzt ist

	LFBitArray& operator+(unsigned int index);		// Bit setzen
	LFBitArray& operator-(unsigned int index);		// Bit l�schen
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

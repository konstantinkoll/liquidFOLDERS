
#pragma once
#include "LFCore.h"
#include "CHeapfile.h"
#include "Progress.h"
#include "TableIndexes.h"


// CIndex
//

typedef class _HORCRUXFILE
{
public:
	operator LFCoreAttributes&() const { return *p_CoreAttributes; }
	operator LPCCOREATTRIBUTES() const { return p_CoreAttributes; }
	operator LPCVOID() const { return p_StoreData; }
	operator LPCWSTR() const { return (LPCWSTR)p_StoreData; }
	operator LPWSTR() const { return (LPWSTR)p_StoreData; }
	operator ITEMSTATE() const { return p_CoreAttributes->State; }

	inline const _HORCRUXFILE(LPCCOREATTRIBUTES pCoreAttributes, LPVOID pStoreData)
	{
		assert(pCoreAttributes);

		p_CoreAttributes = pCoreAttributes;
		p_StoreData = pStoreData;
	}

	inline const _HORCRUXFILE(LFItemDescriptor* pItemDescriptor)
	{
		assert(pItemDescriptor);

		p_CoreAttributes = &pItemDescriptor->CoreAttributes;
		p_StoreData = &pItemDescriptor->StoreData;
	}

protected:
	LPCOREATTRIBUTES p_CoreAttributes;
	LPVOID p_StoreData;
} HORCRUXFILE, *LPHORCRUXFILE;


#define IndexMaintenanceSteps     (IDXTABLECOUNT*2+1)

class CStore;

class CIndex sealed
{
public:
	CIndex(CStore* pStore, BOOL IsMainIndex, BOOL WriteAccess, UINT StoreDataSize);
	~CIndex();

	// Index management
	BOOL Initialize();
	UINT MaintenanceAndStatistics(BOOL Scheduled, BOOL& Repaired, LFProgress* pProgress=NULL);

	// Operations on index only
	UINT Add(LFItemDescriptor* pItemDescriptor);
	void Query(LFFilter* pFilter, LFSearchResult* pSearchResult, BOOL UpdateStatistics=FALSE);
	UINT UpdateStatistics();
	void AddToSearchResult(LFTransactionList* pTransactionList, LFSearchResult* pSearchResult);
	void ResolveLocations(LFTransactionList* pTransactionList);
	void SendTo(LFTransactionList* pTransactionList, const STOREID& StoreID, LFProgress* pProgress=NULL);
	BOOL ExistingFileID(const FILEID& FileID);
	void SetUserContext(LFTransactionList* pTransactionList, ITEMCONTEXT UserContextID);
	void SetItemState(LFTransactionList* pTransactionList, const FILETIME& TransactionTime, ITEMSTATE State);
	UINT UpdateItemState(LFItemDescriptor* pItemDescriptor, const WIN32_FIND_DATA& FindData, BOOL Exists, BOOL RemoveNew=FALSE);
	void Compress(LFTransactionList* pTransactionList, LFProgress* pProgress=NULL);

	// Operations with callbacks to CStore object
	void Update(LFTransactionList* pTransactionList, const LFVariantData* pVariantData1, const LFVariantData* pVariantData2=NULL, const LFVariantData* pVariantData3=NULL, BOOL MakeTask=FALSE);
	UINT SynchronizeMatch();
	UINT SynchronizeCommit(LFProgress* pProgress=NULL);
	void Delete(LFTransactionList* pTransactionList, LFProgress* pProgress=NULL);

protected:
	void SetError(LFTransactionList* pTransactionList, UINT Result, LFProgress* pProgress=NULL) const;
	void SetError(LFTransactionList* pTransactionList, UINT Index, UINT Result, LFProgress* pProgress=NULL) const;
	BOOL LoadTable(UINT TableID, BOOL Initialize=FALSE, UINT* pResult=NULL);
	UINT CompactTable(UINT TableID) const;
	UINT GetFileLocation(const HORCRUXFILE& File, LPWSTR pPath, SIZE_T cCount, LFItemDescriptor* pItemDescriptor);

	CStore* p_Store;
	LFStoreDescriptor* p_StoreDescriptor;
	UINT m_StoreFlags;
	UINT m_StoreDataSize;
	BOOL m_IsMainIndex;
	BOOL m_WriteAccess;
	CHeapfile* m_pTable[IDXTABLECOUNT];

private:
	BOOL ProgressMinorNext(LFProgress* pProgress) const;
	BOOL InspectForUpdate(const LFVariantData* pVData, BOOL& IncludeSlaves, BOOL& DoRename);

	WCHAR m_IdxPath[MAX_PATH];
};

inline void CIndex::SetError(LFTransactionList* pTransactionList, UINT Result, LFProgress* pProgress) const
{
	assert(pTransactionList);

	pTransactionList->SetError(p_StoreDescriptor->StoreID, Result, m_IsMainIndex ? pProgress : NULL);
}

inline void CIndex::SetError(LFTransactionList* pTransactionList, UINT Index, UINT Result, LFProgress* pProgress) const
{
	assert(pTransactionList);

	pTransactionList->SetError(Index, Result, m_IsMainIndex ? pProgress : NULL);
}

inline BOOL CIndex::ProgressMinorNext(LFProgress* pProgress) const
{
	return m_IsMainIndex ? ::ProgressMinorNext(pProgress) : FALSE;
}

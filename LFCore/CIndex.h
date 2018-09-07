
#pragma once
#include "LFCore.h"
#include "CHeapfile.h"
#include "Progress.h"
#include "TableIndexes.h"


// CIndex
//

#define IndexMaintenanceSteps     (IDXTABLECOUNT*2+1)

typedef class _REVENANTFILE
{
public:
	operator LPCCOREATTRIBUTES() const { return p_CoreAttributes; }
	operator LPCVOID() const { return p_StoreData; }
	operator BYTE() const { return p_CoreAttributes->Flags; }

	inline const _REVENANTFILE(LPCCOREATTRIBUTES pCoreAttributes, LPCVOID pStoreData)
	{
		assert(pCoreAttributes);

		p_CoreAttributes = pCoreAttributes;
		p_StoreData = pStoreData;
	}

	inline const _REVENANTFILE(LFItemDescriptor* pItemDescriptor)
	{
		assert(pItemDescriptor);

		p_CoreAttributes = &pItemDescriptor->CoreAttributes;
		p_StoreData = &pItemDescriptor->StoreData;
	}

	friend _REVENANTFILE operator+=(const _REVENANTFILE& File, const BYTE Flag) { File.p_CoreAttributes->Flags |= Flag; return File; }
	friend _REVENANTFILE operator-=(const _REVENANTFILE& File, const BYTE Flag) { File.p_CoreAttributes->Flags &= ~Flag; return File; }
	friend _REVENANTFILE operator|=(const _REVENANTFILE& File, const UINT Flags) { File.p_CoreAttributes->Flags |= Flags; return File; }
	friend _REVENANTFILE operator&=(const _REVENANTFILE& File, const UINT Flags) { File.p_CoreAttributes->Flags &= Flags; return File; }

protected:
	LPCOREATTRIBUTES p_CoreAttributes;
	LPCVOID p_StoreData;
} REVENANTFILE, *LPREVENANTFILE;

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
	void UpdateUserContext(LFTransactionList* pTransactionList, BYTE UserContextID);
	UINT GetFileLocation(LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount, BOOL RemoveNew);
	void UpdateItemState(LFTransactionList* pTransactionList, const FILETIME& TransactionTime, BYTE Flags);

	// Operations with callbacks to CStore object
	void Update(LFTransactionList* pTransactionList, const LFVariantData* pVariantData1, const LFVariantData* pVariantData2=NULL, const LFVariantData* pVariantData3=NULL, BOOL MakeTask=FALSE);
	UINT Synchronize(LFProgress* pProgress=NULL);
	void Delete(LFTransactionList* pTransactionList, LFProgress* pProgress=NULL);

protected:
	void SetError(LFTransactionList* pTransactionList, UINT Result, LFProgress* pProgress=NULL) const;
	void SetError(LFTransactionList* pTransactionList, UINT Index, UINT Result, LFProgress* pProgress=NULL) const;
	BOOL LoadTable(UINT TableID, BOOL Initialize=FALSE, UINT* pResult=NULL);
	UINT CompactTable(UINT TableID) const;
	UINT GetFileLocation(const REVENANTFILE& File, LPWSTR pPath, SIZE_T cCount, LFItemDescriptor* pItemDescriptor=NULL, BOOL RemoveNew=FALSE, WIN32_FIND_DATA* pFindData=NULL);

	CStore* p_Store;
	LFStoreDescriptor* p_StoreDescriptor;
	UINT m_StoreTypeFlags;
	BOOL m_IsMainIndex;
	BOOL m_WriteAccess;
	BOOL m_VolumeWriteable;
	UINT m_StoreDataSize;
	CHeapfile* m_pTable[IDXTABLECOUNT];

private:
	BOOL ProgressMinorNext(LFProgress* pProgress) const;
	BOOL InspectForUpdate(const LFVariantData* pVData, BOOL& IncludeSlaves, BOOL& DoRename);

	// Wrapper function
	UINT GetFileLocation(LPCCOREATTRIBUTES PtrMaster, LPWSTR pPath, SIZE_T cCount, LFItemDescriptor* pItemDescriptor=NULL, BOOL RemoveNew=FALSE, WIN32_FIND_DATA* pFindData=NULL);

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


// Wrapper function
//

inline UINT CIndex::GetFileLocation(LPCCOREATTRIBUTES PtrMaster, LPWSTR pPath, SIZE_T cCount, LFItemDescriptor* pItemDescriptor, BOOL RemoveNew, WIN32_FIND_DATA* pFindData)
{
	return GetFileLocation(REVENANTFILE(PtrMaster, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrMaster)), pPath, cCount, pItemDescriptor, RemoveNew, pFindData);
}


// Workers.h: Schnittstelle für Methoden zum Threading
//

struct WorkerCreateStoreWindowsParameters
{
	LFWorkerParameters Hdr;
	WCHAR StoreName[256];
	WCHAR Path[MAX_PATH];
};

struct WorkerSynchronizeStoresParameters
{
	LFWorkerParameters Hdr;
	STOREID StoreID;
};

struct WorkerStoreMaintenanceParameters
{
	LFWorkerParameters Hdr;
	LFMaintenanceList* pMaintenanceList;
};

struct WorkerStoreDeleteParameters
{
	LFWorkerParameters Hdr;
	ABSOLUTESTOREID StoreID;
};

struct WorkerSendToParameters
{
	LFWorkerParameters Hdr;
	LFTransactionList* pTransactionList;
	STOREID StoreID;
};

struct WorkerImportParameters
{
	LFWorkerParameters Hdr;
	LFFileImportList* pFileImportList;
	STOREID StoreID;
	LFItemDescriptor* pItemTemplate;
	BOOL DeleteSource;
};

struct WorkerDeleteParameters
{
	LFWorkerParameters Hdr;
	LFTransactionList* pTransactionList;
};

DWORD WINAPI WorkerCreateStoreWindows(LPVOID lpParam);
DWORD WINAPI WorkerSendTo(LPVOID lpParam);
DWORD WINAPI WorkerImport(LPVOID lpParam);
DWORD WINAPI WorkerDelete(LPVOID lpParam);

void LFDoWithProgress(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd=NULL);
void LFRunSynchronizeStores(const STOREID& StoreID=DEFAULTSTOREID(), CWnd* pParentWnd=NULL);
void LFRunStoreMaintenance(CWnd* pParentWnd=NULL);
void LFDeleteStore(const ABSOLUTESTOREID& StoreID, CWnd* pParentWnd=NULL);

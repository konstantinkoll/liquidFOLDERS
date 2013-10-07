
// CContextSidebar.cpp: Implementierung der Klasse CContextSidebar
//

#include "stdafx.h"
#include "CContextSidebar.h"
#include "Resource.h"


// Thread
//

struct WorkerParameters
{
	CHAR StoreID[LFKeySize];
	UINT ThreadID;
	HWND hWndSidebar;
};

UINT __cdecl WorkerStatistics(void* lParam)
{
	CoInitialize(NULL);
	WorkerParameters* wp = (WorkerParameters*)lParam;

	LFStatistics* pStatistics = LFQueryStatistics(wp->StoreID);
	PostMessage(wp->hWndSidebar, WM_STATISTICS, wp->ThreadID, (LPARAM)pStatistics);

	delete wp;
	CoUninitialize();

	return 0;
}


// CContextSidebar
//

CContextSidebar::CContextSidebar()
	: CSidebar()
{
	m_StoreID[0] = '\0';
	m_pStatistics = NULL;
	m_ThreadID = 0;
}

CContextSidebar::~CContextSidebar()
{
	if (m_pStatistics)
		delete m_pStatistics;
}

BOOL CContextSidebar::Create(CWnd* pParentWnd, UINT nID)
{
	return CSidebar::Create(pParentWnd, nID, IDB_CONTEXTS_32, IDB_CONTEXTS_16, TRUE);
}

CString CContextSidebar::AppendTooltip(UINT CmdID)
{
	if (!m_pStatistics)
		return _T("");

	const UINT Context = CmdID-IDM_NAV_SWITCHCONTEXT;
	if (Context>LFLastQueryContext)
		return _T("");
	if (!m_pStatistics->FileCount[Context])
		return _T("");

	WCHAR Buffer[256];
	StrFormatByteSize(m_pStatistics->FileSize[Context], Buffer, 256);

	CString tmpMask;
	ENSURE(tmpMask.LoadString(m_pStatistics->FileCount[Context]==1 ? IDS_FILES_SINGULAR : IDS_FILES_PLURAL));
	
	CString tmpStr;
	tmpStr.Format(tmpMask, m_pStatistics->FileCount[Context]);

	return _T("\n")+tmpStr+_T(" (")+Buffer+_T(")");
}

void CContextSidebar::Reset(UINT CmdID, CHAR* StoreID)
{
	if (strcmp(StoreID, m_StoreID))
	{
		strcpy_s(m_StoreID, LFKeySize, StoreID);
		CSidebar::ResetNumbers();
	}

	OnUpdateNumbers();

	CSidebar::Reset(CmdID);
}


BEGIN_MESSAGE_MAP(CContextSidebar, CSidebar)
	ON_MESSAGE_VOID(WM_UPDATENUMBERS, OnUpdateNumbers)
	ON_MESSAGE(WM_STATISTICS, OnStatistics)
END_MESSAGE_MAP()

void CContextSidebar::OnUpdateNumbers()
{
	WorkerParameters* wp = new WorkerParameters;
	strcpy_s(wp->StoreID, LFKeySize, m_StoreID);
	wp->ThreadID = ++m_ThreadID;
	wp->hWndSidebar = GetSafeHwnd();

	AfxBeginThread(WorkerStatistics, wp);
}

LRESULT CContextSidebar::OnStatistics(WPARAM wParam, LPARAM lParam)
{
	LFStatistics* pStatistics = (LFStatistics*)lParam;

	if (wParam==m_ThreadID)
	{
		if (m_pStatistics)
			delete m_pStatistics;
		m_pStatistics = pStatistics;

		for (UINT a=0; a<=LFLastQueryContext; a++)
			SetNumber(IDM_NAV_SWITCHCONTEXT+a, m_pStatistics->FileCount[a]);
	}
	else
	{
		delete pStatistics;
	}

	return NULL;
}

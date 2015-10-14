
// CContextSidebar.cpp: Implementierung der Klasse CContextSidebar
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CContextSidebar
//

CContextSidebar::CContextSidebar()
	: CSidebar()
{
	m_StoreID[0] = '\0';
	m_pStatistics = NULL;
	m_Initialized = FALSE;
}

CContextSidebar::~CContextSidebar()
{
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

	WCHAR tmpStr[256];
	LFCombineFileCountSize(m_pStatistics->FileCount[Context], m_pStatistics->FileSize[Context], tmpStr, 256);

	return tmpStr;
}

void CContextSidebar::SetSelection(UINT CmdID, CHAR* StoreID)
{
	if ((strcmp(StoreID, m_StoreID)!=0) || !m_Initialized)
	{
		strcpy_s(m_StoreID, LFKeySize, StoreID);
		OnUpdateNumbers();

		m_Initialized = TRUE;
	}

	CSidebar::SetSelection(CmdID);
}


BEGIN_MESSAGE_MAP(CContextSidebar, CSidebar)
	ON_MESSAGE_VOID(WM_UPDATENUMBERS, OnUpdateNumbers)
END_MESSAGE_MAP()

void CContextSidebar::OnUpdateNumbers()
{
	delete m_pStatistics;

	m_pStatistics = LFQueryStatistics(m_StoreID);

	for (UINT a=0; a<=LFLastQueryContext; a++)
		SetNumber(IDM_NAV_SWITCHCONTEXT+a, m_pStatistics->FileCount[a]);
}

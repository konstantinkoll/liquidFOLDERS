
// CContextSidebar.cpp: Implementierung der Klasse CContextSidebar
//

#include "stdafx.h"
#include "CContextSidebar.h"
#include "Resource.h"


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
	
	return m_pStatistics->FileCount[Context] ? CombineFileCountSize(m_pStatistics->FileCount[Context], m_pStatistics->FileSize[Context]) : _T("");
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
	if (m_pStatistics)
		delete m_pStatistics;

	m_pStatistics = LFQueryStatistics(m_StoreID);

	for (UINT a=0; a<=LFLastQueryContext; a++)
		SetNumber(IDM_NAV_SWITCHCONTEXT+a, m_pStatistics->FileCount[a]);

	GetOwner()->SendMessage(WM_SETALERT, (WPARAM)(m_pStatistics->FileCount[LFContextNew] || m_pStatistics->FileCount[LFContextTrash]));
}
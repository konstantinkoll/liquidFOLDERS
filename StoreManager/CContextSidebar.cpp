
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

	if (m_pStatistics)
		delete m_pStatistics;
	m_pStatistics = LFQueryStatistics(m_StoreID);

	for (UINT a=0; a<=LFLastQueryContext; a++)
		SetNumber(IDM_NAV_SWITCHCONTEXT+a, m_pStatistics->FileCount[a]);

	CSidebar::Reset(CmdID);
}


BEGIN_MESSAGE_MAP(CContextSidebar, CSidebar)
END_MESSAGE_MAP()

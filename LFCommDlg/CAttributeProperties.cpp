
// CAttributeProperty.cpp: Implementierung der Klasse CAttributeProperty
//

#include "stdafx.h"
#include "CAttributeProperties.h"
#include "LFEditTagsDlg.h"
#include "LFSelectLocationGPSDlg.h"
#include "LFSelectLocationIATADlg.h"
#include "Resource.h"
#include "LFCore.h"


// CAttributeProperty
//

extern CString strMultiple;

CAttributeProperty::CAttributeProperty(LFVariantData* _pData, CAttributeProperty** _pDependentProp1, CAttributeProperty** _pDependentProp2,
	LPCTSTR Mask, LPCTSTR Template, LPCTSTR ValidChars)
	: CMFCPropertyGridProperty(((LFApplication*)AfxGetApp())->m_Attributes[_pData->Attr]->Name, _T(""), NULL, _pData->Attr, Mask, Template, ValidChars)
{
	p_App = (LFApplication*)AfxGetApp();
	p_Data = _pData;
	p_DependentProp1 = _pDependentProp1;
	p_DependentProp2 = _pDependentProp2;
	m_UseDependencies = 0;
	MaxLength = p_App->m_Attributes[_pData->Attr]->cCharacters;

	wchar_t tmpStr[256];
	LFVariantDataToString(_pData, tmpStr, 256);
	SetValue(tmpStr, FALSE);
}

void CAttributeProperty::SetValue(const COleVariant& varValue, BOOL _Multiple)
{
	Multiple = _Multiple;
	CMFCPropertyGridProperty::SetValue(varValue);
}

void CAttributeProperty::SetDependentValue(const COleVariant& varValue)
{
	m_varValue = varValue;
	m_bIsModified = TRUE;
	Redraw();
}

BOOL CAttributeProperty::IsEditable()
{
	return TRUE;
}

BOOL CAttributeProperty::OnEdit(LPPOINT lptClick)
{
	BOOL res = CMFCPropertyGridProperty::OnEdit(lptClick);
	if ((res) && (m_pWndInPlace))
		((CEdit*)m_pWndInPlace)->SetLimitText(MaxLength);

	return res;
}

BOOL CAttributeProperty::OnUpdateValue()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT_VALID(m_pWndList);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	CString strText;
	m_pWndInPlace->GetWindowText(strText);
	strText.Trim();

	if (p_Data->Attr==LFAttrLanguage)
		strText.MakeUpper();

	if (FormatProperty()!=strText)
	{
		if ((strText.IsEmpty()) && (p_Data->Attr==LFAttrFileName))
			return FALSE;

		m_varValue = (LPCTSTR)strText;
		p_Data->IsNull = false;
		switch (p_Data->Type)
		{
		case LFTypeUnicodeString:
		case LFTypeUnicodeArray:
			wcscpy_s(p_Data->UnicodeString, 256, strText);
			break;
		case LFTypeAnsiString:
			WideCharToMultiByte(CP_ACP, 0, strText, strText.GetLength(), p_Data->AnsiString, 256, NULL, NULL);
			break;
		}

		Multiple = FALSE;
		m_pWndList->OnPropertyChanged(this);
	}

	return TRUE;
}

CString CAttributeProperty::FormatProperty()
{
	return Multiple ? _T("") : CMFCPropertyGridProperty::FormatProperty();
}

void CAttributeProperty::OnDrawName(CDC* pDC, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pWndList);

	COLORREF clrTextOld = (COLORREF)-1;

	if (IsSelected())
	{
		CRect rectFill = rect;
		rectFill.top++;

		if (!((CInspectorGrid*)m_pWndList)->m_bFocused)
		{
			clrTextOld = pDC->SetTextColor(afxGlobalData.clrBtnText);
			pDC->FillRect(rectFill, ((CInspectorGrid*)m_pWndList)->m_bControlBarColors ? &afxGlobalData.brBarFace : &afxGlobalData.brBtnFace);
		}
		else
		{
			if (m_pWndList->GetParent()->IsKindOf(RUNTIME_CLASS(CPane)))
			{
				clrTextOld = pDC->SetTextColor(afxGlobalData.clrTextHilite);
				pDC->FillRect(rectFill, &afxGlobalData.brHilite);
			}
			else
			{
				clrTextOld = pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
				pDC->FillSolidRect(rectFill, GetSysColor(COLOR_HIGHLIGHT));
			}
		}
	}

	if (m_pParent && m_bIsValueList)
		rect.left += rect.Height();
	rect.DeflateRect(AFX_TEXT_MARGIN, 0);

	pDC->DrawText(m_strName, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
	m_bNameIsTruncated = pDC->GetTextExtent(m_strName).cx > rect.Width();

	if (clrTextOld != (COLORREF)-1)
		pDC->SetTextColor(clrTextOld);
}

void CAttributeProperty::OnDrawValue(CDC* pDC, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pWndList);

	CFont* pOldFont = NULL;
	COLORREF oldColor = (COLORREF)-1;
	rect.DeflateRect(AFX_TEXT_MARGIN, 0);
	rect.left++;

	CString strVal = FormatProperty();

	if (Multiple)
	{
		strVal = strMultiple;

		oldColor = pDC->SetTextColor(afxGlobalData.clrGrayedText);
		pOldFont = (CFont*)pDC->SelectObject(((CInspectorGrid*)m_pWndList)->GetItalicFnt());
	}
	else
		if (IsModified())
			pOldFont = (CFont*)pDC->SelectObject(((CInspectorGrid*)m_pWndList)->GetBoldFnt());

	pDC->DrawText(strVal, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
	m_bValueIsTruncated = pDC->GetTextExtent(strVal).cx > rect.Width();

	if (pOldFont)
		pDC->SelectObject(pOldFont);
	if (oldColor!=-1)
		pDC->SetTextColor(oldColor);
}


// CAttributePropertyTags
//

CAttributePropertyTags::CAttributePropertyTags(LFVariantData* _pData, char* _StoreID)
	: CAttributeProperty(_pData)
{
	SetStore(_StoreID);
}

BOOL CAttributePropertyTags::HasButton() const
{
	return TRUE;
}

void CAttributePropertyTags::OnClickButton(CPoint /*point*/)
{
	LFEditTagsDlg dlg(NULL, Multiple ? _T("") : p_Data->UnicodeArray, StoreIDValid ? StoreID : NULL);

	if (dlg.DoModal()==IDOK)
	{
		p_Data->IsNull = false;
		wcscpy_s(p_Data->UnicodeArray, 256, dlg.m_Tags);
		SetValue(dlg.m_Tags, FALSE);
	}
}

BOOL CAttributePropertyTags::OnUpdateValue()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT_VALID(m_pWndList);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	CString strText;
	m_pWndInPlace->GetWindowText(strText);
	strText.Trim();

	if ((strText.IsEmpty()) && (p_Data->Attr==LFAttrFileName))
		return FALSE;

	wchar_t tmpStr[256];
	wcscpy_s(tmpStr, 256, strText);
	LFSanitizeUnicodeArray(tmpStr, 256);
	strText = tmpStr;

	if (FormatProperty()!=strText)
	{
		m_varValue = (LPCTSTR)strText;
		p_Data->IsNull = false;
		wcscpy_s(p_Data->UnicodeString, 256, strText);

		Multiple = FALSE;
		m_pWndList->OnPropertyChanged(this);
	}

	return TRUE;
}

void CAttributePropertyTags::SetStore(char* _StoreID)
{
	StoreIDValid = (_StoreID!=NULL);
	if (_StoreID)
		strcpy_s(StoreID, LFKeySize, _StoreID);
}


// CAttributePropertyIATA
//

CAttributePropertyIATA::CAttributePropertyIATA(LFVariantData* _pData, CAttributeProperty** _pDependentProp1, CAttributeProperty** _pDependentProp2)
	: CAttributeProperty(_pData, _pDependentProp1, _pDependentProp2, NULL, NULL, _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"))
{
}

BOOL CAttributePropertyIATA::HasButton() const
{
	return TRUE;
}

void CAttributePropertyIATA::OnClickButton(CPoint /*point*/)
{
	LFSelectLocationIATADlg dlg(NULL, IDD_SELECTIATA, &p_Data->AnsiString[0]);

	if (dlg.DoModal()==IDOK)
		if (dlg.m_Airport)
		{
			if ((p_DependentProp1) && (dlg.m_IATA_OverwriteName))
			{
				ASSERT((*p_DependentProp1)->p_Data->Attr==LFAttrLocationName);
				size_t sz = strlen(dlg.m_Airport->Name)+1;
				(*p_DependentProp1)->p_Data->IsNull = false;
				MultiByteToWideChar(CP_ACP, 0, dlg.m_Airport->Name, (int)sz, (*p_DependentProp1)->p_Data->UnicodeString, (int)sz);
				(*p_DependentProp1)->SetDependentValue((*p_DependentProp1)->p_Data->UnicodeString);
				m_UseDependencies |= 1;
			}
			if ((p_DependentProp2) && (dlg.m_IATA_OverwriteGPS))
			{
				ASSERT((*p_DependentProp2)->p_Data->Attr==LFAttrLocationGPS);
				(*p_DependentProp2)->p_Data->IsNull = false;
				(*p_DependentProp2)->p_Data->GeoCoordinates = dlg.m_Airport->Location;
				wchar_t tmpStr[256];
				LFGeoCoordinatesToString(dlg.m_Airport->Location, tmpStr, 256, true);
				(*p_DependentProp2)->SetDependentValue(tmpStr);
				m_UseDependencies |= 2;
			}

			p_Data->IsNull = false;
			strcpy_s(p_Data->AnsiString, 256, dlg.m_Airport->Code);
			CString tmpStr(dlg.m_Airport->Code);
			SetValue(tmpStr.MakeUpper(), FALSE);
		}
}

BOOL CAttributePropertyIATA::OnUpdateValue()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT_VALID(m_pWndList);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	CString strText;
	m_pWndInPlace->GetWindowText(strText);
	if ((strText.GetLength()!=0) && (strText.GetLength()!=3))
		return FALSE;

	strText.MakeUpper();

	if (FormatProperty()!=strText)
	{
		m_varValue = (LPCTSTR)strText;
		p_Data->IsNull = false;
		WideCharToMultiByte(CP_ACP, 0, strText, strText.GetLength()+1, p_Data->AnsiString, 256, NULL, NULL);

		Multiple = FALSE;
		m_pWndList->OnPropertyChanged(this);
	}

	return TRUE;
}


// CAttributePropertyGPS
//

CAttributePropertyGPS::CAttributePropertyGPS(LFVariantData* _pData)
	: CAttributeProperty(_pData)
{
}

BOOL CAttributePropertyGPS::HasButton() const
{
	return TRUE;
}

BOOL CAttributePropertyGPS::IsEditable()
{
	return FALSE;
}

void CAttributePropertyGPS::OnClickButton(CPoint /*point*/)
{
	LFSelectLocationGPSDlg dlg(NULL, p_Data->GeoCoordinates);

	if (dlg.DoModal()==IDOK)
	{
		p_Data->GeoCoordinates = dlg.m_Location;
		p_Data->IsNull = false;

		wchar_t tmpStr[256];
		LFGeoCoordinatesToString(p_Data->GeoCoordinates, &tmpStr[0], 256, true);
		SetValue(tmpStr, FALSE);
	}
}


// CAttributePropertyRating
//

CAttributePropertyRating::CAttributePropertyRating(LFVariantData* _pData)
	: CAttributeProperty(_pData)
{
}

BOOL CAttributePropertyRating::OnEdit(LPPOINT lptClick)
{
	if (lptClick)
	{
		Multiple = FALSE;
		int pos = lptClick->x-m_pWndList->GetLeftColumnWidth();

		int rating = 2*(pos/18)+(pos%18>=7);
		if (rating<0)
			rating = 0;
		if (rating>LFMaxRating)
			rating = LFMaxRating;

		if (p_Data->Rating!=(unsigned char)rating)
		{
			p_Data->Rating = (unsigned char)rating;
			p_Data->IsNull = false;
			Redraw();
			m_pWndList->OnPropertyChanged(this);
		}
	}

	return FALSE;
}

BOOL CAttributePropertyRating::OnSetCursor() const
{
	return FALSE;
}

BOOL CAttributePropertyRating::PushChar(UINT nChar)
{
	Multiple = FALSE;
	int rating = p_Data->Rating;

	// + -
	switch (nChar)
	{
	case '+':
		rating++;
		break;
	case '-':
		rating--;
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		rating = (nChar-'0')*2;
		break;
	}

	if (rating<0)
		rating = 0;
	if (rating>LFMaxRating)
		rating = LFMaxRating;

	if (p_Data->Rating!=(unsigned char)rating)
	{
		p_Data->Rating = (unsigned char)rating;
		p_Data->IsNull = false;
		Redraw();
		m_pWndList->OnPropertyChanged(this);
	}

	return TRUE;
}

void CAttributePropertyRating::OnDrawValue(CDC* pDC, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pWndList);

	HDC hdcMem = CreateCompatibleDC(pDC->m_hDC);
	UCHAR level = Multiple ? 0 : p_Data->Rating;
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, p_Data->Attr==LFAttrRating ? p_App->m_RatingBitmaps[level] : p_App->m_PriorityBitmaps[level]);

	int w = min(rect.Width()-6, RatingBitmapWidth);
	int h = min(rect.Height(), RatingBitmapHeight);
	BLENDFUNCTION LF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
	AlphaBlend(pDC->m_hDC, rect.left+6, rect.top+(rect.Height()-h)/2, w, h, hdcMem, 0, 0, w, h, LF);

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);
}


// CAttributePropertyTime
//

CPropDateTimeCtrl::CPropDateTimeCtrl(CAttributePropertyTime *pProp, COLORREF clrBack)
{
	m_clrBack = clrBack;
	m_brBackground.CreateSolidBrush(m_clrBack);
	m_pProp = pProp;
}

BEGIN_MESSAGE_MAP(CPropDateTimeCtrl, CDateTimeCtrl)
	ON_WM_KILLFOCUS()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_HSCROLL_REFLECT()
END_MESSAGE_MAP()


void CPropDateTimeCtrl::OnKillFocus(CWnd* pNewWnd)
{
	if(pNewWnd!=NULL && IsChild(pNewWnd))
		return;

	CDateTimeCtrl::OnKillFocus(pNewWnd);
}

HBRUSH CPropDateTimeCtrl::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetBkColor(m_clrBack);
	return m_brBackground;
}

void CPropDateTimeCtrl::HScroll(UINT /*nSBCode*/, UINT /*nPos*/)
{
	ASSERT_VALID(m_pProp);

	m_pProp->OnUpdateValue();
	m_pProp->Redraw();
}

LRESULT CPropDateTimeCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message>=WM_MOUSEFIRST && message<=WM_MOUSELAST)
	{
		POINT pt;
		GetCursorPos(&pt);
		CWnd *wnd=NULL;
		CRect rect;
		wnd=GetWindow(GW_CHILD);
		while(wnd)
		{
			wnd->GetWindowRect(rect);
			if(rect.PtInRect(pt))
			{
				wnd->SendMessage(message,wParam,lParam);
				return TRUE;
			}
			wnd=wnd->GetWindow(GW_HWNDNEXT);
		}
	}
	if (message==WM_DESTROY)
		SetFont(NULL,FALSE);

	return CDateTimeCtrl::WindowProc(message, wParam, lParam);
}


CAttributePropertyTime::CAttributePropertyTime(LFVariantData* _pData)
	: CAttributeProperty(_pData)
{
	LeftDate = RightDate = LeftTime = RightTime = 0;
}

BOOL CAttributePropertyTime::OnEdit(LPPOINT lptClick)
{
	if (lptClick)
	{
		CRect rectEdit;
		CRect rectSpin;
		AdjustInPlaceEditRect(rectEdit, rectSpin);

		SYSTEMTIME st;
		FileTimeToSystemTime(&p_Data->Time, &st);

		int pos = lptClick->x-m_pWndList->GetLeftColumnWidth();

		if ((pos>=LeftDate) && (pos<=RightDate))
		{
			Multiple = FALSE;
			CPropDateTimeCtrl* pWndDateTime = new CPropDateTimeCtrl(this, m_pWndList->GetBkColor());

			DWORD dwStyle = WS_VISIBLE | WS_CHILD | DTS_SHORTDATEFORMAT;
			rectEdit.InflateRect(4, 4, 0, 3);
			pWndDateTime->Create(dwStyle, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE);
			pWndDateTime->SetTime(&st);
			m_pWndInPlace = pWndDateTime;
		}

		if ((pos>=LeftTime) && (pos<=RightTime))
		{
			Multiple = FALSE;
			CPropDateTimeCtrl* pWndDateTime = new CPropDateTimeCtrl(this, m_pWndList->GetBkColor());

			DWORD dwStyle = WS_VISIBLE | WS_CHILD | DTS_TIMEFORMAT;
			rectEdit.InflateRect(7-LeftTime, 4, 0, 3);
			pWndDateTime->Create(dwStyle, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE);
			pWndDateTime->SetTime(&st);
			m_pWndInPlace = pWndDateTime;
		}

		if (m_pWndInPlace)
		{
			m_pWndInPlace->SetFocus();
			m_bInPlaceEdit = TRUE;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CAttributePropertyTime::OnUpdateValue()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT_VALID(m_pWndList);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	CDateTimeCtrl* pProp = (CDateTimeCtrl*)m_pWndInPlace;

	SYSTEMTIME st;
	pProp->GetTime(&st);
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);

	if ((ft.dwHighDateTime!=p_Data->Time.dwHighDateTime) || (ft.dwLowDateTime!=p_Data->Time.dwLowDateTime))
	{
		p_Data->Time = ft;
		wchar_t tmpStr[256];
		LFTimeToString(ft, tmpStr, 256);
		m_varValue = tmpStr;
		Redraw();

		Multiple = FALSE;
		m_pWndList->OnPropertyChanged(this);
	}

	return TRUE;
}

BOOL CAttributePropertyTime::OnSetCursor() const
{
	return FALSE;
}

void CAttributePropertyTime::OnDrawValue(CDC* pDC, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pWndList);

	CFont* pOldFont = NULL;
	COLORREF oldColor = (COLORREF)-1;
	rect.DeflateRect(AFX_TEXT_MARGIN, 0);

	wchar_t tmpStr1[256];
	wchar_t tmpStr2[256];

	if (((p_Data->Time.dwHighDateTime) || (p_Data->Time.dwLowDateTime)) && (!Multiple))
	{
		LFTimeToString(p_Data->Time, tmpStr1, 256, 1);
		LFTimeToString(p_Data->Time, tmpStr2, 256, 2);

		if (IsModified())
			pOldFont = (CFont*)pDC->SelectObject(((CInspectorGrid*)m_pWndList)->GetBoldFnt());
	}
	else
	{
		wcscpy_s(tmpStr1, 256, L"Date");
		wcscpy_s(tmpStr2, 256, L"time");

		oldColor = pDC->SetTextColor(afxGlobalData.clrGrayedText);
		pOldFont = (CFont*)pDC->SelectObject(((CInspectorGrid*)m_pWndList)->GetItalicFnt());
	}

	CString strVal = tmpStr1;
	strVal += _T(", ");
	strVal += tmpStr2;
	pDC->DrawText(strVal, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
	m_bValueIsTruncated = pDC->GetTextExtent(strVal).cx > rect.Width();

	LeftDate = ((CInspectorGrid*)m_pWndList)->GetLeftMargin();
	RightDate = LeftDate+AFX_TEXT_MARGIN+1+pDC->GetTextExtent(tmpStr1).cx;
	LeftTime = RightDate+pDC->GetTextExtent(_T(", ")).cx;
	RightTime = LeftTime+pDC->GetTextExtent(tmpStr2).cx;

	if (pOldFont)
		pDC->SelectObject(pOldFont);
	if (oldColor!=-1)
		pDC->SetTextColor(oldColor);
}

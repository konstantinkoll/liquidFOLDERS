
// CAttributeProperty.h: Schnittstelle der Klasse CAttributeProperty
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFApplication.h"


// CAttributeProperty
//

class AFX_EXT_CLASS CAttributeProperty : public CMFCPropertyGridProperty
{
public:
	CAttributeProperty(LFVariantData* _pData, CAttributeProperty** _pDependentProp1=NULL, CAttributeProperty** _pDependentProp2=NULL,
		LPCTSTR Mask=NULL, LPCTSTR Template=NULL, LPCTSTR ValidChars=NULL);
	virtual ~CAttributeProperty();

	virtual BOOL IsEditable();
	virtual BOOL OnEdit(LPPOINT lptClick);
	virtual BOOL OnUpdateValue();
	virtual CString FormatProperty();
	virtual void SetValue(const COleVariant& varValue, BOOL _Multiple);
	virtual void OnDrawName(CDC* pDC, CRect rect);

	void SetDependentValue(const COleVariant& varValue);

	LFVariantData* p_Data;
	CAttributeProperty** p_DependentProp1;
	CAttributeProperty** p_DependentProp2;
	UINT m_UseDependencies;
	BOOL Multiple;

protected:
	LFApplication* p_App;
	int MaxLength;
};


// CAttributePropertyTags
//

class AFX_EXT_CLASS CAttributePropertyTags : public CAttributeProperty
{
public:
	CAttributePropertyTags(LFVariantData* _pData, char* _StoreID=NULL);
	virtual ~CAttributePropertyTags();

	virtual BOOL HasButton() const;
	virtual void OnClickButton(CPoint point);
	virtual BOOL OnUpdateValue();

	void SetStore(char* _StoreID);

protected:
	char StoreID[LFKeySize];
	BOOL StoreIDValid;
};


// CAttributePropertyIATA
//

class AFX_EXT_CLASS CAttributePropertyIATA : public CAttributeProperty
{
public:
	CAttributePropertyIATA(LFVariantData* _pData, CAttributeProperty** _pDependentProp1=NULL, CAttributeProperty** _pDependentProp2=NULL);
	virtual ~CAttributePropertyIATA();

	virtual BOOL HasButton() const;
	virtual void OnClickButton(CPoint point);
	virtual BOOL OnUpdateValue();
};


// CAttributePropertyGPS
//

class AFX_EXT_CLASS CAttributePropertyGPS : public CAttributeProperty
{
public:
	CAttributePropertyGPS(LFVariantData* _pData);
	virtual ~CAttributePropertyGPS();

	virtual BOOL HasButton() const;
	virtual BOOL IsEditable();
	virtual void OnClickButton(CPoint point);
};


// CAttributePropertyRating
//

class AFX_EXT_CLASS CAttributePropertyRating : public CAttributeProperty
{
public:
	CAttributePropertyRating(LFVariantData* _pData);
	virtual ~CAttributePropertyRating();

	virtual BOOL OnEdit(LPPOINT lptClick);
	virtual BOOL OnSetCursor() const;
	virtual BOOL PushChar(UINT nChar);
	virtual void OnDrawValue(CDC* pDC, CRect rect);
};


// CAttributePropertyTime
//

class CAttributePropertyTime;

class CPropDateTimeCtrl: public CDateTimeCtrl
{
public:
	CPropDateTimeCtrl(CAttributePropertyTime* pProp, COLORREF clrBack);

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void HScroll(UINT nSBCode, UINT nPos);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

	CBrush m_brBackground;
	COLORREF m_clrBack;
	CAttributePropertyTime* m_pProp;
};

class AFX_EXT_CLASS CAttributePropertyTime : public CAttributeProperty
{
public:
	CAttributePropertyTime(LFVariantData* _pData);
	virtual ~CAttributePropertyTime();

	virtual BOOL OnEdit(LPPOINT lptClick);
	virtual BOOL OnUpdateValue();
	virtual BOOL OnSetCursor() const;
	virtual void OnDrawValue(CDC* pDC, CRect rect);

protected:
	int LeftDate;
	int RightDate;
	int LeftTime;
	int RightTime;
};

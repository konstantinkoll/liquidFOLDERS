#pragma once

const IID IID_IListViewFooter = {0xF0034DA8, 0x8A22, 0x4151, {0x8F, 0x16, 0x2E, 0xBA, 0x76, 0x56, 0x5B, 0xCC}};
const IID IID_IListViewFooterCallback = {0x88EB9442, 0x913B, 0x4AB4, {0xA7, 0x41, 0xDD, 0x99, 0xDC, 0xB7, 0x55, 0x8B}};


class IListViewFooterCallback : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE OnButtonClicked(INT itemIndex, LPARAM lParam, PINT pRemoveFooter) = 0;
	virtual HRESULT STDMETHODCALLTYPE OnDestroyButton(INT itemIndex, LPARAM lParam) = 0;
};


class IListViewFooter : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE IsVisible(PINT pVisible) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetFooterFocus(PINT pItemIndex) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetFooterFocus(INT itemIndex) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetIntroText(LPCWSTR pText) = 0;
	virtual HRESULT STDMETHODCALLTYPE Show(IListViewFooterCallback* pCallbackObject) = 0;
	virtual HRESULT STDMETHODCALLTYPE RemoveAllButtons(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE InsertButton(INT insertAt, LPCWSTR pText, LPCWSTR pUnknown, UINT iconIndex, LONG lParam) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetButtonLParam(INT itemIndex, LONG* pLParam) = 0;
};

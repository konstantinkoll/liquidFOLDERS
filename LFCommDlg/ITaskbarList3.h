
// ITaskbarList.h: Schnittstelle der Klassen ITaskbarList
//

#pragma once

typedef enum TBPFLAG
{
	TBPF_NOPROGRESS = 0x0,
	TBPF_INDETERMINATE = 0x1,
	TBPF_NORMAL = 0x2,
	TBPF_ERROR = 0x4,
	TBPF_PAUSED = 0x8
};

typedef struct tagTHUMBBUTTON
{
	DWORD dwMask;
	UINT iId;
	UINT iBitmap;
	HICON hIcon;
	WCHAR pszTip[260];
	DWORD dwFlags;
} THUMBBUTTON;

typedef struct tagTHUMBBUTTON *LPTHUMBBUTTON;

typedef enum THUMBBUTTONMASK
{
	THB_BITMAP = 0x1,
	THB_ICON = 0x2,
	THB_TOOLTIP = 0x4,
	THB_FLAGS = 0x8
} THUMBBUTTONMASK;

typedef enum THUMBBUTTONFLAGS
{
	THBF_ENABLED = 0x0,
	THBF_DISABLED = 0x1,
	THBF_DISMISSONCLICK = 0x2,
	THBF_NOBACKGROUND = 0x4,
	THBF_HIDDEN = 0x8,
	THBF_NONINTERACTIVE = 0x10
} THUMBBUTTONFLAGS;

interface ITaskbarList3 : public ITaskbarList2
{
	virtual HRESULT STDMETHODCALLTYPE SetProgressValue(HWND hwnd, ULONGLONG ullCompleted, ULONGLONG ullTotal) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetProgressState(HWND hwnd, TBPFLAG tbpFlags) = 0;
	virtual HRESULT STDMETHODCALLTYPE RegisterTab(HWND hwndTab, HWND hwndMDI) = 0;
	virtual HRESULT STDMETHODCALLTYPE UnregisterTab(HWND hwndTab) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetTabOrder(HWND hwndTab, HWND hwndInsertBefore) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetTabActive(HWND hwndTab, HWND hwndMDI, DWORD dwReserved) = 0;
	virtual HRESULT STDMETHODCALLTYPE ThumbBarAddButtons(HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) = 0;
	virtual HRESULT STDMETHODCALLTYPE ThumbBarUpdateButtons(HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) = 0;
	virtual HRESULT STDMETHODCALLTYPE ThumbBarSetImageList(HWND hwnd, HIMAGELIST himl) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetOverlayIcon(HWND hwnd, HICON hIcon, LPCWSTR pszDescription) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetThumbnailTooltip(HWND hwnd, LPCWSTR pszTip) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetThumbnailClip(HWND hwnd, RECT *prcClip) = 0;
};

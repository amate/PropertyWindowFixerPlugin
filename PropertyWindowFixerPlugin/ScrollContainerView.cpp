
#include "pch.h"
#include "ScrollContainerView.h"
#include <cstdint>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

extern HMODULE g_hModule;

CScrollConteinerView::CScrollConteinerView() : m_bDebug(false), m_propertyWindow(this, 1)
{
}



void CScrollConteinerView::DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine)
{
	RECT rect = { 0 };
	GetClientRect(&rect);
	int cxyClient = (nType == SB_VERT) ? rect.bottom : rect.right;
	int cxyMax = cxySizeAll - cxyClient;

	if (cxyMax < 0)   // can't scroll, client area is bigger
		return;

	bool bUpdate = true;
	int cxyScroll = 0;

	switch (nScrollCode)
	{
	case SB_TOP:		// top or all left
		cxyScroll = cxyOffset;
		cxyOffset = 0;
		break;
	case SB_BOTTOM:		// bottom or all right
		cxyScroll = cxyOffset - cxyMax;
		cxyOffset = cxyMax;
		break;
	case SB_LINEUP:		// line up or line left
		if (cxyOffset >= cxySizeLine)
		{
			cxyScroll = cxySizeLine;
			cxyOffset -= cxySizeLine;
		} else
		{
			cxyScroll = cxyOffset;
			cxyOffset = 0;
		}
		break;
	case SB_LINEDOWN:	// line down or line right
		if (cxyOffset < cxyMax - cxySizeLine)
		{
			cxyScroll = -cxySizeLine;
			cxyOffset += cxySizeLine;
		} else
		{
			cxyScroll = cxyOffset - cxyMax;
			cxyOffset = cxyMax;
		}
		break;
	case SB_PAGEUP:		// page up or page left
		if (cxyOffset >= cxySizePage)
		{
			cxyScroll = cxySizePage;
			cxyOffset -= cxySizePage;
		} else
		{
			cxyScroll = cxyOffset;
			cxyOffset = 0;
		}
		break;
	case SB_PAGEDOWN:	// page down or page right
		if (cxyOffset < cxyMax - cxySizePage)
		{
			cxyScroll = -cxySizePage;
			cxyOffset += cxySizePage;
		} else
		{
			cxyScroll = cxyOffset - cxyMax;
			cxyOffset = cxyMax;
		}
		break;
	case SB_THUMBTRACK:
		if (true/*IsNoThumbTracking()*/)
			break;
		// else fall through
	case SB_THUMBPOSITION:
	{
		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_TRACKPOS };
		if (GetScrollInfo(nType, &si))
		{
			cxyScroll = cxyOffset - si.nTrackPos;
			cxyOffset = si.nTrackPos;
		}
	}
	break;
	case SB_ENDSCROLL:
	default:
		bUpdate = false;
		break;
	}

	if (bUpdate && (cxyScroll != 0))
	{
		UINT m_uScrollFlags = 0;
		SetScrollPos(nType, cxyOffset, TRUE);
		if (nType == SB_VERT) {
			RECT rect = { 0 };
			::GetWindowRect(m_propertyWindow, &rect);
			::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 1);
			::SetWindowPos(m_propertyWindow, NULL, 0, -cxyOffset, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

			//ScrollWindowEx(0, cxyScroll, (m_uScrollFlags & ~SCRL_SCROLLCHILDREN));
		} else {
			SetScrollOffset(cxyOffset, 0);
			//ScrollWindowEx(cxyScroll, 0, (m_uScrollFlags & ~SCRL_SCROLLCHILDREN));
		}
	}
}



int CScrollConteinerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	WCHAR debugFilePath[MAX_PATH];
	::GetModuleFileName(g_hModule, debugFilePath, MAX_PATH);
	::PathRemoveFileSpec(debugFilePath);
	::PathAppend(debugFilePath, L"PropertyWindowFixerPluginDebug");
	m_bDebug = ::PathFileExists(debugFilePath) != 0;

	const DWORD currentThreadId = ::GetCurrentThreadId();
	HWND hwnd = ::FindWindowEx(NULL, NULL, L"ExtendedFilterClass", nullptr);
	if (hwnd != NULL) {
		do {
			const DWORD windowThreadId = ::GetWindowThreadProcessId(hwnd, nullptr);
			if (currentThreadId == windowThreadId) {
				break;
			}
			hwnd = ::FindWindowEx(NULL, hwnd, L"ExtendedFilterClass", nullptr);
		} while (hwnd);
	}
	if (hwnd == NULL) {
		MessageBox(L"拡張編集のプロパティウィンドウが見つかりません...", L"PropertyWindowFixerPlugin");
		return 0;
	}

	m_propertyWindow.SubclassWindow(hwnd);

	m_propertyWindow.GetWindowRect(&m_rcLastPropertyWindowPos);
	m_propertyWindow.SetParent(m_hWnd);
	m_propertyWindow.ModifyStyle(WS_CAPTION, 0);

	m_propertyWindow.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	OnPropertySize(0, m_rcLastPropertyWindowPos.Size());

	return 0;
}

void CScrollConteinerView::OnDestroy()
{
	if (m_propertyWindow.IsWindow()) {
		CWindow propertyWindow = m_propertyWindow;
		m_propertyWindow.UnsubclassWindow();

		CRect rcNowPropertyWindowRect;
		propertyWindow.GetWindowRect(&rcNowPropertyWindowRect);

		propertyWindow.ModifyStyle(0, WS_CAPTION);
		propertyWindow.SetParent(NULL);
		propertyWindow.SetWindowPos(NULL, m_rcLastPropertyWindowPos.left, m_rcLastPropertyWindowPos.top, rcNowPropertyWindowRect.Width(), rcNowPropertyWindowRect.Height(), SWP_NOZORDER);

	}

}

LRESULT CScrollConteinerView::OnDelayInvalidateRect(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_propertyWindow.Invalidate();
	if (m_bDebug) {
		Invalidate();
		UpdateWindow();
		m_propertyWindow.UpdateWindow();
	}
	return 0;
}


void CScrollConteinerView::OnPropertySize(UINT nType, CSize size)
{
#if 0
	// プロパティウィンドウが一定以上の縦幅にならないため、強制的に幅を広げる措置
	// SWP_NOSENDCHANGING を指定することによって、幅の制限を防ぐ
	if (size.cy >= 1000) {	
		size.cy = 2000;
	}
#endif
	m_propertyWindow.SetWindowPos(NULL,0, 0, size.cx, size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSENDCHANGING);

	enum { kBufferSize = 128 };
	CString propertyTitle;
	m_propertyWindow.GetWindowText(propertyTitle.GetBuffer(kBufferSize), kBufferSize);
	propertyTitle.ReleaseBuffer();

	GetParent().SetWindowText(propertyTitle);

	SetScrollSize(size, FALSE, false);
	PostMessage(WM_DELAY_INVALIDATERECT);
}

// 拡張編集側でのプロパティウィンドウのサイズ制限を抑止するために、WM_WINDOWPOSCHANGINGメッセージを横取りする
LRESULT CScrollConteinerView::OnPropertyWindowPosChanging(UINT, WPARAM, LPARAM, BOOL&)
{
	return 0;
}

LRESULT CScrollConteinerView::OnPropertyMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_nWheelLines = kWheelLines;
	return __super::OnMouseWheel(uMsg, wParam, lParam, bHandled);;
}

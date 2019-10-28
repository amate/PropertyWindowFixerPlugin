
#include "stdafx.h"
#include "MainFrm.h"

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
#if 1
	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);
	// replace with appropriate values for the app
	m_view.SetScrollSize(2000, 1000);
#endif
#if 0
	ModifyStyle(0, WS_VSCROLL);

	CWindow propertyWindow = ::FindWindow(L"ExtendedFilterClass", nullptr);
	m_hWndClient = propertyWindow;
	propertyWindow.GetWindowRect(&m_rcLastPropertyWindowPos);
	propertyWindow.SetParent(m_hWnd);
	propertyWindow.ModifyStyle(WS_CAPTION, 0);

	propertyWindow.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
#endif
	//UpdateLayout();

	//m_hWndClient = ::FindWindow(L"ExtendedFilterClass", nullptr);
	//CWindow propertyWindow = m_hWndClient;


	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
#if 0
	CWindow propertyWindow = m_hWndClient;
	propertyWindow.ModifyStyle(0, WS_CAPTION);

	propertyWindow.SetParent(NULL);
	propertyWindow.SetWindowPos(NULL, m_rcLastPropertyWindowPos.left, m_rcLastPropertyWindowPos.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
#endif

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}


LRESULT CMainFrame::OnVScroll(UINT, WPARAM wParam, LPARAM, BOOL&)
{
	CWindow propertyWindow = m_hWndClient;
	CRect rcPropertyWindow;
	propertyWindow.GetWindowRect(&rcPropertyWindow);
	ScreenToClient(&rcPropertyWindow);

	rcPropertyWindow.OffsetRect(0, -10);
	propertyWindow.MoveWindow(&rcPropertyWindow);

	//DoScroll(SB_VERT, (int)(short)LOWORD(wParam), (int&)m_ptOffset.y, rcPropertyWindow.bottom/*m_sizeAll.cy*/, 10, 10);
	return 0;
}

void CMainFrame::DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine)
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
		if (nType == SB_VERT)
			ScrollWindowEx(0, cxyScroll, m_uScrollFlags);
		else
			ScrollWindowEx(cxyScroll, 0, m_uScrollFlags);
	}
}

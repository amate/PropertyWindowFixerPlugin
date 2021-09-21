// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>


class CScrollConteinerView : 
	public CScrollWindowImpl<CScrollConteinerView>
{
public:
	//DECLARE_WND_CLASS(NULL)
	DECLARE_WND_CLASS_EX(L"PropertyWindowFixerPlugin", CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BTNFACE)

	enum { 
		WM_DELAY_INVALIDATERECT = WM_APP + 1,
		WM_DELAY_PROPERTYCHANGED = WM_APP + 2,
		WM_DELAY_CALCANDSETSCROLLSIZE = WM_APP + 3,
		WM_DELAY_CORRECTMENUPOSITION = WM_APP + 4,

		kDelaySwitchManualShowHideTimerId = 1,
		kDelaySwitchManualShowHideTimerInterval = 100,
		kDelayRestoreWindowId = 2,
		kDelayRestoreWindowInterval = 100,
		
		kEdgeRightHeaderPartId = 0x453,
		kEvalcxMargin = 5,

		kcxScrollLine = 20,

		ID_COMMAND_FILTERPLUS = 0x453,
	};

	CScrollConteinerView();

	BOOL SubclassWindow();
	void UnsubclassWindow();

	// Overrides
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	void DoPaint(CDCHandle dc)
	{
		//TODO: Add your drawing code here
	}

	void DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine);

	LRESULT DefWindowProc();
	LRESULT DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	BEGIN_MSG_MAP_EX(CScrollConteinerView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_TIMER(OnTimer)

		COMMAND_ID_HANDLER(ID_COMMAND_FILTERPLUS, OnFilterPlus)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_INITMENU, OnInitMenu)
		MESSAGE_HANDLER(WM_DELAY_CORRECTMENUPOSITION, OnDelayCorrectMenuPosition)		

		MSG_WM_WINDOWPOSCHANGING(OnWindowPosChanging)
		MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
		MSG_WM_COMMAND(OnPropertyWindowCommand)
		MSG_WM_SIZING(OnSizing)
		MSG_WM_SIZE(OnSize)

		MESSAGE_HANDLER(WM_SETTEXT, OnSetText)
		MESSAGE_HANDLER(WM_DELAY_PROPERTYCHANGED, OnDelayPropertyChanged)
		MESSAGE_HANDLER(WM_DELAY_CALCANDSETSCROLLSIZE, OnDelayCalcAndSetscrollsize)

		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnPropertyMouseWheel)

		MESSAGE_HANDLER(WM_DELAY_INVALIDATERECT, OnDelayInvalidateRect)
		CHAIN_MSG_MAP(CScrollWindowImpl<CScrollConteinerView>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	void OnClose();
	void OnPaint(CDCHandle dc);
	void OnTimer(UINT_PTR nIDEvent);

	LRESULT OnFilterPlus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDelayCorrectMenuPosition(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
	void OnWindowPosChanged(LPWINDOWPOS lpWndPos);
	void OnPropertyWindowCommand(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnSizing(UINT fwSide, LPRECT pRect);
	void OnSize(UINT nType, CSize size);

	LRESULT OnSetText(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDelayPropertyChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDelayCalcAndSetscrollsize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		_CalcAndChangeScrollSize();
		return 0;
	}

	LRESULT OnDelayInvalidateRect(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnPropertyMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);


private:
	HWND	_FindExPropertyWindow();
	CRect	_GetWindowRelativePositon(HWND hwndChild);
	void	_CalcAndChangeScrollSize();

	bool	m_bDebug;

	CRect	m_rcFixedWindow;

	bool	m_bPropertyChanging;
	bool	m_bManualShowHideFilter;
	bool	m_bNoCorrectMenuPositon;
	CPoint	m_ptLastOffset;

	std::vector<std::pair<HWND, CRect>>	m_defaultHeaderPartPosition;
	int		m_barBottom;

	LONG_PTR m_pfnDefWndProc = 0;
	LONG_PTR m_pfnSubWndProc = 0;

	// config
	int		m_cyScrollLine = 60;
	int		m_cyMargin = 120;
	bool	m_bAlwaysRestoreScrollPos = false;
};

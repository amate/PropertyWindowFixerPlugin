// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


class CScrollConteinerView : 
	public CScrollWindowImpl<CScrollConteinerView>
{
public:
	//DECLARE_WND_CLASS(NULL)
	DECLARE_WND_CLASS_EX(L"PropertyWindowFixerPlugin", CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BTNFACE)

	enum { 
		WM_DELAY_INVALIDATERECT = WM_APP + 1,
		
		kWheelLines = 12,
	};

	CScrollConteinerView();

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

	BEGIN_MSG_MAP_EX(CScrollConteinerView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MESSAGE_HANDLER(WM_DELAY_INVALIDATERECT, OnDelayInvalidateRect)
		CHAIN_MSG_MAP(CScrollWindowImpl<CScrollConteinerView>)
	ALT_MSG_MAP(1)
		MSG_WM_SIZE(OnPropertySize)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnPropertyWindowPosChanging)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnPropertyMouseWheel)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();

	LRESULT OnDelayInvalidateRect(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void OnPropertySize(UINT nType, CSize size);
	LRESULT OnPropertyWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPropertyMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:

	CContainedWindow	m_propertyWindow;
	CRect	m_rcLastPropertyWindowPos;

};

// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlscrl.h>
#include <atlcrack.h>
#include <atlmisc.h>

#include "resource.h"

class CScrollConteinerView : public CScrollWindowImpl<CScrollConteinerView>
{
public:
	DECLARE_WND_CLASS(NULL)

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
		CHAIN_MSG_MAP(CScrollWindowImpl<CScrollConteinerView>)
	ALT_MSG_MAP(1)

	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();

private:

	//CContainedWindow	m_propertyWindow;
	CWindow			m_propertyWindow;
	CRect	m_rcLastPropertyWindowPos;
};

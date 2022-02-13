
#include "pch.h"
#include "ScrollContainerView.h"
#include <unordered_set>
#include <cstdint>
#include <shlwapi.h>
#include "Logger.h"
#include "ptreeWrapper.h"

#pragma comment(lib, "shlwapi.lib")

extern HMODULE g_hModule;
HHOOK	g_hMouseHook;
HWND	g_hwndExProperty;
const std::unordered_set<std::wstring> kNoWheelList = { L"Edit", L"msctls_trackbar32", L"ComboBox" };

constexpr LPCSTR kConfigFileName = "PropertyWindowFixerPluginConfig.ini";

std::string	LogFileName()
{
	char logFilePath[MAX_PATH];
	::GetModuleFileNameA(g_hModule, logFilePath, MAX_PATH);
	::PathRemoveFileSpecA(logFilePath);
	::PathAppendA(logFilePath, "PropertyWindowFixerPlugin.log");
	return logFilePath;
}

fs::path GetExeDirectory()
{
	WCHAR exePath[MAX_PATH] = L"";
	GetModuleFileName(g_hModule, exePath, MAX_PATH);
	fs::path exeFolder = exePath;
	return exeFolder.parent_path();
}

LRESULT CALLBACK GetMsgProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (nCode < 0)  // do not process the message 
		return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);

	auto pMsg = (LPMSG)lParam;	
	if (pMsg->message == WM_MOUSEWHEEL) {
		if (::IsChild(g_hwndExProperty, pMsg->hwnd)) {
			HWND hwndFocus = ::GetFocus();
			if (hwndFocus != pMsg->hwnd) {
				WCHAR className[64] = L"";
				GetClassName(pMsg->hwnd, className, 64);
				//INFO_LOG << L"GetMsgProc className: " << className;
				if (kNoWheelList.find(className) != kNoWheelList.end()) {
					//INFO_LOG << L"switch hwnd ";
					pMsg->hwnd = g_hwndExProperty;
				}
			}
		}
	}

	return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
}


LRESULT CALLBACK MySubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto pThis = (CScrollConteinerView*)uIdSubclass;

	_ATL_MSG msg(hWnd, uMsg, wParam, lParam);
	pThis->m_pCurrentMsg = &msg;

	LRESULT lResult = 0;
	BOOL processed = pThis->ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult);
	if (processed) {
		return lResult;
	} else {
		return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
}

///////////////////////////////////////////////////////////////////
// CScrollConteinerView

CScrollConteinerView::CScrollConteinerView() : m_bDebug(false)
	, m_rcFixedWindow(CPoint(), CSize(530, 550)), m_bPropertyChanging(false), m_bPropertyChanged(false), m_bManualShowHideFilter(false), m_bNoCorrectMenuPositon(false)
{
}

BOOL CScrollConteinerView::SubclassWindow()
{
	CWindow hwnd = _FindExPropertyWindow();
	if (hwnd == NULL) {
		::MessageBox(NULL, L"�ݒ�_�C�A���O��������܂���\n�g���ҏW�̐ݒ�_�C�A���O��\���������\n�ēx�ݒ�_�C�A���O��ʃT�C�Y�Œ艻�v���O�C����\�����Ă��������B", L"PropertyWindowFixerPlugin �G���[", MB_ICONERROR);
		return FALSE;
	}
	hwnd.ModifyStyle(0, WS_THICKFRAME | WS_CLIPCHILDREN);
	::SetClassLong(hwnd, GCL_STYLE, CS_DBLCLKS | CS_BYTEALIGNWINDOW);

	//hwnd.ModifyStyleEx(0, WS_EX_COMPOSITED);
	//BOOL bb = hwnd.ShowScrollBar(SB_VERT);
	//BOOL b = __super::SubclassWindow(hwnd);	
	m_pfnDefWndProc = ::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
	BOOL b = ::SetWindowSubclass(hwnd, MySubclassProc, (UINT_PTR)this, 0);
	ATLASSERT(b);
	m_hWnd = hwnd;
	m_pfnSubWndProc = ::GetWindowLongPtr(hwnd, GWLP_WNDPROC);

	{
		auto ptree = ptreeWrapper::LoadIniPtree(kConfigFileName);
		CRect rcWindow;
		rcWindow.top = ptree.get<int>(L"Window.top", 0);
		rcWindow.left = ptree.get<int>(L"Window.left", 0);
		rcWindow.right = ptree.get<int>(L"Window.right", 0);
		rcWindow.bottom = ptree.get<int>(L"Window.bottom", 0);
		if (rcWindow != CRect()) {
			// �E�B���h�E�ʒu�ƃT�C�Y�𕜌�����
			m_rcFixedWindow = rcWindow;
			SetTimer(kDelayRestoreWindowId, kDelayRestoreWindowInterval);
		}

		m_cyScrollLine = ptree.get<int>(L"Config.cyScrollLine", m_cyScrollLine);
		m_cyMargin = ptree.get<int>(L"Config.cyMargin", m_cyMargin);
		m_bAlwaysRestoreScrollPos = ptree.get<bool>(L"Config.bAlwaysRestoreScrollPos", m_bAlwaysRestoreScrollPos);

		//bEnableIPC = ptree.get<bool>(L"Config.bEnableIPC", true);
	}

	CRect rcClient;
	hwnd.GetClientRect(&rcClient);
	SetScrollSize(rcClient.Width(), rcClient.Height());
	SetScrollLine(kcxScrollLine, m_cyScrollLine);

	PostMessage(WM_DELAY_CALCANDSETSCROLLSIZE);

	INFO_LOG << L"SubclassWindow : scrollsize cx : " << rcClient.Width() << L" cy: " << rcClient.Height();


	HWND hwndRightEdgePart = GetDlgItem(kEdgeRightHeaderPartId);
	ATLASSERT(hwndRightEdgePart);
	const CPoint ptHeaderRightBottom = _GetWindowRelativePositon(hwndRightEdgePart).BottomRight();
	m_barBottom = _GetWindowRelativePositon(hwndRightEdgePart).bottom + kEvalcxMargin;

	HWND hwndChild = ::GetWindow(m_hWnd, GW_CHILD);
	do {
		const CRect rcPart = _GetWindowRelativePositon(hwndChild);
		const CPoint partRightBottom = rcPart.BottomRight();
		if (partRightBottom.x <= (ptHeaderRightBottom.x + kEvalcxMargin) && 
			partRightBottom.y <= (ptHeaderRightBottom.y + kEvalcxMargin)) {
			m_defaultHeaderPartPosition.emplace_back(hwndChild, rcPart);
		}
	} while (hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT));
	
	// �R���{�{�b�N�X��G�f�B�b�g�{�b�N�X�ɃX�N���[����D���Ȃ��悤�ɂ���
	g_hwndExProperty = hwnd;
	g_hMouseHook = ::SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, NULL, ::GetCurrentThreadId());
	ATLASSERT(g_hMouseHook);

	return b;
}


void CScrollConteinerView::UnsubclassWindow()
{
	::UnhookWindowsHookEx(g_hMouseHook);
	g_hMouseHook = NULL;

	CRect rc;
	GetWindowRect(&rc);

	{
		auto ptree = ptreeWrapper::LoadIniPtree(kConfigFileName);
		ptree.put(L"Window.top", rc.top);
		ptree.put(L"Window.left", rc.left);
		ptree.put(L"Window.right", rc.right);
		ptree.put(L"Window.bottom", rc.bottom);

		bool success = ptreeWrapper::SaveIniPtree(kConfigFileName, ptree);
	}

	ShowScrollBar(SB_BOTH, FALSE);
	ModifyStyle(WS_THICKFRAME | WS_CLIPCHILDREN, 0);

	SetScrollOffset(0, 0);

	BOOL b = ::RemoveWindowSubclass(m_hWnd, MySubclassProc, (UINT_PTR)this);
	ATLASSERT(b);
	//CWindow hwnd = __super::UnsubclassWindow();
	CWindow hwnd = m_hWnd;
	m_hWnd = NULL;
	const int scrollbarWidth = ::GetSystemMetrics(SM_CXVSCROLL);
	rc.right = rc.left + m_sizeAll.cx + scrollbarWidth;
	rc.bottom = rc.top + m_sizeAll.cy + scrollbarWidth - m_cyMargin;
	hwnd.MoveWindow(&rc);
}


void CScrollConteinerView::DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine)
{
	HWND hwndScrollBar = (HWND)m_pCurrentMsg->lParam;
	if (IsChild(hwndScrollBar)) {
		DefWindowProc();
	} else {
		__super::DoScroll(nType, nScrollCode, cxyOffset, cxySizeAll, cxySizePage, cxySizeLine);
	}
	return ;
}

LRESULT CScrollConteinerView::DefWindowProc()
{
	const _ATL_MSG* pMsg = m_pCurrentMsg;
	LRESULT lRes = 0;
	if (pMsg != NULL) {
		lRes = DefWindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
	}
	return lRes;
}

LRESULT CScrollConteinerView::DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const _ATL_MSG* pMsg = m_pCurrentMsg;
	LRESULT lRes = 0;
	if (pMsg != NULL) {
		lRes = ::DefSubclassProc(pMsg->hwnd, uMsg, wParam, lParam);
	}
	return lRes;
}



int CScrollConteinerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
#if 0
	WCHAR debugFilePath[MAX_PATH];
	::GetModuleFileName(g_hModule, debugFilePath, MAX_PATH);
	::PathRemoveFileSpec(debugFilePath);
	::PathAppend(debugFilePath, L"PropertyWindowFixerPluginDebug");
	m_bDebug = ::PathFileExists(debugFilePath) != 0;
#endif
	return 0;
}

void CScrollConteinerView::OnDestroy()
{
	UnsubclassWindow();
}

void CScrollConteinerView::OnClose()
{
	SetScrollOffset(0, 0);
	DefWindowProc();
}

void CScrollConteinerView::OnPaint(CDCHandle dc)
{
	if (!m_bPropertyChanging && m_ptOffset.y < m_barBottom) {
		DefWindowProc();
	} else {
		CPaintDC dc(m_hWnd);
		RECT rc;
		GetClientRect(&rc);
		dc.FillSolidRect(&rc, GetSysColor(COLOR_BTNFACE));
	}
}

void CScrollConteinerView::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case kDelaySwitchManualShowHideTimerId:
		KillTimer(kDelaySwitchManualShowHideTimerId);

		if (!m_bPropertyChanging) {
			m_bManualShowHideFilter = false;
		}
		break;

	case kDelayRestoreWindowId:
		MoveWindow(&m_rcFixedWindow);
		KillTimer(kDelayRestoreWindowId);
		break;

	case kDelayPropertyChanged2Id:
		KillTimer(kDelayPropertyChanged2Id);

		{
			m_bPropertyChanging = false;

			{	// �p�[�c�̃f�t�H���g�ʒu��ۑ�
				m_prevPartPosition.clear();
				HWND hwndChild = ::GetWindow(m_hWnd, GW_CHILD);
				do {
					const CRect rcPart = _GetWindowRelativePositon(hwndChild);
					if (::IsWindowVisible(hwndChild)) {
						m_prevPartPosition.emplace_back(hwndChild, rcPart);
					}
				} while (hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT));
			}

			// ��̐�������`�悳����
			CRect rcBar = { 0, 0, 1000, m_barBottom };
			InvalidateRect(rcBar, FALSE);

			if (m_bManualShowHideFilter || m_bAlwaysRestoreScrollPos) {
				SetScrollOffset(m_ptLastOffset, TRUE);
				RedrawWindow(nullptr, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);

				m_bManualShowHideFilter = false;
			}
		}
		break;

	default:
		break;
	}
}

LRESULT CScrollConteinerView::OnFilterPlus(WORD, WORD, HWND, BOOL&)
{
	m_bNoCorrectMenuPositon = true;
	return DefWindowProc();
}

// �ݒ�_�C�A���O�͉�ʂ��X�N���[������Ă��邱�Ƃ�m��Ȃ��̂ŁA
// �������t�B���^�[�̉E�N���b�N���j���[�o�����߂ɃI�t�Z�b�g��������
LRESULT CScrollConteinerView::OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	m_bManualShowHideFilter = true;
	int xPos = LOWORD(lParam);
	int yPos = HIWORD(lParam);
	LRESULT ret = DefWindowProc(WM_RBUTTONDOWN, wParam, MAKELPARAM(xPos + m_ptOffset.x, yPos + m_ptOffset.y));
	SetTimer(kDelaySwitchManualShowHideTimerId, kDelaySwitchManualShowHideTimerInterval);
	return ret;
}

LRESULT CScrollConteinerView::OnInitMenu(UINT, WPARAM, LPARAM, BOOL&)
{
	PostMessage(WM_DELAY_CORRECTMENUPOSITION);
	return 0;
}

// ���̂܂܂ł̓I�t�Z�b�g�������\���ʒu�������̂ŁA�J�[�\���̈ʒu�Ƀ��j���[���ړ�������
LRESULT CScrollConteinerView::OnDelayCorrectMenuPosition(UINT, WPARAM, LPARAM, BOOL&)
{
	if (m_bNoCorrectMenuPositon) {
		m_bNoCorrectMenuPositon = false;
		return 0;
	}

	HWND hwnd = ::FindWindow(L"#32768", NULL);
	ATLASSERT(hwnd);
	if (hwnd == NULL) {
		return 0;
	}

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);

	CRect rcMenu;
	::GetWindowRect(hwnd, &rcMenu);

	HMONITOR hMonitor = ::MonitorFromPoint(ptCursor, MONITOR_DEFAULTTONEAREST);
	MONITORINFO	mni = { sizeof(mni) };
	GetMonitorInfo(hMonitor, &mni);
	if (mni.rcWork.bottom < (ptCursor.y + rcMenu.Height())) {
		// ���j���[�̒ꂪ��ʉ����т��ꍇ�A���j���[�𔽓]�����Ă�����
		ptCursor.y -= rcMenu.Height();

		if (ptCursor.y < mni.rcWork.top) {
			// �������]�������j���[�����j�^�[�̏�𒴂���Ȃ�A���j�^�[�̏���ړ��̏���ɂ���
			ptCursor.y = mni.rcWork.top;
		}
	}

	::SetWindowPos(hwnd, NULL, rcMenu.left, ptCursor.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	return 0;
}

// �g���ҏW���ł̃v���p�e�B�E�B���h�E�̃T�C�Y������}�~���邽�߂ɁAWM_WINDOWPOSCHANGING���b�Z�[�W������肷��
void CScrollConteinerView::OnWindowPosChanging(LPWINDOWPOS lpWndPos)
{
	//ATLTRACE(L"OnWindowPosChanging cx: %d cy: %d\n", lpWndPos->cx, lpWndPos->cy);

	// �E�B���h�E�T�C�Y�ύX��
	if ((lpWndPos->flags & SWP_NOSIZE) == 0) {
		INFO_LOG << L"OnWindowPosChanging cx: " << lpWndPos->cx << L" cy: " << lpWndPos->cy;
		m_bPropertyChanged = true;

		if (m_bPropertyChanging) {
			if (!GetDlgItem(kEdgeRightHeaderPartId).IsWindowVisible()) {
				// �������t�B���^�[��Ctrl+Z�ŕ��A�������ꍇ�A�q�E�B���h�E��������̂ł��̑΍�
				PostMessage(WM_DELAY_CALCANDSETSCROLLSIZE);
			} else {
				_CalcAndChangeScrollSize();
			}
		}

		// �E�B���h�E�T�C�Y�Œ艻
		lpWndPos->cx = m_rcFixedWindow.Width();
		lpWndPos->cy = m_rcFixedWindow.Height();

		if (lpWndPos->cx != m_sizeClient.cx || lpWndPos->cy != m_sizeClient.cy) {

		}
	}
}

void CScrollConteinerView::OnWindowPosChanged(LPWINDOWPOS lpWndPos)
{
	INFO_LOG << L"OnWindowPosChanged cx: " << lpWndPos->cx << L" cy: " << lpWndPos->cy;

	m_bPropertyChanged = false;

	if (lpWndPos->cx != m_sizeClient.cx || lpWndPos->cy != m_sizeClient.cy) {
		// �X�N���[���o�[�̍ĕ`��̂���
		CRect rc;
		GetWindowRect(&rc);
		DoSize(rc.Width(), rc.Height());

		// CS_VREDRAW���O�����̂Ń��T�C�Y���Ɏq�E�B���h�E�̕`����w�����Ȃ���΂Ȃ�Ȃ�
		RedrawWindow(nullptr, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}

void CScrollConteinerView::OnPropertyWindowCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	enum { kStartFoldButtonId = 0x1194, kEndFoldButtonId = 0x119E };
	if (kStartFoldButtonId <= nID && nID <= kEndFoldButtonId) {
		//INFO_LOG << L"�� clicked";
		//if (::GetKeyState(VK_CONTROL) < 0) {
			//SetMsgHandled(FALSE);
			m_bPropertyChanging = true;
			m_bManualShowHideFilter = true;
			DefWindowProc();
			m_bPropertyChanging = false;
		//}
		return;
	}
	SetMsgHandled(FALSE);
}

// �蓮�ł̃E�B���h�E���T�C�Y���͋�����
void CScrollConteinerView::OnSizing(UINT fwSide, LPRECT pRect)
{
	m_rcFixedWindow = *pRect;
}

void CScrollConteinerView::OnSize(UINT nType, CSize size)
{
	INFO_LOG << L"OnSize nType: " << nType << L" cx:" << size.cx << L" cy: " << size.cy;
	DoSize(size.cx, size.cy);
}

LRESULT CScrollConteinerView::OnSetText(UINT, WPARAM, LPARAM, BOOL&)
{
	INFO_LOG << L"OnSetText";
	DefWindowProc();

	m_bPropertyChanging = true;
	PostMessage(WM_DELAY_PROPERTYCHANGED);

	// �X�N���v�g���ёւ��v���O�O�C���ɂ�� 
	// �ݒ�_�C�A���O�ւ̃��[�U�[�t�B���^�h���b�v�ɂ���āA
	// �T�u�N���X�����������ꂽ��A�T�u�N���X���𕜋A�����鏈��
	std::thread([this]() {
		enum { kSleepInterval = 100, kMaxRetryCount = 10 };
		for (int i = 0; i < kMaxRetryCount; ++i) {
			LONG_PTR pfnWndProc = ::GetWindowLongPtr(m_hWnd, GWLP_WNDPROC);
			if (m_pfnDefWndProc == pfnWndProc) {	// �f�t�H���g(�I���W�i��)��WindowProc�֖߂��Ă��܂���
				INFO_LOG << L"Restore subclass";
				::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pfnSubWndProc);	// restore
				break;
			}
			::Sleep(kSleepInterval);
		}
		}).detach();

	return LRESULT();
}

LRESULT CScrollConteinerView::OnDelayPropertyChanged(UINT, WPARAM, LPARAM, BOOL&)
{
	INFO_LOG << L"OnDelayPropertyChanged";

	if (m_bPropertyChanged) {
		// OnWindowPosChanging�̌�� OnWindowPosChanged���Ă΂�Ă��Ȃ�
		// ����I�u�W�F�N�g�̒��ԓ_�؂�ւ����ȂǂŁA�g���ҏW�ɂ���Đݒ�_�C�A���O���̃p�[�c�ʒu�����Z�b�g����Ȃ��̂ŁA�����Ń��Z�b�g���s��
		for (const auto& defaultPart : m_prevPartPosition) {
			CWindow(defaultPart.first).MoveWindow(defaultPart.second, FALSE);
		}
	}

	//m_bPropertyChanging = false;

	//SetScrollOffset(m_ptLastOffset);
	// 
	//std::thread([this]() {
	//	::Sleep(3 * 1000);
	//	if (m_bManualShowHideFilter || m_bAlwaysRestoreScrollPos) {
	//		SetScrollOffset(m_ptLastOffset, FALSE);
	//		m_bManualShowHideFilter = false;
	//	}
	//	}).detach();
	SetTimer(kDelayPropertyChanged2Id, kDelayPropertyChanged2Interval);
	//if (m_bManualShowHideFilter || m_bAlwaysRestoreScrollPos) {
	//	SetScrollOffset(m_ptLastOffset, FALSE);
	//	m_bManualShowHideFilter = false;
	//}

	//PostMessage(WM_DELAY_INVALIDATERECT);

	return 0;
}

// UpdateWindow���Ȃ��悤�ɕύX
LRESULT CScrollConteinerView::OnPropertyMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLASSERT(::IsWindow(m_hWnd));

	int zDelta = (int)GET_WHEEL_DELTA_WPARAM(wParam);
	int nScrollCode = (m_nWheelLines == WHEEL_PAGESCROLL) ? ((zDelta > 0) ? SB_PAGEUP : SB_PAGEDOWN) : ((zDelta > 0) ? SB_LINEUP : SB_LINEDOWN);
	m_zDelta += zDelta;   // cumulative
	int zTotal = (m_nWheelLines == WHEEL_PAGESCROLL) ? abs(m_zDelta) : abs(m_zDelta) * m_nWheelLines;
	if (m_sizeAll.cy > m_sizeClient.cy)
	{
		for (int i = 0; i < zTotal; i += WHEEL_DELTA)
		{
			DoScroll(SB_VERT, nScrollCode, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
			//pT->UpdateWindow();
		}
	} else if (m_sizeAll.cx > m_sizeClient.cx)   // can't scroll vertically, scroll horizontally
	{
		for (int i = 0; i < zTotal; i += WHEEL_DELTA)
		{
			DoScroll(SB_HORZ, nScrollCode, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
			//pT->UpdateWindow();
		}
	}
	m_zDelta %= WHEEL_DELTA;

	return 0;
}

LRESULT CScrollConteinerView::OnDelayInvalidateRect(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
#if 0
	m_propertyWindow.Invalidate();
	if (m_bDebug) {
		Invalidate();
		UpdateWindow();
		m_propertyWindow.UpdateWindow();
	}
#endif
	INFO_LOG << L"OnDelayInvalidateRect";
	SetScrollOffset(m_ptOffset.x, m_ptOffset.y, TRUE);
	RedrawWindow(nullptr, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);

	return 0;
}


HWND CScrollConteinerView::_FindExPropertyWindow()
{
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
	return hwnd;
}

CRect CScrollConteinerView::_GetWindowRelativePositon(HWND hwndChild)
{
	CRect rc;
	::GetWindowRect(hwndChild, &rc);
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rc, 2);
	return rc;
}

void CScrollConteinerView::_CalcAndChangeScrollSize()
{
	CSize maxSize(100, 100);
	::EnumChildWindows(m_hWnd, [](HWND hwnd, LPARAM lParam) -> BOOL {
		if (!CWindow(hwnd).IsWindowVisible()) {
			return TRUE;
		}
		WCHAR className[64];
		::GetClassName(hwnd, className, 64);
		if (::lstrcmp(className, L"Static") == 0) {
			return TRUE;
		}
		CRect rcWindow;
		CWindow(hwnd).GetWindowRect(&rcWindow);
		::MapWindowPoints(HWND_DESKTOP, ::GetParent(hwnd), (LPPOINT)&rcWindow, 2);
		CSize& maxSize = *(CSize*)lParam;
		maxSize.cx = max(maxSize.cx, rcWindow.right);
		maxSize.cy = max(maxSize.cy, rcWindow.bottom);
		return TRUE;
		}, (LPARAM)&maxSize);
	maxSize.cy += m_cyMargin;

	// ResetOffset��true�ɂ��邪�A�p�[�c�E�B���h�E�̈ʒu�͊g���ҏW���̂����Z�b�g����̂�
	// �ꎞ�I��SetScrollSize�� �p�[�c�E�B���h�E�̈ʒu���ړ������Ȃ��悤�ɂ���	

	// ���݂̃I�t�Z�b�g�ʒu���L�����Ă����āA�g���ҏW���p�[�c�E�B���h�E�̈ʒu�����Z�b�g���I������^�C�~���O�ŁA
	// �X�N���[�������邱�Ƃɂ���āA�^���I��ResetOffset��false�̏�Ԃ��Č�����

	for (const auto& defaultPart : m_defaultHeaderPartPosition) {
		CWindow(defaultPart.first).MoveWindow(defaultPart.second, FALSE);
	}
	m_ptLastOffset = m_ptOffset;
	m_dwExtendedStyle &= ~SCRL_SCROLLCHILDREN;
	SetScrollSize(maxSize.cx, maxSize.cy, FALSE, true);
	SetScrollLine(kcxScrollLine, m_cyScrollLine);
	m_dwExtendedStyle |= SCRL_SCROLLCHILDREN;
	INFO_LOG << L"_CalcAndChangeScrollSize maxSize cx: " << maxSize.cx << L" cy: " << maxSize.cy;

	PostMessage(WM_DELAY_INVALIDATERECT);
}

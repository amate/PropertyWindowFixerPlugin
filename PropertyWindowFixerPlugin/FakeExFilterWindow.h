#pragma once

#include <atlctrls.h>
#include <atlcrack.h>


class CFakeExFilterWindow : public CWindowImpl<CFakeExFilterWindow>
{
public:
	DECLARE_WND_CLASS(_T("ExtendedFilterClass"))

	BEGIN_MSG_MAP_EX(CFakeExFilterWindow)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		enum { kEditControlId = 0x5655 };
		CRect rcEdit = { 0, 0, 400, 200 };
		m_mainEdit.Create(m_hWnd, rcEdit, nullptr,
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_WANTRETURN, WS_EX_CLIENTEDGE, kEditControlId);
		m_mainEdit.SetWindowText(L"(<?-- きりたん 差分追加.psd\r\n"
			L"\r\n"
			L"o = { --オプション設定\r\n"
			L"lipsync = 9    ,--口パク準備のレイヤー番号\r\n"
			L"mpslider = 9    ,--多目的スライダーのレイヤー番号\r\n"
			L"scene = 0    ,--シーン番号\r\n"
			L"tag = 144970057    ,--識別用タグ\r\n"
			L"\r\n"
			L"-- 口パク準備のデフォルト設定\r\n"
			L"ls_locut = 100    ,--ローカット\r\n"
			L"ls_hicut = 1000    ,--ハイカット\r\n"
			L"ls_threshold = 100 ,--しきい値\r\n"
			L"ls_sensitivity = 1    ,--感度\r\n"
			L"\r\n"
			L"-- 以下は書き換えないでください\r\n"
			L"ptkf = \"E:\\\\ゲーム録画\\\\鶏肉_きりたん\\\\きりたん 差分追加.psd\",ptkl = \"L.1 V._dEA2A0A_wECAP9ABgD9Mo4A\" }PSD, subobj = require(\"PSDToolKit\").PSDState.init(obj, o) ? > )\r\n");
		char temp[512] = "";
		GetWindowTextA(m_mainEdit, temp, 512);
		return 0;
	}

	void OnDestroy()
	{
		m_mainEdit.DestroyWindow();
	}

	CEdit	m_mainEdit;
};


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
		m_mainEdit.SetWindowText(L"(<?-- ���肽�� �����ǉ�.psd\r\n"
			L"\r\n"
			L"o = { --�I�v�V�����ݒ�\r\n"
			L"lipsync = 9    ,--���p�N�����̃��C���[�ԍ�\r\n"
			L"mpslider = 9    ,--���ړI�X���C�_�[�̃��C���[�ԍ�\r\n"
			L"scene = 0    ,--�V�[���ԍ�\r\n"
			L"tag = 144970057    ,--���ʗp�^�O\r\n"
			L"\r\n"
			L"-- ���p�N�����̃f�t�H���g�ݒ�\r\n"
			L"ls_locut = 100    ,--���[�J�b�g\r\n"
			L"ls_hicut = 1000    ,--�n�C�J�b�g\r\n"
			L"ls_threshold = 100 ,--�������l\r\n"
			L"ls_sensitivity = 1    ,--���x\r\n"
			L"\r\n"
			L"-- �ȉ��͏��������Ȃ��ł�������\r\n"
			L"ptkf = \"E:\\\\�Q�[���^��\\\\�{��_���肽��\\\\���肽�� �����ǉ�.psd\",ptkl = \"L.1 V._dEA2A0A_wECAP9ABgD9Mo4A\" }PSD, subobj = require(\"PSDToolKit\").PSDState.init(obj, o) ? > )\r\n");
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


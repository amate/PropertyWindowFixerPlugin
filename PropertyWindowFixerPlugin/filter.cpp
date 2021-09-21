//----------------------------------------------------------------------------------
//		�T���v���ҏW�v���O�C��(�t�B���^�v���O�C��)  for AviUtl ver0.99i�ȍ~
//----------------------------------------------------------------------------------

#include "pch.h"
#include <windows.h>
#include "filter.h"

#include "ScrollContainerView.h"

#define PLUGIN_NAME "�ݒ�_�C�A���O��ʃT�C�Y�Œ艻�v���O�C��"
#define PLUGIN_VERSION "2.3"


#define	WINDOW_W		540
#define	WINDOW_H		560

CScrollConteinerView	g_scrollContainerView;


//---------------------------------------------------------------------
//		�t�B���^�\���̒�`
//---------------------------------------------------------------------
FILTER_DLL filter = {
	FILTER_FLAG_DISP_FILTER
	//| FILTER_FLAG_WINDOW_HSCROLL
	| FILTER_FLAG_WINDOW_THICKFRAME // �T�C�Y�ύX�\�ȃE�B���h�E�����܂�
	| FILTER_FLAG_ALWAYS_ACTIVE
	| FILTER_FLAG_WINDOW_SIZE
	| FILTER_FLAG_PRIORITY_LOWEST
	| FILTER_FLAG_EX_INFORMATION,
	WINDOW_W,WINDOW_H,
	PLUGIN_NAME,
	NULL,NULL,NULL,
	NULL,NULL,
	NULL,NULL,NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	func_WndProc,
	NULL,NULL,
	NULL,
	NULL,
	PLUGIN_NAME " version " PLUGIN_VERSION " by amate",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};


//---------------------------------------------------------------------
//		�t�B���^�\���̂̃|�C���^��n���֐�
//---------------------------------------------------------------------
EXTERN_C FILTER_DLL __declspec(dllexport)* __stdcall GetFilterTable(void)
{
	return &filter;
}


void UpdateLayout(HWND hwnd)
{
	if (g_scrollContainerView.IsWindow()) {
		RECT rect = { 0 };
		::GetClientRect(hwnd, &rect);
		// resize client window
		::SetWindowPos(g_scrollContainerView, NULL, rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

//---------------------------------------------------------------------
//		WndProc
//---------------------------------------------------------------------
#define	COPY_MODE_VIDEO	0
#define	COPY_MODE_AUDIO	1
#define	COPY_MODE_ALL	2
BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, FILTER* fp)
{
	//	TRUE��Ԃ��ƑS�̂��ĕ`�悳���

	switch (message) {
	case WM_FILTER_CHANGE_WINDOW:
	{
		bool isDisp = fp->exfunc->is_filter_window_disp(fp) != 0;
		if (isDisp) {
			::ShowWindow(hwnd, SW_HIDE);
			g_scrollContainerView.SubclassWindow();

			//UpdateLayout(hwnd);

		} else {
			if (g_scrollContainerView.IsWindow()) {
				//g_scrollContainerView.DestroyWindow();
				g_scrollContainerView.UnsubclassWindow();
			}
		}

	}
		break;

	case WM_SIZE:
		//UpdateLayout(hwnd);
		break;


	case WM_FILTER_INIT:	// �J�n����ɑ����܂�
		break;

	case WM_FILTER_EXIT:	// �I�����O�ɑ����܂�
		if (g_scrollContainerView.IsWindow()) {
			//g_scrollContainerView.DestroyWindow();
			g_scrollContainerView.UnsubclassWindow();
		}
		break;

	}

	return FALSE;
}



#pragma once
#include <Windows.h>

class WinApp
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	// 初期化
	void Initialize();
	//更新
	void Update();
	HWND GetHwnd() const { return hwnd; } // 追加：HWNDを取得するためのメソッド
	MSG& GetMsg() { return msg_; } // msg へのアクセス用
	WNDCLASS& GetWndClass() { return wc_; } // wc へのアクセス用
private:
	HWND hwnd = nullptr;
	MSG msg_{};
	WNDCLASS wc_{};
};


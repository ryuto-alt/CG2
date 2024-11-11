#pragma once
#include <Windows.h>

class WinApp
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

public:
	// 初期化
	void Initialize();
	//更新
	void Update();
	HWND GetHwnd() const { return hwnd; } // 追加：HWNDを取得するためのメソッド

	HINSTANCE GetInstance() const {return wc.hInstance}

private:
	HWND hwnd = nullptr;
	MSG msg_{};
	WNDCLASS wc{};
};


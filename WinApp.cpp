#include "WinApp.h"

#include<cstdint>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include "externals/imgui/imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
	{
		return true;
	}

	//メッセージに応じてゲーム固有の処理を行う
	switch (msg)
	{
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::Initialize()
{
    const int32_t kClientWidth = 1280;
    const int32_t kClientHeight = 720;
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

    // ウィンドウクラスを設定
    wc_.lpfnWndProc = WindowProc;
    wc_.lpszClassName = L"CG2WindowClass";
    wc_.hInstance = GetModuleHandle(nullptr);
    wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc_);

    RECT wrc = { 0, 0, kClientWidth, kClientHeight };
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    hwnd = CreateWindow(
        wc_.lpszClassName,
        L"CG2",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        wrc.right - wrc.left,
        wrc.bottom - wrc.top,
        nullptr,
        nullptr,
        wc_.hInstance,
        nullptr
    );

    ShowWindow(hwnd, SW_SHOW);
}

void WinApp::Update()
{
}

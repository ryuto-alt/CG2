#include <Windows.h>
#include <cassert>
#include "WinApp.h"
#include "DirectXCommon.h"

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	// ウィンドウアプリケーションの初期化
	WinApp* winApp = new WinApp;
	winApp->Initialize();

	// DirectX基盤部分の初期化を行うクラス
	DirectXCommon* dxCommon = new DirectXCommon;
	dxCommon->Initialize(winApp);








	// メインループ
	MSG msg = {};
	while (true) {
		if (winApp->ProcessMessage()) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			break;
		}
		dxCommon->Begin(); // 描画開始


		// - シーンの更新
		// - オブジェクトの描画
		// - 入力の処理
		// - UIの描画

		dxCommon->PreDraw();

		dxCommon->PostDraw();

		dxCommon->End();   // 描画終了
	}

	// リソースの解放
	delete dxCommon;
	delete winApp;
	CoUninitialize();

	return 0;
}
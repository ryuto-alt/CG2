#include <windows.h>
#include "WinApp.h"
#include "MyGame.h"
#include "D3DResourceCheck.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // リソースリーク検出用
    D3DResourceLeakChecker leakCheck;

    // COM初期化
    CoInitializeEx(0, COINIT_MULTITHREADED);

    // WindowsAPIの初期化
    WinApp* winApp = new WinApp();
    winApp->Initialize();

    // ゲームの作成と初期化
    MyGame* game = new MyGame();
    game->Initialize(winApp);

    // メインループ
    while (!game->IsEndRequested()) {
        // ゲームの更新と描画
        game->Update();
        game->Draw();
    }

    // ゲームの終了処理
    delete game;

    // WindowsAPIの終了処理
    winApp->Finalize();
    delete winApp;

    // COM終了処理
    CoUninitialize();

    return 0;
}
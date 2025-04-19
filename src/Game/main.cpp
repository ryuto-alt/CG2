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

    // ゲームの作成
    MyGame* game = new MyGame();

    // WinAppを設定
    game->SetWinApp(winApp);

    // ゲームのメインループを実行
    // これにより、Initialize -> (Update/Draw のループ) -> Finalize の流れが実行される
    game->Run();

    // ゲームの解放
    delete game;

    // WindowsAPIの終了処理
    winApp->Finalize();
    delete winApp;

    // COM終了処理
    CoUninitialize();

    return 0;
}
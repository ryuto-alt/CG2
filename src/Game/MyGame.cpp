// MyGame.cpp（修正版）
#include "MyGame.h"
#include "D3DResourceCheck.h"
#include <string>
#include <algorithm>

MyGame::MyGame()
    : winApp_(nullptr)
    , dxCommon_(nullptr)
    , input_(nullptr)
    , spriteCommon_(nullptr)
    , srvManager_(nullptr)
    , camera_(nullptr)
    , sceneManager_(nullptr)
    , sceneFactory_(nullptr) {
}

MyGame::~MyGame() {
    // Framework::Finalizeが呼ばれるので、ここでは追加の処理は不要
}

void MyGame::Initialize() {
    // WinAppが設定されていることを確認
    assert(winApp_ != nullptr);

    // DirectXCommonの初期化
    dxCommon_ = new DirectXCommon();
    dxCommon_->Initialize(winApp_);

    // SRVマネージャの初期化
    srvManager_ = new SrvManager();
    srvManager_->Initialize(dxCommon_);

    // テクスチャマネージャの初期化
    TextureManager::GetInstance()->Initialize(dxCommon_, srvManager_);

    // ImGuiの初期化
    InitializeImGui();

    // 入力初期化
    input_ = new Input();
    input_->Initialize(winApp_);

    // スプライト共通部分の初期化
    spriteCommon_ = new SpriteCommon();
    spriteCommon_->Initialize(dxCommon_);

    // カメラの作成と初期化
    camera_ = new Camera();
    camera_->SetTranslate({ 0.0f, 0.0f, -5.0f });
    Object3dCommon::SetDefaultCamera(camera_);

    // シーンファクトリーの作成
    sceneFactory_ = new GameSceneFactory();

    // シーンマネージャーの作成と初期化
    sceneManager_ = SceneManager::GetInstance();
    sceneManager_->SetDirectXCommon(dxCommon_);
    sceneManager_->SetInput(input_);
    sceneManager_->SetSpriteCommon(spriteCommon_);
    sceneManager_->SetSrvManager(srvManager_);
    sceneManager_->SetCamera(camera_);
    sceneManager_->SetWinApp(winApp_);
    sceneManager_->Initialize(sceneFactory_);
}

void MyGame::InitializeImGui() {
    // ImGui初期化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(winApp_->GetHwnd());

    // SrvManagerのディスクリプタヒープを使用
    ImGui_ImplDX12_Init(
        dxCommon_->GetDevice(),
        2, // SwapChainのバッファ数
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        srvManager_->GetDescriptorHeap().Get(),
        srvManager_->GetCPUDescriptorHandle(0), // ImGui用に0番を使用
        srvManager_->GetGPUDescriptorHandle(0)
    );
}

void MyGame::Update() {
    // Windowsのメッセージ処理
    if (winApp_->ProcessMessage()) {
        endRequest_ = true;
        return;
    }

    // 入力更新
    input_->Update();

    // シーンマネージャーの更新
    sceneManager_->Update();
}

void MyGame::Draw() {
    // DirectXの描画準備
    dxCommon_->Begin();

    // シーンマネージャーの描画
    sceneManager_->Draw();

    // 描画終了
    dxCommon_->End();
}

void MyGame::Finalize() {
    // シーンマネージャーの終了処理
    sceneManager_->Finalize();

    // シーンファクトリーの解放
    delete sceneFactory_;

    // ImGuiの解放
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // テクスチャマネージャの解放
    TextureManager::GetInstance()->Finalize();

    // カメラの解放
    delete camera_;

    // リソースの解放
    delete spriteCommon_;
    delete input_;
    delete srvManager_;
    delete dxCommon_;

    // winAppはmain.cppで解放するため、ここでは解放しない
}
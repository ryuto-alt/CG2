// UnoEngine.cpp
// 自作エンジンの機能を統合した実装ファイル
#include "UnoEngine.h"

namespace Uno {

    // 静的メンバ変数の初期化
    UnoEngine* UnoEngine::instance_ = nullptr;

    UnoEngine* UnoEngine::GetInstance() {
        if (!instance_) {
            instance_ = new UnoEngine();
        }
        return instance_;
    }

    void UnoEngine::Initialize(HINSTANCE hInstance) {
        // COM初期化
        CoInitializeEx(0, COINIT_MULTITHREADED);

        // WinAppの初期化
        winApp_ = std::make_unique<WinApp>();
        winApp_->Initialize();

        // DirectXCommonの初期化
        dxCommon_ = std::make_unique<DirectXCommon>();
        dxCommon_->Initialize(winApp_.get());

        // SRVマネージャの初期化
        srvManager_ = std::make_unique<SrvManager>();
        srvManager_->Initialize(dxCommon_.get());

        // テクスチャマネージャの初期化
        TextureManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());

        // デフォルトテクスチャの事前読み込み
        TextureManager::GetInstance()->LoadDefaultTexture();

        // ImGuiの初期化
        InitializeImGui();

        // 入力初期化
        input_ = std::make_unique<Input>();
        input_->Initialize(winApp_.get());

        // スプライト共通部分の初期化
        spriteCommon_ = std::make_unique<SpriteCommon>();
        spriteCommon_->Initialize(dxCommon_.get());

        // カメラの作成と初期化
        camera_ = std::make_unique<Camera>();
        camera_->SetTranslate({ 0.0f, 0.0f, -5.0f });
        Object3dCommon::SetDefaultCamera(camera_.get());

        // オーディオマネージャの初期化
        AudioManager::GetInstance()->Initialize();

        // パーティクルマネージャの初期化
        ParticleManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());

        // 基本的なパーティクルグループの作成
        ParticleManager::GetInstance()->CreateParticleGroup("smoke", "Resources/particle/smoke.png");

        // デバッグ出力
        OutputDebugStringA("UnoEngine: Successfully initialized\n");
    }

    void UnoEngine::InitializeImGui() {
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

        // デバッグ出力
        OutputDebugStringA("UnoEngine: ImGui initialized successfully\n");
    }

    void UnoEngine::Finalize() {
        // 基本的なマネージャークラスの終了処理
        ParticleManager::GetInstance()->Finalize();
        AudioManager::GetInstance()->Finalize();
        TextureManager::GetInstance()->Finalize();

        // ImGuiの解放
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        // 各リソースはunique_ptrにより自動的に解放される
        camera_.reset();
        spriteCommon_.reset();
        input_.reset();
        srvManager_.reset();
        dxCommon_.reset();
        winApp_->Finalize();
        winApp_.reset();

        // COM終了処理
        CoUninitialize();

        // シングルトンインスタンスの解放
        delete instance_;
        instance_ = nullptr;

        // デバッグ出力
        OutputDebugStringA("UnoEngine: Successfully finalized\n");
    }

    void UnoEngine::BeginFrame() {
        // SRVヒープを設定
        srvManager_->PreDraw();

        // 入力の更新
        input_->Update();

        // カメラの更新
        camera_->Update();

        // パーティクルマネージャの更新
        ParticleManager::GetInstance()->Update(camera_.get());

        // オーディオマネージャの更新
        AudioManager::GetInstance()->Update();

        // DirectXの描画準備
        dxCommon_->Begin();

        // ImGuiの新しいフレーム開始
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void UnoEngine::EndFrame() {
        // ImGuiの描画
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());

        // DirectXの描画終了
        dxCommon_->End();
    }

    bool UnoEngine::ProcessMessage() const {
        // Windowsのメッセージ処理を実行
        // trueが返された場合は終了要求があったことを示す
        return winApp_->ProcessMessage();
    }

} // namespace Uno
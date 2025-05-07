#include "UnoEngine.h"
#include "D3DResourceCheck.h"
#include <string>
#include <cassert>

// 静的メンバ変数の初期化
UnoEngine* UnoEngine::instance_ = nullptr;

UnoEngine::UnoEngine()
    : winApp_(nullptr),
    dxCommon_(nullptr),
    input_(nullptr),
    spriteCommon_(nullptr),
    srvManager_(nullptr),
    camera_(nullptr),
    sceneManager_(nullptr),
    sceneFactory_(nullptr),
    isInitialized_(false) {
}

UnoEngine::~UnoEngine() {
    // Finalizeメソッドで解放するため、ここでは何もしない
}

UnoEngine* UnoEngine::GetInstance() {
    if (!instance_) {
        instance_ = new UnoEngine();
    }
    return instance_;
}

void UnoEngine::Initialize(WinApp* winApp) {
    // 既に初期化済みの場合は何もしない
    if (isInitialized_) {
        return;
    }

    try {
        // WinAppの設定
        winApp_ = winApp;
        assert(winApp_ != nullptr);

        // DirectXCommonの初期化
        dxCommon_ = std::make_unique<DirectXCommon>();
        dxCommon_->Initialize(winApp_);

        // SRVマネージャの初期化
        srvManager_ = std::make_unique<SrvManager>();
        srvManager_->Initialize(dxCommon_.get());

        // ここで明示的にPreDrawを呼び出し、ディスクリプタヒープを設定
        srvManager_->PreDraw();

        // テクスチャマネージャの初期化
        TextureManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());

        // デフォルトテクスチャの事前読み込み
        TextureManager::GetInstance()->LoadDefaultTexture();

        // ImGuiの初期化
        InitializeImGui();

        // 入力初期化
        input_ = std::make_unique<Input>();
        input_->Initialize(winApp_);

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
        InitializeParticle();

        // シーンマネージャーの取得
        sceneManager_ = SceneManager::GetInstance();
        sceneManager_->SetDirectXCommon(dxCommon_.get());
        sceneManager_->SetInput(input_.get());
        sceneManager_->SetSpriteCommon(spriteCommon_.get());
        sceneManager_->SetSrvManager(srvManager_.get());
        sceneManager_->SetCamera(camera_.get());
        sceneManager_->SetWinApp(winApp_);

        // 初期化完了フラグを設定
        isInitialized_ = true;

        // デバッグ出力
        OutputDebugStringA("UnoEngine: Successfully initialized\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::Initialize: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::InitializeImGui() {
    try {
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
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Failed to initialize ImGui: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::InitializeParticle() {
    try {
        // パーティクルマネージャの初期化
        ParticleManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());

        // 基本的なパーティクルグループの作成
        ParticleManager::GetInstance()->CreateParticleGroup("smoke", "Resources/particle/smoke.png");
        ParticleManager::GetInstance()->CreateParticleGroup("fire", "Resources/particle/fire.png");
        ParticleManager::GetInstance()->CreateParticleGroup("star", "Resources/particle/star.png");

        // デバッグ出力
        OutputDebugStringA("UnoEngine: Particle system initialized successfully\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Failed to initialize particle system: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::SetSceneFactory(SceneFactory* sceneFactory) {
    sceneFactory_ = sceneFactory;
    
    // SceneFactoryが設定された後にSceneManagerを初期化
    if (sceneManager_ && sceneFactory_) {
        sceneManager_->Initialize(sceneFactory_);
    }
}

void UnoEngine::Update() {
    try {
        // 入力更新
        input_->Update();

        // SRVヒープを描画前に明示的に設定
        if (srvManager_) {
            srvManager_->PreDraw();
        }

        // オーディオマネージャの更新
        AudioManager::GetInstance()->Update();

        // パーティクルマネージャの更新
        ParticleManager::GetInstance()->Update(camera_.get());

        // シーンマネージャーの更新
        if (sceneManager_) {
            sceneManager_->Update();
        }
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::Update: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::BeginFrame() {
    try {
        // ImGuiの新規フレーム開始
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // DirectXの描画準備
        dxCommon_->Begin();

        // SRVヒープを描画前に明示的に設定
        if (srvManager_) {
            srvManager_->PreDraw();
        }
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::BeginFrame: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::EndFrame() {
    try {
        // ImGuiの描画
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());

        // 描画終了
        dxCommon_->End();
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::EndFrame: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::ChangeScene(const std::string& sceneName) {
    if (sceneManager_) {
        sceneManager_->ChangeScene(sceneName);
    }
}

bool UnoEngine::ProcessMessage() {
    if (winApp_) {
        return winApp_->ProcessMessage();
    }
    return false;
}

void UnoEngine::Finalize() {
    try {
        // シーンマネージャーの終了処理
        if (sceneManager_) {
            sceneManager_->Finalize();
            // シングルトンなのでここではnullptrにするだけ
            sceneManager_ = nullptr;
        }

        // パーティクルマネージャーの終了処理
        ParticleManager::GetInstance()->Finalize();

        // オーディオマネージャーの終了処理
        AudioManager::GetInstance()->Finalize();

        // ImGuiの解放
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        // テクスチャマネージャの解放
        TextureManager::GetInstance()->Finalize();

        // 各リソースはunique_ptrにより自動的に解放される
        // 明示的にnullptrを設定
        camera_.reset();
        spriteCommon_.reset();
        input_.reset();
        srvManager_.reset();
        dxCommon_.reset();

        // winAppはmain.cppで解放するため、ここでは解放しない
        winApp_ = nullptr;
        sceneFactory_ = nullptr;
        
        // 初期化フラグを下げる
        isInitialized_ = false;

        // インスタンスの解放
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }

        // デバッグ出力
        OutputDebugStringA("UnoEngine: Successfully finalized\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::Finalize: " + std::string(e.what()) + "\n").c_str());
    }
}

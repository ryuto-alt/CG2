#include "UnoEngine.h"

UnoEngine* UnoEngine::GetInstance() {
    static UnoEngine instance;
    return &instance;
}

void UnoEngine::Initialize() {
    // WinAppの初期化
    winApp_ = std::make_unique<WinApp>();
    winApp_->Initialize();

    // DirectXCommonの初期化
    dxCommon_ = std::make_unique<DirectXCommon>();
    dxCommon_->Initialize(winApp_.get());

    // 入力の初期化
    input_ = std::make_unique<Input>();
    input_->Initialize(winApp_.get()); // HInstanceとhwndを渡すのではなく、WinAppポインタを渡す

    // テクスチャマネージャーはシングルトンパターンを使用するので、GetInstance()でアクセスする
    // SrvManagerを先に初期化しておく必要がある
    srvManager_ = std::make_unique<SrvManager>();
    srvManager_->Initialize(dxCommon_.get());
    
    // TextureManagerの初期化
    TextureManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());

    // オーディオマネージャーはシングルトンパターンを使用するので、GetInstance()でアクセスする
    AudioManager::GetInstance()->Initialize();

    // 物理マネージャーはシングルトンパターンを使用するので、unique_ptrではなく、GetInstance()でアクセスする
    // デストラクタがprivateなので、unique_ptrで管理することができない
}

void UnoEngine::Update() {
    // 入力の更新
    input_->Update();

    // 物理シミュレーションの更新
    PhysicsManager::GetInstance()->Update();
}

void UnoEngine::PreDraw() {
    // 描画前処理
    dxCommon_->Begin();
}

void UnoEngine::PostDraw() {
    // 描画後処理
    dxCommon_->End();
}

void UnoEngine::Finalize() {
    // 各マネージャーの終了処理
    // physicsManagerには明示的なFinalizeメソッドはない
    AudioManager::GetInstance()->Finalize();
    // srvManager_にはFinalizeメソッドはない
    TextureManager::GetInstance()->Finalize();
    input_->Finalize();
    // dxCommon_にはFinalizeメソッドはない
    winApp_->Finalize();
}

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
    , blueStarEmitter_(nullptr)
    , greenStarEmitter_(nullptr)
    , purpleStarEmitter_(nullptr)
    , cameraSpeed_(0.1f)
    , mouseSensitivity_(0.0008f)
    , showMouseCursor_(false)
    , bgmLoaded_(false)
    , seLoaded_(false)
    , mp3Loaded_(false)
    , masterVolume_(1.0f)
    , bgmVolume_(1.0f)
    , seVolume_(1.0f)
    , mp3Volume_(1.0f) {
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

    // AudioManagerの初期化
    AudioManager::GetInstance()->Initialize();

    // サウンドファイルの読み込み
    bgmLoaded_ = AudioManager::GetInstance()->LoadWAV("bgm", "resources/audio/bgm.wav");
    seLoaded_ = AudioManager::GetInstance()->LoadWAV("se_shot", "resources/audio/se_shot.wav");
    mp3Loaded_ = AudioManager::GetInstance()->LoadMP3("music", "resources/audio/music.mp3");

    // パーティクルマネージャの初期化
    ParticleManager::GetInstance()->Initialize(dxCommon_, srvManager_);

    // パーティクルグループの作成
    ParticleManager::GetInstance()->CreateParticleGroup("star", "resources/particle/star.png");
    ParticleManager::GetInstance()->CreateParticleGroup("star_green", "resources/particle/star.png");
    ParticleManager::GetInstance()->CreateParticleGroup("star_purple", "resources/particle/star.png");

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

    // パーティクルエミッタの初期化
    InitializeParticleEmitters();

    // マウスカーソルを非表示に設定
    input_->SetMouseCursor(showMouseCursor_);
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

void MyGame::InitializeParticleEmitters() {
    // 青色系の星パーティクル
    blueStarEmitter_ = new ParticleEmitter(
        "star",  // 元のグループ名
        { -1.0f, 0.5f, -2.0f },   // 固定位置
        2,                        // 一度に発生する数
        0.3f,                     // 発生頻度
        { -1.0f, -1.0f, -1.0f },  // 最小速度
        { 1.0f, 1.0f, 1.0f },     // 最大速度
        { 0.0f, -0.5f, 0.0f },    // 最小加速度
        { 0.0f, -0.3f, 0.0f },    // 最大加速度
        4.0f,                     // 最小開始サイズ
        6.0f,                     // 最大開始サイズ
        1.0f,                     // 最小終了サイズ
        2.0f,                     // 最大終了サイズ
        { 0.0f, 0.5f, 1.0f, 1.0f },  // 最小開始色（青色）
        { 0.5f, 0.7f, 1.0f, 1.0f },  // 最大開始色（水色）
        { 0.0f, 1.0f, 1.0f, 0.0f },  // 最小終了色（シアン、透明）
        { 0.5f, 1.0f, 1.0f, 0.0f },  // 最大終了色（水色、透明）
        0.0f,                        // 最小開始回転
        3.14f * 2.0f,                // 最大開始回転
        -1.0f,                       // 最小回転速度
        1.0f,                        // 最大回転速度
        0.5f,                        // 最小生存時間
        1.0f                         // 最大生存時間
    );

    // 緑色系の星パーティクル - 横方向の動きを強調
    greenStarEmitter_ = new ParticleEmitter(
        "star_green",  // 新しいグループ名
        { 0.0f, 0.5f, -2.0f },    // 固定位置（中央）
        2,                        // 一度に発生する数
        0.4f,                     // 発生頻度
        { -1.5f, 0.1f, -0.2f },   // 最小速度 - 横方向に強い
        { 1.5f, 0.8f, 0.2f },     // 最大速度 - 横方向に強い
        { 0.0f, 0.1f, 0.0f },     // 最小加速度 - わずかに上向き
        { 0.0f, 0.3f, 0.0f },     // 最大加速度 - わずかに上向き
        3.0f,                     // 最小開始サイズ
        5.0f,                     // 最大開始サイズ
        0.5f,                     // 最小終了サイズ
        1.5f,                     // 最大終了サイズ
        { 0.0f, 0.8f, 0.2f, 1.0f },  // 最小開始色（緑色）
        { 0.2f, 1.0f, 0.5f, 1.0f },  // 最大開始色（明るい緑）
        { 0.1f, 0.7f, 0.3f, 0.0f },  // 最小終了色（緑、透明）
        { 0.3f, 1.0f, 0.6f, 0.0f },  // 最大終了色（明るい緑、透明）
        0.0f,                        // 最小開始回転
        3.14f * 2.0f,                // 最大開始回転
        -2.0f,                       // 最小回転速度 - 速い回転
        2.0f,                        // 最大回転速度 - 速い回転
        0.6f,                        // 最小生存時間
        1.2f                         // 最大生存時間
    );

    // 紫色系の星パーティクル - 爆発的な動き
    purpleStarEmitter_ = new ParticleEmitter(
        "star_purple",  // 新しいグループ名
        { 1.0f, 0.5f, -2.0f },    // 固定位置（右側）
        3,                        // 一度に発生する数
        0.6f,                     // 発生頻度
        { -2.0f, -2.0f, -2.0f },  // 最小速度 - 全方向に強い
        { 2.0f, 2.0f, 2.0f },     // 最大速度 - 全方向に強い
        { 0.0f, -0.2f, 0.0f },    // 最小加速度 - わずかに下向き
        { 0.0f, -0.1f, 0.0f },    // 最大加速度 - わずかに下向き
        2.0f,                     // 最小開始サイズ
        4.0f,                     // 最大開始サイズ
        0.2f,                     // 最小終了サイズ - 小さく消えていく
        0.5f,                     // 最大終了サイズ - 小さく消えていく
        { 0.8f, 0.2f, 1.0f, 1.0f },  // 最小開始色（紫色）
        { 1.0f, 0.4f, 1.0f, 1.0f },  // 最大開始色（明るい紫）
        { 0.6f, 0.0f, 0.8f, 0.0f },  // 最小終了色（暗い紫、透明）
        { 1.0f, 0.2f, 1.0f, 0.0f },  // 最大終了色（紫、透明）
        0.0f,                        // 最小開始回転
        3.14f * 2.0f,                // 最大開始回転
        -3.0f,                       // 最小回転速度 - 非常に速い回転
        3.0f,                        // 最大回転速度 - 非常に速い回転
        0.4f,                        // 最小生存時間 - 短め
        0.8f                         // 最大生存時間 - 短め
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

    // ImGui開始
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // ImGuiウィンドウの描画
    DrawImGui();

    // カメラ操作
    ControlCamera();

    // エミッタの更新
    blueStarEmitter_->Update();
    greenStarEmitter_->Update();
    purpleStarEmitter_->Update();

    // パーティクルマネージャの更新
    ParticleManager::GetInstance()->Update(camera_);

    // オーディオマネージャの更新
    AudioManager::GetInstance()->Update();
}

void MyGame::DrawImGui() {
    // カメラ設定ウィンドウ
    ImGui::Begin("Camera Settings");

    // カメラ操作UI
    ImGui::Text("Camera Controls");
    ImGui::Text("WASD: Move in the direction you're facing");
    ImGui::Text("Mouse: Look around");
    ImGui::Text("SPACE: Move Up | SHIFT: Move Down");
    ImGui::Text("ESC: Toggle mouse cursor visibility");

    // マウスカーソル状態の表示とチェックボックスで切り替え
    if (ImGui::Checkbox("Show Mouse Cursor", &showMouseCursor_)) {
        input_->SetMouseCursor(showMouseCursor_);

        if (!showMouseCursor_) {
            // マウスカーソルを非表示にした場合、マウスを中央に戻す
            POINT center;
            center.x = WinApp::kClientWidth / 2;
            center.y = WinApp::kClientHeight / 2;
            ClientToScreen(winApp_->GetHwnd(), &center);
            SetCursorPos(center.x, center.y);
        }
    }

    // マウス感度設定
    ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity_, 0.001f, 0.01f);

    // カメラ設定
    Vector3 cameraPos = camera_->GetTranslate();
    Vector3 cameraRot = camera_->GetRotate();

    if (ImGui::DragFloat3("Camera Position", &cameraPos.x, 0.1f)) {
        camera_->SetTranslate(cameraPos);
    }

    if (ImGui::DragFloat3("Camera Rotation", &cameraRot.x, 0.01f)) {
        camera_->SetRotate(cameraRot);
    }

    float fovY = camera_->GetFovY();
    if (ImGui::SliderFloat("Field of View", &fovY, 0.1f, 1.5f)) {
        camera_->SetFovY(fovY);
    }

    ImGui::End();

    // パーティクル設定用GUI
    ImGui::Begin("Particle Settings");

    // 青色の星エミッタの設定
    ImGui::Text("Blue Star Emitter");
    Vector3 bluePos = blueStarEmitter_->GetPosition();
    if (ImGui::DragFloat3("Blue Position", &bluePos.x, 0.1f)) {
        blueStarEmitter_->SetPosition(bluePos);
    }
    bool blueEmitting = blueStarEmitter_->IsEmitting();
    if (ImGui::Checkbox("Blue Emitting", &blueEmitting)) {
        blueStarEmitter_->SetEmitting(blueEmitting);
    }
    uint32_t blueCount = blueStarEmitter_->GetEmitCount();
    if (ImGui::DragInt("Blue Count", (int*)&blueCount, 1, 1, 10)) {
        blueStarEmitter_->SetEmitCount(blueCount);
    }
    float blueRate = blueStarEmitter_->GetEmitRate();
    if (ImGui::DragFloat("Blue Rate", &blueRate, 0.1f, 0.1f, 5.0f)) {
        blueStarEmitter_->SetEmitRate(blueRate);
    }

    // 緑色の星エミッタの設定
    ImGui::Text("Green Star Emitter");
    Vector3 greenPos = greenStarEmitter_->GetPosition();
    if (ImGui::DragFloat3("Green Position", &greenPos.x, 0.1f)) {
        greenStarEmitter_->SetPosition(greenPos);
    }
    bool greenEmitting = greenStarEmitter_->IsEmitting();
    if (ImGui::Checkbox("Green Emitting", &greenEmitting)) {
        greenStarEmitter_->SetEmitting(greenEmitting);
    }
    uint32_t greenCount = greenStarEmitter_->GetEmitCount();
    if (ImGui::DragInt("Green Count", (int*)&greenCount, 1, 1, 10)) {
        greenStarEmitter_->SetEmitCount(greenCount);
    }
    float greenRate = greenStarEmitter_->GetEmitRate();
    if (ImGui::DragFloat("Green Rate", &greenRate, 0.1f, 0.1f, 5.0f)) {
        greenStarEmitter_->SetEmitRate(greenRate);
    }

    // 紫色の星エミッタの設定
    ImGui::Text("Purple Star Emitter");
    Vector3 purplePos = purpleStarEmitter_->GetPosition();
    if (ImGui::DragFloat3("Purple Position", &purplePos.x, 0.1f)) {
        purpleStarEmitter_->SetPosition(purplePos);
    }
    bool purpleEmitting = purpleStarEmitter_->IsEmitting();
    if (ImGui::Checkbox("Purple Emitting", &purpleEmitting)) {
        purpleStarEmitter_->SetEmitting(purpleEmitting);
    }
    uint32_t purpleCount = purpleStarEmitter_->GetEmitCount();
    if (ImGui::DragInt("Purple Count", (int*)&purpleCount, 1, 1, 10)) {
        purpleStarEmitter_->SetEmitCount(purpleCount);
    }
    float purpleRate = purpleStarEmitter_->GetEmitRate();
    if (ImGui::DragFloat("Purple Rate", &purpleRate, 0.1f, 0.1f, 5.0f)) {
        purpleStarEmitter_->SetEmitRate(purpleRate);
    }

    // デバッグ情報の表示
    ImGui::Separator();
    ImGui::Text("Debug Info");
    ImGui::Text("Camera Position: %.2f, %.2f, %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
    ImGui::Text("Blue Star Position: %.2f, %.2f, %.2f", bluePos.x, bluePos.y, bluePos.z);
    ImGui::Text("Green Star Position: %.2f, %.2f, %.2f", greenPos.x, greenPos.y, greenPos.z);
    ImGui::Text("Purple Star Position: %.2f, %.2f, %.2f", purplePos.x, purplePos.y, purplePos.z);
    ImGui::Text("Blue Star Particles: %d", ParticleManager::GetInstance()->GetParticleCount("star"));
    ImGui::Text("Green Star Particles: %d", ParticleManager::GetInstance()->GetParticleCount("star_green"));
    ImGui::Text("Purple Star Particles: %d", ParticleManager::GetInstance()->GetParticleCount("star_purple"));

    // デバッグ用ボタン - シンプルな四角形描画テスト
    if (ImGui::Button("Draw Simple Quad Test")) {
        // 次のフレームで単純な四角形を描画するフラグを立てる
        ParticleManager::GetInstance()->DrawSimpleQuad();
    }

    ImGui::End();

    // オーディオ設定用GUI
    ImGui::Begin("Audio Settings");

    // BGM再生/停止ボタン
    if (bgmLoaded_) {
        if (ImGui::Button("Play BGM")) {
            AudioManager::GetInstance()->Play("bgm", true); // ループ再生
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop BGM")) {
            AudioManager::GetInstance()->Stop("bgm");
        }
    }

    // 効果音再生ボタン
    if (seLoaded_) {
        if (ImGui::Button("Play SE")) {
            AudioManager::GetInstance()->Play("se_shot", false); // 一回再生
        }
    }

    // MP3再生/停止ボタン（読み込みが成功した場合のみ表示）
    if (mp3Loaded_) {
        if (ImGui::Button("Play MP3")) {
            AudioManager::GetInstance()->Play("music", true); // ループ再生
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop MP3")) {
            AudioManager::GetInstance()->Stop("music");
        }
    }

    // マスターボリューム設定スライダー
    if (ImGui::SliderFloat("Master Volume", &masterVolume_, 0.0f, 1.0f)) {
        AudioManager::GetInstance()->SetMasterVolume(masterVolume_);
    }

    // BGMボリューム設定スライダー
    if (bgmLoaded_) {
        if (ImGui::SliderFloat("BGM Volume", &bgmVolume_, 0.0f, 1.0f)) {
            AudioManager::GetInstance()->SetVolume("bgm", bgmVolume_);
        }
    }

    // SE（効果音）ボリューム設定スライダー
    if (seLoaded_) {
        if (ImGui::SliderFloat("SE Volume", &seVolume_, 0.0f, 1.0f)) {
            AudioManager::GetInstance()->SetVolume("se_shot", seVolume_);
        }
    }

    // MP3ボリューム設定スライダー（読み込みが成功した場合のみ表示）
    if (mp3Loaded_) {
        if (ImGui::SliderFloat("MP3 Volume", &mp3Volume_, 0.0f, 1.0f)) {
            AudioManager::GetInstance()->SetVolume("music", mp3Volume_);
        }
    }

    ImGui::End();
}

void MyGame::ControlCamera() {
    // マウス入力の取得とカメラ回転
    DIMOUSESTATE mouseState;
    if (SUCCEEDED(input_->GetMouseState(&mouseState)) && !showMouseCursor_) {
        // マウスの移動量を回転に変換
        Vector3 rot = camera_->GetRotate();
        rot.x += mouseState.lY * mouseSensitivity_; // マウスY移動→X軸回転(上下)
        rot.y += mouseState.lX * mouseSensitivity_; // マウスX移動→Y軸回転(左右)

        // 上下の視点移動を制限（-89°～89°）
        if (rot.x > 1.55f) rot.x = 1.55f;
        if (rot.x < -1.55f) rot.x = -1.55f;

        camera_->SetRotate(rot);

        // マウスを中央に戻す
        POINT center;
        center.x = WinApp::kClientWidth / 2;
        center.y = WinApp::kClientHeight / 2;
        ClientToScreen(winApp_->GetHwnd(), &center);
        SetCursorPos(center.x, center.y);
    }

    // カメラの向きベクトルを計算
    Vector3 rot = camera_->GetRotate();
    Vector3 forward = {
        sinf(rot.y),
        0.0f,
        cosf(rot.y)
    };
    Vector3 right = {
        cosf(rot.y),
        0.0f,
        -sinf(rot.y)
    };

    // 正規化（単位ベクトル化）
    float length = sqrtf(forward.x * forward.x + forward.z * forward.z);
    forward.x /= length;
    forward.z /= length;

    length = sqrtf(right.x * right.x + right.z * right.z);
    right.x /= length;
    right.z /= length;

    // キーボードによるカメラ操作（方向ベクトルに沿った移動）
    Vector3 pos = camera_->GetTranslate();
    if (input_->PushKey(DIK_W)) {
        pos.x += forward.x * cameraSpeed_;
        pos.z += forward.z * cameraSpeed_;
    }
    if (input_->PushKey(DIK_S)) {
        pos.x -= forward.x * cameraSpeed_;
        pos.z -= forward.z * cameraSpeed_;
    }
    if (input_->PushKey(DIK_A)) {
        pos.x -= right.x * cameraSpeed_;
        pos.z -= right.z * cameraSpeed_;
    }
    if (input_->PushKey(DIK_D)) {
        pos.x += right.x * cameraSpeed_;
        pos.z += right.z * cameraSpeed_;
    }
    // 上下移動の追加
    if (input_->PushKey(DIK_SPACE)) {
        pos.y += cameraSpeed_; // 上に移動
    }
    if (input_->PushKey(DIK_LSHIFT)) {
        pos.y -= cameraSpeed_; // 下に移動
    }

    camera_->SetTranslate(pos);

    // マウスカーソル表示切替
    if (input_->TriggerKey(DIK_ESCAPE)) { // ESCキーでマウスカーソル表示切替
        showMouseCursor_ = !showMouseCursor_;
        input_->SetMouseCursor(showMouseCursor_);

        if (!showMouseCursor_) {
            // マウスを中央に戻す
            POINT center;
            center.x = WinApp::kClientWidth / 2;
            center.y = WinApp::kClientHeight / 2;
            ClientToScreen(winApp_->GetHwnd(), &center);
            SetCursorPos(center.x, center.y);
        }
    }
}

void MyGame::Draw() {
    // DirectXの描画準備
    dxCommon_->Begin();

    // SRVヒープのセット
    srvManager_->PreDraw();

    // カメラの更新
    camera_->Update();

    // パーティクルの描画
    ParticleManager::GetInstance()->Draw();

    // ImGuiの描画
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());

    // 描画終了
    dxCommon_->End();
}

void MyGame::Finalize() {
    // ImGuiの解放
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // パーティクルエミッタの解放
    delete blueStarEmitter_;
    delete greenStarEmitter_;
    delete purpleStarEmitter_;
    ParticleManager::GetInstance()->Finalize();

    // オーディオマネージャの解放
    AudioManager::GetInstance()->Finalize();

    // カメラの解放
    delete camera_;

    // 終了処理
    TextureManager::GetInstance()->Finalize();

    // リソースの解放
    delete spriteCommon_;
    delete input_;
    delete srvManager_;
    delete dxCommon_;

    // winAppはmain.cppで解放するため、ここでは解放しない
}
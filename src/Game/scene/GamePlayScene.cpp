#include "GamePlayScene.h"
#include "SceneManager.h"
#include <cassert>

GamePlayScene::GamePlayScene() {
    // コンストラクタでオブジェクトを作成
    player_ = std::make_unique<Player>();
    ground_ = std::make_unique<Ground>();
    playerModel_ = std::make_unique<Model>();
    groundModel_ = std::make_unique<Model>();
}

GamePlayScene::~GamePlayScene() {
    // デストラクタでは特に処理はない
}

void GamePlayScene::Initialize() {
    // 必須のリソースチェック
    assert(dxCommon_);
    assert(input_);
    assert(camera_);
    assert(spriteCommon_);
    assert(srvManager_);

    // カメラの初期位置設定
    cameraDistance_ = 10.0f;
    cameraHeight_ = 5.0f;
    camera_->SetTranslate({ 0.0f, cameraHeight_, -cameraDistance_ });
    camera_->SetTarget({ 0.0f, 0.0f, 0.0f });

    // 初期状態ではマウスカーソルを非表示に設定
    showCursor_ = false;
    input_->SetMouseCursor(showCursor_);
    OutputDebugStringA("GamePlayScene: Mouse cursor hidden\n");

    // デバッグ情報を追加
    OutputDebugStringA("モデル初期化開始\n");
    
    // モデルの初期化
    playerModel_->Initialize(dxCommon_);
    groundModel_->Initialize(dxCommon_);
    
    OutputDebugStringA("モデル読み込み開始\n");
    
    try {
        // モデルの読み込み
        playerModel_->LoadFromObj("Resources/Models/", "sphere.obj");
        OutputDebugStringA("プレイヤーモデル読み込み成功\n");
        
        groundModel_->LoadFromObj("Resources/Models/ground", "ground.obj");
        OutputDebugStringA("地面モデル読み込み成功\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA("モデル読み込みエラー: ");
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
    }

    // プレイヤーの初期化
    player_->Initialize(dxCommon_, spriteCommon_, playerModel_.get(), input_);
    
    // 地面の初期化
    ground_->Initialize(dxCommon_, spriteCommon_, groundModel_.get());
    
    // 地面の位置とスケールを設定
    ground_->SetPosition({ 0.0f, -1.5f, 0.0f }); // 位置を少し下げて、プレイヤーが地面に正しく着地するようにする
    ground_->SetScale({ 1.0f, 1.0f, 1.0f }); // 地面を大きくしてまた落ちないようにする

    // プレイヤーの初期位置を設定
    player_->SetPosition({ 0.0f, 5.0f, 0.0f });

    // 物理マネージャーの取得
    physicsManager_ = PhysicsManager::GetInstance();

    // 初期化完了
    initialized_ = true;
}

void GamePlayScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // 基本的なカメラ操作
    ControlCamera();

    // カメラの更新
    camera_->Update();

    // 物理マネージャーの更新
    physicsManager_->Update();

    // 地面の更新
    ground_->Update();

    // プレイヤーの更新
    player_->Update();

    // ESCキーでタイトルシーンへ戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        // マウスカーソルを表示に戻す
        input_->SetMouseCursor(true);
        sceneManager_->ChangeScene("Title");
    }

    // TABキーでマウスカーソルの表示切替
    if (input_->TriggerKey(DIK_TAB)) {
        showCursor_ = !showCursor_;
        input_->SetMouseCursor(showCursor_);
    }
}

void GamePlayScene::ControlCamera() {
    // プレイヤーが存在する場合はプレイヤーの位置を参照
    if (player_) {
        Vector3 playerPos = player_->GetPosition();
        
        // 上下左右キーでカメラ移動
        // 上キー
        if (input_->PushKey(DIK_UP)) {
            cameraHeight_ += cameraMoveSpeed_;
        }
        // 下キー
        if (input_->PushKey(DIK_DOWN)) {
            cameraHeight_ -= cameraMoveSpeed_;
        }
        // 左キー
        if (input_->PushKey(DIK_LEFT)) {
            // 左右キーでカメラを回転
            cameraRotation_ -= cameraMoveSpeed_ * 2.0f;
        }
        // 右キー
        if (input_->PushKey(DIK_RIGHT)) {
            cameraRotation_ += cameraMoveSpeed_ * 2.0f;
        }
        
        // PageUp/PageDownキーでカメラの距離を調整
        if (input_->PushKey(DIK_PGUP)) {
            cameraDistance_ -= cameraMoveSpeed_ * 5.0f;
            if (cameraDistance_ < 2.0f) cameraDistance_ = 2.0f; // 最小距離
        }
        if (input_->PushKey(DIK_PGDN)) {
            cameraDistance_ += cameraMoveSpeed_ * 5.0f;
            if (cameraDistance_ > 50.0f) cameraDistance_ = 50.0f; // 最大距離
        }
        
        // カメラの位置を計算
        float cameraX = playerPos.x + std::sin(cameraRotation_) * cameraDistance_;
        float cameraZ = playerPos.z - std::cos(cameraRotation_) * cameraDistance_;
        Vector3 cameraPos = {
            cameraX,
            playerPos.y + cameraHeight_,
            cameraZ
        };
        
        // カメラの注視点をプレイヤーに設定
        Vector3 targetPos = playerPos;
        
        // カメラに設定を反映
        camera_->SetTranslate(cameraPos);
        camera_->SetTarget(targetPos);
    }
}

void GamePlayScene::Draw() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // 3Dオブジェクトの描画準備（SRVヒープの設定）
    srvManager_->PreDraw();
    
    // 地面の描画
    ground_->Draw();
    
    // プレイヤーの描画
    player_->Draw();

    // ImGuiの描画
    DrawImGui();
}

void GamePlayScene::DrawImGui() {
    // ImGuiの新しいフレーム開始
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    // ゲーム情報ウィンドウ
    ImGui::Begin("Game Information");
    
    // プレイヤー情報を表示
    if (player_) {
        const Vector3& playerPos = player_->GetPosition();
        const Vector3& playerVel = player_->GetVelocity();
        bool isGrounded = player_->IsGrounded();
        
        ImGui::Text("Player Position: (%.2f, %.2f, %.2f)", playerPos.x, playerPos.y, playerPos.z);
        ImGui::Text("Player Velocity: (%.2f, %.2f, %.2f)", playerVel.x, playerVel.y, playerVel.z);
        ImGui::Text("Grounded: %s", isGrounded ? "Yes" : "No");
    }
    
    // カメラ情報を表示
    if (camera_) {
        const Vector3& cameraPos = camera_->GetTranslate();
        const Vector3& targetPos = camera_->GetTarget();
        
        ImGui::Separator();
        ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
        ImGui::Text("Camera Target: (%.2f, %.2f, %.2f)", targetPos.x, targetPos.y, targetPos.z);
        
        // カメラパラメータの調整
        ImGui::Separator();
        ImGui::Text("Camera Settings:");
        ImGui::SliderFloat("Camera Speed", &cameraMoveSpeed_, 0.01f, 1.0f);
        ImGui::SliderFloat("Camera Distance", &cameraDistance_, 2.0f, 50.0f);
        ImGui::SliderFloat("Camera Height", &cameraHeight_, 0.0f, 20.0f);
        ImGui::SliderFloat("Camera Rotation", &cameraRotation_, -3.14f, 3.14f);
    }
    
    // 操作説明
    ImGui::Separator();
    ImGui::Text("Controls:");
    ImGui::Text("WASD - Move Player");
    ImGui::Text("SPACE - Jump");
    ImGui::Text("Arrow Keys - Move/Rotate Camera");
    ImGui::Text("PAGE UP/DOWN - Zoom Camera");
    ImGui::Text("TAB - Toggle Mouse Cursor");
    ImGui::Text("ESC - Return to Title");
    
    ImGui::End();

    // ImGuiの描画
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());
}

void GamePlayScene::Finalize() {
    // マウスカーソルを確実に表示状態に戻す
    if (input_) {
        showCursor_ = true;
        input_->SetMouseCursor(true);
        OutputDebugStringA("GamePlayScene: Mouse cursor restored to visible state\n");
    }
}
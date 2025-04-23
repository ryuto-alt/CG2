#include "GamePlayScene.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "ParticleManager.h"
#include <cassert>
#include <cmath>

GamePlayScene::GamePlayScene() {
    // コンストラクタでの初期化は最小限にする
}

GamePlayScene::~GamePlayScene() {
    // デストラクタでは特に何もしない
}

void GamePlayScene::Initialize() {
    try {
        // リソースのnullチェック
        assert(dxCommon_);
        assert(input_);
        assert(spriteCommon_);
        assert(srvManager_);
        assert(camera_);

        // 3Dモデルの初期化
        Initialize3DModels();

        // パーティクルの初期化
        InitializeParticles();

        // カメラの初期設定
        camera_->SetTranslate({ 0.0f, 1.7f, -5.0f }); // プレイヤーの目線の高さを設定

        // カメラの角度初期化
        cameraYaw_ = 0.0f;
        cameraPitch_ = 0.0f;

        // マウス初期位置の取得
        DIMOUSESTATE mouseState = {};
        input_->GetMouseState(&mouseState);
        prevMouseX_ = 0;
        prevMouseY_ = 0;

        // FPSモードに合わせてマウスカーソルを設定
        input_->SetMouseCursor(false);
        isFPSMode_ = true;

        // 回転角度の初期化
        yRotationAngle_ = 0.0f; // Y軸回転用の変数を初期化

        // 初期化完了フラグ
        initialized_ = true;

        // デバッグ出力
        OutputDebugStringA("GamePlayScene: Successfully initialized\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Failed to initialize GamePlayScene: " + std::string(e.what()) + "\n").c_str());
        throw; // 再スローして、上位の例外ハンドラで処理できるようにする
    }
}

void GamePlayScene::Initialize3DModels() {
    try {
        // Axisモデルの初期化
        axisModel_ = std::make_unique<Model>();
        axisModel_->Initialize(dxCommon_);
        axisModel_->LoadFromObj("Resources/models", "sphere.obj");

        // モデル読み込み後に明示的にテクスチャを確認
        std::string texturePath = axisModel_->GetTextureFilePath();
        if (!texturePath.empty() && !TextureManager::GetInstance()->IsTextureExists(texturePath)) {
            OutputDebugStringA(("GamePlayScene: Loading texture from model: " + texturePath + "\n").c_str());
            TextureManager::GetInstance()->LoadTexture(texturePath);
        }
        else if (texturePath.empty()) {
            OutputDebugStringA("GamePlayScene: Model has no texture, will use default texture\n");
        }

        // 3Dオブジェクトの初期化
        axisObject_ = std::make_unique<Object3d>();
        axisObject_->Initialize(dxCommon_, spriteCommon_);
        axisObject_->SetModel(axisModel_.get());

        // オブジェクトの初期設定
        axisObject_->SetScale({ 0.5f, 0.5f, 0.5f });
        axisObject_->SetPosition({ 0.0f, 0.0f, 0.0f });

        // ライティングを有効化
        axisObject_->SetEnableLighting(true);

        // ディレクショナルライトの設定
        DirectionalLight light;
        light.color = { 1.0f, 1.0f, 1.0f, 1.0f };
        light.direction = { 0.5f, -1.0f, 0.5f };
        light.intensity = 1.0f;
        axisObject_->SetDirectionalLight(light);

        // デバッグ出力
        OutputDebugStringA("GamePlayScene: 3D models initialized successfully\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR in Initialize3DModels: " + std::string(e.what()) + "\n").c_str());
        throw; // 再スローして、上位の例外ハンドラで処理できるようにする
    }
}

void GamePlayScene::InitializeParticles() {
    try {
        // パーティクルグループの作成（既存の場合はスキップされる）
        ParticleManager::GetInstance()->CreateParticleGroup("beam", "Resources/particle/particle.png");

        // パーティクルエミッタが既に作成されている場合は再利用
        if (!particleEmitter_) {
            // パーティクルエミッタの作成
            // 重要: 最小値と最大値を確実に正しく設定
            particleEmitter_ = std::make_unique<ParticleEmitter>(
                "beam",                // グループ名
                Vector3{ 0, 0, 0 },      // 位置（後で更新）
                10,                    // 一度に発生するパーティクル数
                5.0f,                  // 発生頻度（秒間）
                Vector3{ -0.5f, -0.5f, -0.5f }, // 最小速度（確実に最小値）
                Vector3{ 0.5f, 0.5f, 0.5f },    // 最大速度（確実に最大値）
                Vector3{ 0.0f, -0.2f, 0.0f },   // 最小加速度（y方向に少し下向き）
                Vector3{ 0.0f, 0.0f, 0.0f },    // 最大加速度（無加速）
                0.1f,                  // 最小開始サイズ
                0.3f,                  // 最大開始サイズ
                0.0f,                  // 最小終了サイズ
                0.05f,                 // 最大終了サイズ
                Vector4{ 0.5f, 0.5f, 1.0f, 0.7f }, // 最小開始色
                Vector4{ 1.0f, 1.0f, 1.0f, 1.0f }, // 最大開始色
                Vector4{ 0.0f, 0.2f, 0.8f, 0.0f }, // 最小終了色
                Vector4{ 0.2f, 0.5f, 1.0f, 0.1f }, // 最大終了色
                0.0f,                  // 最小回転角度
                6.28f,                 // 最大回転角度
                -0.3f,                 // 最小回転速度
                0.3f,                  // 最大回転速度
                0.3f,                  // 最小寿命
                0.8f                   // 最大寿命
            );

            // デフォルトではエミッタの自動発生を無効化
            particleEmitter_->SetEmitting(false);
        }
        else {
            // 既存のエミッタがある場合は位置をリセット
            particleEmitter_->SetPosition(Vector3{ 0, 0, 0 });
            particleEmitter_->SetEmitting(false);
        }

        OutputDebugStringA("GamePlayScene: Particles initialized successfully\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR in InitializeParticles: " + std::string(e.what()) + "\n").c_str());
        throw;
    }
}

void GamePlayScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) {
        OutputDebugStringA("GamePlayScene: Update called before initialization\n");
        return;
    }

    try {
        // FPS視点でのカメラ制御
        if (isFPSMode_) {
            ControlFPSCamera();
        }
        else {
            ControlCamera(); // 従来の制御
        }

        // カメラの更新
        camera_->Update();

        // パーティクルのクールダウン更新
        if (particleCooldown_ > 0.0f) {
            particleCooldown_ -= 1.0f / 60.0f; // 60FPS想定
        }

        // 左クリック検出とパーティクル発射
        DIMOUSESTATE mouseState = {};
        input_->GetMouseState(&mouseState);

        // 現在の左クリック状態を保存
        wasLeftButtonPressed_ = isLeftButtonPressed_;
        isLeftButtonPressed_ = (mouseState.rgbButtons[0] & 0x80) != 0;

        // 左クリックでパーティクル発射
        if (isLeftButtonPressed_ && particleCooldown_ <= 0.0f) {
            ShootParticle();
            particleCooldown_ = particleCooldownMax_;
        }

        // Y軸（横方向）回転の計算 - 毎フレーム回転角度を増加
        yRotationAngle_ += 0.02f; // 回転スピード調整

        // 角度が2πを超えたら0に戻す（オプション）
        if (yRotationAngle_ > 6.28f) {
            yRotationAngle_ -= 6.28f;
        }

        // モデルのY軸回転を設定
        axisObject_->SetRotation({ 0.0f, yRotationAngle_, 0.0f });

        // 3Dオブジェクトの更新
        axisObject_->Update();

        // パーティクルエミッタの更新
        particleEmitter_->Update();

        // ESCキーでタイトルシーンへ戻る
        if (input_->TriggerKey(DIK_ESCAPE)) {
            // マウスカーソルを表示に戻す
            input_->SetMouseCursor(true);
            sceneManager_->ChangeScene("Title");
        }

        // TABキーでマウスカーソルの表示切替
        if (input_->TriggerKey(DIK_TAB)) {
            // マウスカーソルの表示状態を切り替え
            showCursor_ = !showCursor_;
            input_->SetMouseCursor(showCursor_);

            // FPSモードもカーソル表示に連動させる
            isFPSMode_ = !showCursor_;
        }
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR in GamePlayScene::Update: " + std::string(e.what()) + "\n").c_str());
    }
}

void GamePlayScene::ControlCamera() {
    // WASDキーでカメラ移動
    Vector3 cameraPos = camera_->GetTranslate();

    if (input_->PushKey(DIK_W)) {
        cameraPos.z += 0.1f;
    }
    if (input_->PushKey(DIK_S)) {
        cameraPos.z -= 0.1f;
    }
    if (input_->PushKey(DIK_A)) {
        cameraPos.x -= 0.1f;
    }
    if (input_->PushKey(DIK_D)) {
        cameraPos.x += 0.1f;
    }
    if (input_->PushKey(DIK_Q)) {
        cameraPos.y += 0.1f;
    }
    if (input_->PushKey(DIK_E)) {
        cameraPos.y -= 0.1f;
    }

    camera_->SetTranslate(cameraPos);
}

void GamePlayScene::ControlFPSCamera() {
    // マウスの状態を取得
    DIMOUSESTATE mouseState = {};
    input_->GetMouseState(&mouseState);

    // マウスの移動量を計算
    int mouseX = mouseState.lX;
    int mouseY = mouseState.lY;

    // マウスの移動量に基づいて視点を回転
    cameraYaw_ += mouseX * mouseSensitivity_;
    cameraPitch_ += mouseY * mouseSensitivity_;

    // ピッチ角の制限（真上と真下を見過ぎないようにする）
    const float pitchLimit = 1.5f; // ラジアン単位（約85度）
    if (cameraPitch_ > pitchLimit) cameraPitch_ = pitchLimit;
    if (cameraPitch_ < -pitchLimit) cameraPitch_ = -pitchLimit;

    // カメラの方向ベクトルを計算
    Vector3 cameraFront;
    cameraFront.x = std::sin(cameraYaw_) * std::cos(cameraPitch_);
    cameraFront.y = std::sin(cameraPitch_);
    cameraFront.z = std::cos(cameraYaw_) * std::cos(cameraPitch_);

    // カメラの右方向ベクトルを計算
    Vector3 cameraRight;
    cameraRight.x = std::sin(cameraYaw_ - 1.57f); // 90度左回転
    cameraRight.y = 0.0f;
    cameraRight.z = std::cos(cameraYaw_ - 1.57f);

    // 現在のカメラ位置を取得
    Vector3 cameraPos = camera_->GetTranslate();

    // WASDキーで移動（カメラの向きに合わせて移動）
    if (input_->PushKey(DIK_W)) {
        cameraPos.x += cameraFront.x * cameraSpeed_;
        cameraPos.z += cameraFront.z * cameraSpeed_;
    }
    if (input_->PushKey(DIK_S)) {
        cameraPos.x -= cameraFront.x * cameraSpeed_;
        cameraPos.z -= cameraFront.z * cameraSpeed_;
    }
    if (input_->PushKey(DIK_A)) {
        cameraPos.x += cameraRight.x * cameraSpeed_;
        cameraPos.z += cameraRight.z * cameraSpeed_;
    }
    if (input_->PushKey(DIK_D)) {
        cameraPos.x -= cameraRight.x * cameraSpeed_;
        cameraPos.z -= cameraRight.z * cameraSpeed_;
    }

    // QEキーで上下移動
    if (input_->PushKey(DIK_Q)) {
        cameraPos.y += cameraSpeed_;
    }
    if (input_->PushKey(DIK_E)) {
        cameraPos.y -= cameraSpeed_;
    }

    // カメラの位置を更新
    camera_->SetTranslate(cameraPos);

    // カメラの回転を更新
    camera_->SetRotate({ cameraPitch_, cameraYaw_, 0.0f });
}

void GamePlayScene::ShootParticle() {
    // カメラの位置と回転角度を取得
    Vector3 cameraPos = camera_->GetTranslate();
    Vector3 cameraRotate = camera_->GetRotate();

    // 発射方向（カメラの前方向）を計算
    Vector3 frontDir;
    frontDir.x = std::sin(cameraRotate.y) * std::cos(cameraRotate.x);
    frontDir.y = std::sin(cameraRotate.x);
    frontDir.z = std::cos(cameraRotate.y) * std::cos(cameraRotate.x);

    // カメラの上方向を計算
    Vector3 upDir;
    upDir.x = std::sin(cameraRotate.y) * std::sin(cameraRotate.x);
    upDir.y = std::cos(cameraRotate.x);
    upDir.z = std::cos(cameraRotate.y) * std::sin(cameraRotate.x);

    // カメラの右方向を計算（前方向と上方向の外積）
    Vector3 rightDir;
    rightDir.x = std::cos(cameraRotate.y);
    rightDir.y = 0.0f;
    rightDir.z = -std::sin(cameraRotate.y);

    // 発射位置をカメラの少し前に設定
    float distanceFromCamera = 1.0f;
    Vector3 shootPosition = {
        cameraPos.x + frontDir.x * distanceFromCamera,
        cameraPos.y + frontDir.y * distanceFromCamera,
        cameraPos.z + frontDir.z * distanceFromCamera
    };

    // 基本速度の設定（カメラの前方向）
    float baseSpeed = 35.0f;
    Vector3 baseVelocity = {
        frontDir.x * baseSpeed,
        frontDir.y * baseSpeed,
        frontDir.z * baseSpeed
    };

    // 上方向へのブースト
    float upBoostAmount = 3.0f;
    Vector3 upBoost = {
        upDir.x * upBoostAmount,
        upDir.y * upBoostAmount,
        upDir.z * upBoostAmount
    };

    // 最終速度の計算（基本速度 + 上方向ブースト）
    Vector3 finalVelocity = {
        baseVelocity.x + upBoost.x,
        baseVelocity.y + upBoost.y,
        baseVelocity.z + upBoost.z
    };

    // 固定値のみを使用して最小/最大速度を設定
    Vector3 velMin = {
        finalVelocity.x - 0.5f,
        finalVelocity.y - 0.5f,
        finalVelocity.z - 0.5f
    };

    Vector3 velMax = {
        finalVelocity.x + 0.5f,
        finalVelocity.y + 0.5f,
        finalVelocity.z + 0.5f
    };

    // 重力方向（上方向の逆）
    Vector3 gravityDir = {
        -upDir.x,
        -upDir.y,
        -upDir.z
    };

    // 固定の重力強度を使用
    float gravityStrength = 5.0f; // 重力を弱めに設定

    // 最小最大加速度（重力）- 非常に小さい差異で設定
    Vector3 accelMin = {
        gravityDir.x * gravityStrength - 0.01f,
        gravityDir.y * gravityStrength - 0.01f,
        gravityDir.z * gravityStrength - 0.01f
    };

    Vector3 accelMax = {
        gravityDir.x * gravityStrength + 0.01f,
        gravityDir.y * gravityStrength + 0.01f,
        gravityDir.z * gravityStrength + 0.01f
    };

    // パーティクルを発射
    ParticleManager::GetInstance()->Emit(
        "beam",          // グループ名
        shootPosition,   // 発射位置
        20,              // パーティクル数
        velMin,          // 最小速度
        velMax,          // 最大速度
        accelMin,        // 最小加速度
        accelMax,        // 最大加速度
        0.15f,           // 最小開始サイズ
        0.25f,           // 最大開始サイズ
        0.0f,            // 最小終了サイズ
        0.05f,           // 最大終了サイズ
        { 0.7f, 0.7f, 1.0f, 1.0f }, // 最小開始色
        { 1.0f, 1.0f, 1.0f, 1.0f }, // 最大開始色
        { 0.0f, 0.2f, 0.8f, 0.0f }, // 最小終了色
        { 0.2f, 0.5f, 1.0f, 0.0f }, // 最大終了色
        0.0f,            // 最小回転角度
        6.28f,           // 最大回転角度
        -0.5f,           // 最小回転速度
        0.5f,            // 最大回転速度
        1.0f,            // 最小寿命
        1.5f             // 最大寿命
    );
}

void GamePlayScene::Draw() {
    // 初期化されていない場合は何もしない
    if (!initialized_) {
        OutputDebugStringA("GamePlayScene: Draw called before initialization\n");
        return;
    }

    try {
        // 3Dオブジェクトの描画準備（SRVヒープの設定）
        srvManager_->PreDraw();

        // 3Dオブジェクトの描画
        axisObject_->Draw();

        // 描画の順序を確認: パーティクルは最後に描画される
        // ParticleManagerの描画はMyGame.cppのDraw内で行われる
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR in GamePlayScene::Draw: " + std::string(e.what()) + "\n").c_str());
    }
}

void GamePlayScene::Finalize() {
    try {
        // マウスカーソルを表示に戻す
        input_->SetMouseCursor(true);

        // リソースの解放（必要に応じて）
        axisObject_.reset();
        axisModel_.reset();
        particleEmitter_.reset();

        // デバッグ出力
        OutputDebugStringA("GamePlayScene: Successfully finalized\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR in GamePlayScene::Finalize: " + std::string(e.what()) + "\n").c_str());
    }
}
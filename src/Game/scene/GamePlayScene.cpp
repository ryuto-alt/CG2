#include "GamePlayScene.h"
#include "SceneManager.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "TextureManager.h"
#include <cassert>

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

        // ImGuiの初期化
        InitializeImGui();

        // 3Dモデルの初期化
        Initialize3DModels();

        // カメラの初期設定
        camera_->SetTranslate({ 0.0f, 0.0f, -50.0f });

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

void GamePlayScene::InitializeImGui() {
    // ImGuiの設定（必要に応じて）
    OutputDebugStringA("GamePlayScene: ImGui initialized\n");
}

void GamePlayScene::Initialize3DModels() {
    try {
        // Axisモデルの初期化
        axisModel_ = std::make_unique<Model>();
        axisModel_->Initialize(dxCommon_);
        axisModel_->LoadFromObj("Resources/models", "dragon.obj");

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

void GamePlayScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) {
        OutputDebugStringA("GamePlayScene: Update called before initialization\n");
        return;
    }

    try {
        // カメラ制御
        ControlCamera();

        // カメラの更新
        camera_->Update();

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

        // ESCキーでタイトルシーンへ戻る
        if (input_->TriggerKey(DIK_ESCAPE)) {
            sceneManager_->ChangeScene("Title");
        }
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR in GamePlayScene::Update: " + std::string(e.what()) + "\n").c_str());
    }
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

        // ImGuiの描画
        DrawImGui();
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR in GamePlayScene::Draw: " + std::string(e.what()) + "\n").c_str());
    }
}

void GamePlayScene::DrawImGui() {
    try {
        // ImGuiの新しいフレーム開始
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // ImGuiウィンドウ
        ImGui::Begin("GamePlayScene");
        ImGui::Text("Press ESC key to return to title");
        ImGui::Text("Camera Position: %.2f, %.2f, %.2f",
            camera_->GetTranslate().x,
            camera_->GetTranslate().y,
            camera_->GetTranslate().z);
        ImGui::Text("Y Rotation Angle: %.2f", yRotationAngle_);
        ImGui::End();

        // ImGuiの描画
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR in DrawImGui: " + std::string(e.what()) + "\n").c_str());
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

void GamePlayScene::Finalize() {
    try {
        // リソースの解放（必要に応じて）
        axisObject_.reset();
        axisModel_.reset();

        // デバッグ出力
        OutputDebugStringA("GamePlayScene: Successfully finalized\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR in GamePlayScene::Finalize: " + std::string(e.what()) + "\n").c_str());
    }
}
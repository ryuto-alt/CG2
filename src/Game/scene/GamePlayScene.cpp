#include "GamePlayScene.h"
#include "SceneManager.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <cassert>

GamePlayScene::GamePlayScene() {
    // コンストラクタでの初期化は最小限にする
}

GamePlayScene::~GamePlayScene() {
    // デストラクタでは特に何もしない
}

void GamePlayScene::Initialize() {
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
    camera_->SetTranslate({ 0.0f, 0.0f, -10.0f });

    // 初期化完了フラグ
    initialized_ = true;
}

void GamePlayScene::InitializeImGui() {
    // ImGuiの設定（必要に応じて）
}

void GamePlayScene::Initialize3DModels() {
    // Axisモデルの初期化
    axisModel_ = std::make_unique<Model>();
    axisModel_->Initialize(dxCommon_);
    axisModel_->LoadFromObj("Resources/models", "sphere.obj");

    // 3Dオブジェクトの初期化
    axisObject_ = std::make_unique<Object3d>();
    axisObject_->Initialize(dxCommon_, spriteCommon_);
    axisObject_->SetModel(axisModel_.get());

    // オブジェクトの初期設定
    axisObject_->SetScale({ 1.0f, 1.0f, 1.0f });
    axisObject_->SetPosition({ 0.0f, 0.0f, 0.0f });

    // ライティングを有効化
    axisObject_->SetEnableLighting(true);

    // ディレクショナルライトの設定
    DirectionalLight light;
    light.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    light.direction = { 0.5f, -1.0f, 0.5f };
    light.intensity = 1.0f;
    axisObject_->SetDirectionalLight(light);
}

void GamePlayScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // カメラ制御
    ControlCamera();

    // カメラの更新
    camera_->Update();

    // オブジェクトの回転
    rotationAngle_ += 0.01f;
    axisObject_->SetRotation({ rotationAngle_, rotationAngle_, rotationAngle_ });

    // 3Dオブジェクトの更新
    axisObject_->Update();

    // ESCキーでタイトルシーンへ戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        sceneManager_->ChangeScene("Title");
    }
}

void GamePlayScene::Draw() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // 3Dオブジェクトの描画準備（SRVヒープの設定）
    srvManager_->PreDraw();

    // 3Dオブジェクトの描画
    axisObject_->Draw();

    // ImGuiの描画
    DrawImGui();
}

void GamePlayScene::DrawImGui() {
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
    ImGui::Text("Rotation Angle: %.2f", rotationAngle_);
    ImGui::End();

    // ImGuiの描画
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());
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
    // リソースの解放（必要に応じて）
    axisObject_.reset();
    axisModel_.reset();
}
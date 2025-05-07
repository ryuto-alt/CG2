#include "GamePlayScene.h"
#include "SceneManager.h"
#include "Model.h"
#include "Object3d.h"
#include "TextureManager.h"

GamePlayScene::GamePlayScene() {
    // UnoEngineのインスタンス取得
    engine_ = Uno::UnoEngine::GetInstance();
    
    // 初期化
    sphereModel_ = nullptr;
    sphereObject_ = nullptr;
    rotationAngle_ = 0.0f;
    modelInitialized_ = false;
}

GamePlayScene::~GamePlayScene() {
    // リソースの解放
    Finalize();
}

void GamePlayScene::Initialize() {
    // 必須のリソースチェック
    assert(input_);
    assert(camera_);

    // カメラの初期位置設定
    camera_->SetTranslate({ 0.0f, 1.0f, -5.0f });

    // マウスカーソルを非表示かつウィンドウ内に拘束
    showCursor_ = false;
    input_->SetMouseCursorConfined(showCursor_, true);
    
    // 初期化完了
    initialized_ = true;
}

bool GamePlayScene::InitializeModel() {
    // DirectXCommonポインタの取得
    DirectXCommon* dxCommon = engine_->GetDirectXCommon();
    // SpriteCommonポインタの取得
    SpriteCommon* spriteCommon = engine_->GetSpriteCommon();
    
    // ポインタのチェック
    if (!dxCommon || !spriteCommon) {
        return false; // 初期化失敗
    }
    
    // 既に初期化済みなら何もしない
    if (modelInitialized_) {
        return true;
    }

    try {
        // 球体モデルの生成
        sphereModel_ = new Model();
        sphereModel_->Initialize(dxCommon);
        sphereModel_->LoadFromObj("Resources/Models", "sphere.obj");
        
        // 球体オブジェクトの生成
        sphereObject_ = new Object3d();
        sphereObject_->Initialize(dxCommon, spriteCommon);
        sphereObject_->SetModel(sphereModel_);
        sphereObject_->SetCamera(camera_);
        sphereObject_->SetPosition({ 0.0f, 0.0f, 0.0f });
        sphereObject_->SetScale({ 0.5f, 0.5f, 0.5f });
        
        modelInitialized_ = true;
        return true;
    }
    catch (...) {
        // 例外発生時の処理
        if (sphereObject_) {
            delete sphereObject_;
            sphereObject_ = nullptr;
        }
        
        if (sphereModel_) {
            delete sphereModel_;
            sphereModel_ = nullptr;
        }
        
        modelInitialized_ = false;
        return false;
    }
}

void GamePlayScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // モデルの初期化（まだ初期化されていない場合）
    if (!modelInitialized_) {
        if (!InitializeModel()) {
            // モデル初期化失敗時は次のフレームで再試行
            return;
        }
    }

    // 基本的なカメラ操作
    ControlCamera();

    // カメラの更新
    camera_->Update();
    
    // 球体の回転更新
    if (modelInitialized_ && sphereObject_) {
        rotationAngle_ += 0.02f;
        sphereObject_->SetRotation({ 0.0f, rotationAngle_, 0.0f });
        sphereObject_->Update();
    }

    // ESCキーでタイトルシーンへ戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        // マウスカーソルを表示に戻し、拘束を解除
        input_->SetMouseCursorConfined(true, false);
        SceneManager::GetInstance()->ChangeScene("Title");
    }

    // TABキーでマウスカーソルの表示切替
    if (input_->TriggerKey(DIK_TAB)) {
        showCursor_ = !showCursor_;
        // カーソルの表示/非表示と拘束状態を設定
        // 非表示の場合は拘束する
        input_->SetMouseCursorConfined(showCursor_, !showCursor_);
    }
}

void GamePlayScene::ControlCamera() {
    // WASD基本移動操作
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

    // 上下移動
    if (input_->PushKey(DIK_SPACE)) {
        cameraPos.y += 0.1f;
    }
    if (input_->PushKey(DIK_LCONTROL)) {
        cameraPos.y -= 0.1f;
    }

    camera_->SetTranslate(cameraPos);
}

void GamePlayScene::Draw() {
    // 球体の描画（モデルが初期化されている場合のみ）
    if (modelInitialized_ && sphereObject_) {
        sphereObject_->Draw();
    }
    
    // ImGuiの描画
    DrawImGui();
}

void GamePlayScene::DrawImGui() {
    // ImGuiウィンドウ
    // ImGuiのフレームは MyGame::Draw() で開始されていることを前提とします
    ImGui::Begin("GamePlayScene");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)",
        camera_->GetTranslate().x,
        camera_->GetTranslate().y,
        camera_->GetTranslate().z);
    
    // 球体情報
    if (modelInitialized_ && sphereObject_) {
        ImGui::Separator();
        ImGui::Text("Sphere Information");
        ImGui::Text("Position: (%.2f, %.2f, %.2f)",
            sphereObject_->GetPosition().x,
            sphereObject_->GetPosition().y,
            sphereObject_->GetPosition().z);
        ImGui::Text("Rotation: (%.2f, %.2f, %.2f)",
            sphereObject_->GetRotation().x,
            sphereObject_->GetRotation().y,
            sphereObject_->GetRotation().z);
        ImGui::Text("Rotation Angle: %.2f", rotationAngle_);
    }
    
    ImGui::Separator();
    ImGui::Checkbox("Show Cursor", &showCursor_);
    ImGui::Text("Press ESC to return to Title");
    ImGui::End();
}

void GamePlayScene::Finalize() {
    // マウスカーソルを表示に戻し、拘束を解除する
    input_->SetMouseCursorConfined(true, false);
    
    // オブジェクトの解放
    if (sphereObject_) {
        delete sphereObject_;
        sphereObject_ = nullptr;
    }
    
    // モデルの解放
    if (sphereModel_) {
        delete sphereModel_;
        sphereModel_ = nullptr;
    }
    
    // 初期化フラグをリセット
    modelInitialized_ = false;
}
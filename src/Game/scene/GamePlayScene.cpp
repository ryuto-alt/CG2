#include "GamePlayScene.h"
#include "SceneManager.h"

GamePlayScene::GamePlayScene() {
    // UnoEngineのインスタンス取得
    engine_ = Uno::UnoEngine::GetInstance();
}

GamePlayScene::~GamePlayScene() {
    // デストラクタも空のままにする
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

void GamePlayScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // 基本的なカメラ操作
    ControlCamera();

    // カメラの更新
    camera_->Update();

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
    // 特に何も描画しない（必要に応じて3Dオブジェクトやスプライトを追加）
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
    ImGui::Checkbox("Show Cursor", &showCursor_);
    ImGui::Text("Press ESC to return to Title");
    ImGui::End();
}

void GamePlayScene::Finalize() {
    // マウスカーソルを表示に戻し、拘束を解除する
    input_->SetMouseCursorConfined(true, false);
}
// GamePlayScene.cpp
#include "GamePlayScene.h"
#include "SceneManager.h"
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

    // 初期化完了フラグ
    initialized_ = true;
}

void GamePlayScene::InitializeImGui() {
    // 何も初期化しない
}

void GamePlayScene::InitializeParticleEmitters() {
    // パーティクルを初期化しない
}

void GamePlayScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // 最小限の更新のみ
    camera_->Update();
}

void GamePlayScene::Draw() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // 何も描画しない（空の画面）
}

void GamePlayScene::Finalize() {
    // 何も解放しない
}

void GamePlayScene::DrawImGui() {
    // ImGuiも表示しない
}

void GamePlayScene::ControlCamera() {
    // カメラ制御も無効
}
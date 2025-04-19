// SceneManager.cpp
#include "SceneManager.h"
#include "SceneFactory.h"
#include <cassert>

// 静的メンバ変数の実体化
SceneManager* SceneManager::instance_ = nullptr;

SceneManager* SceneManager::GetInstance() {
    if (!instance_) {
        instance_ = new SceneManager();
    }
    return instance_;
}

void SceneManager::Initialize(SceneFactory* sceneFactory) {
    assert(sceneFactory);
    sceneFactory_ = sceneFactory;

    // 最初のシーンを設定（例: "GamePlay"）
    nextScene_ = "GamePlay";
}

void SceneManager::Update() {
    // シーン切り替えチェック
    if (!nextScene_.empty()) {
        // 現在のシーンの終了処理
        if (currentScene_) {
            currentScene_->Finalize();
            currentScene_.reset();
        }

        // 次のシーンを生成
        currentScene_ = sceneFactory_->CreateScene(nextScene_);

        // シーンマネージャーのポインタをセット
        currentScene_->SetSceneManager(this);

        // 共通リソースをセット
        currentScene_->SetDirectXCommon(dxCommon_);
        currentScene_->SetInput(input_);
        currentScene_->SetSpriteCommon(spriteCommon_);
        currentScene_->SetSrvManager(srvManager_);
        currentScene_->SetCamera(camera_);

        // シーンの初期化
        currentScene_->Initialize();

        // 次のシーン名をクリア
        nextScene_.clear();
    }

    // 現在のシーンの更新
    if (currentScene_) {
        currentScene_->Update();
    }
}

void SceneManager::Draw() {
    // 現在のシーンの描画
    if (currentScene_) {
        currentScene_->Draw();
    }
}

void SceneManager::Finalize() {
    // 現在のシーンの終了処理
    if (currentScene_) {
        currentScene_->Finalize();
        currentScene_.reset();
    }

    // シングルトンインスタンスの解放
    delete instance_;
    instance_ = nullptr;
}

void SceneManager::ChangeScene(const std::string& sceneName) {
    // 次のシーン名を設定
    nextScene_ = sceneName;
}
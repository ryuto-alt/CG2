// GamePlayScene.h
// ゲームプレイシーンクラス
#pragma once

#include "IScene.h"

// ゲームプレイシーンクラス
class GamePlayScene : public IScene {
public:
    // コンストラクタ
    GamePlayScene();

    // デストラクタ
    ~GamePlayScene() override;

    // 初期化
    void Initialize() override;

    // 更新
    void Update() override;

    // 描画
    void Draw() override;

    // 終了処理
    void Finalize() override;

private:
    // 各種ヘルパーメソッド
    void InitializeImGui();
    void InitializeParticleEmitters();
    void DrawImGui();
    void ControlCamera();

private:
    // シーンの状態管理
    bool initialized_ = false;
};
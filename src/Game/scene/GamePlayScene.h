#pragma once

#include "IScene.h"
#include "../Player.h"  // パスを修正
#include "../Ground.h"  // パスを修正
#include "PhysicsManager.h"
#include <memory>

// ゲームプレイシーンクラス - 完全実装
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
    // カメラ操作
    void ControlCamera();
    
    // ImGui表示
    void DrawImGui();

private:
    // 初期化フラグ
    bool initialized_ = false;

    // ESCキーでタイトルに戻るための機能
    bool showCursor_ = true;

    // プレイヤー
    std::unique_ptr<Player> player_ = nullptr;

    // 地面
    std::unique_ptr<Ground> ground_ = nullptr;

    // プレイヤーのモデル
    std::unique_ptr<Model> playerModel_ = nullptr;

    // 地面のモデル
    std::unique_ptr<Model> groundModel_ = nullptr;

    // 物理マネージャー
    PhysicsManager* physicsManager_ = nullptr;
    
    // カメラ関連パラメータ
    float cameraMoveSpeed_ = 0.1f;
    float cameraDistance_ = 10.0f;
    float cameraHeight_ = 5.0f;
    float cameraRotation_ = 0.0f;
};
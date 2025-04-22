// AthleticGame.h
#pragma once

#include "FPSPlayerController.h"
#include "IScene.h"
#include "Object3d.h"
#include "Model.h"
#include "Sprite.h"
#include "ParticleEmitter.h"
#include <memory>
#include <vector>
#include <string>
#include <chrono>

// ゲーム状態
enum class GameState {
    Title,       // タイトル画面
    Playing,     // ゲームプレイ中
    Paused,      // 一時停止
    GameClear,   // ゲームクリア
    GameOver     // ゲームオーバー
};

// アスレチックゲームクラス
class AthleticGame : public IScene {
public:
    // コンストラクタ
    AthleticGame();

    // デストラクタ
    ~AthleticGame() override;

    // 初期化
    void Initialize() override;

    // 更新
    void Update() override;

    // 描画
    void Draw() override;

    // 終了処理
    void Finalize() override;

private:
    // ステージ関連の初期化
    void InitializeStage();

    // UI関連の初期化
    void InitializeUI();

    // ImGuiの初期化
    void InitializeImGui();

    // ImGuiの描画
    void DrawImGui();

    // ゲーム状態に応じた更新
    void UpdateByState();

    // タイマーの更新
    void UpdateTimer();

    // ステージのリセット
    void ResetStage();

    // パーティクルの設定
    void SetupParticles();

    // ステージ固有のロジック
    void UpdateStageLogic();

    // キーボード入力の処理
    void ProcessInput();

    // ゴール判定
    void CheckGoalCondition();

private:
    // ゲーム状態
    GameState gameState_ = GameState::Title;

    // プレイヤーコントローラー
    std::unique_ptr<FPSPlayerController> playerController_;

    // 3Dモデル
    std::vector<std::unique_ptr<Model>> stageModels_;
    std::vector<std::unique_ptr<Object3d>> stageObjects_;

    std::unique_ptr<Model> goalModel_;
    std::unique_ptr<Object3d> goalObject_;

    // UI
    std::unique_ptr<Sprite> titleSprite_;    // タイトル画面
    std::unique_ptr<Sprite> gameOverSprite_; // ゲームオーバー画面
    std::unique_ptr<Sprite> clearSprite_;    // クリア画面
    std::unique_ptr<Sprite> timerSprite_;    // タイマー表示

    // パーティクルエミッタ
    std::unique_ptr<ParticleEmitter> goalParticleEmitter_;  // ゴール地点のパーティクル
    std::unique_ptr<ParticleEmitter> jumpPadParticleEmitter_; // ジャンプ台のパーティクル

    // シーンの状態管理
    bool initialized_ = false;

    // ステージデータ
    std::vector<CollisionObject> stageCollisions_;

    // ゴール地点
    Vector3 goalPosition_ = { 0.0f, 0.0f, 0.0f };
    float goalRadius_ = 2.0f;

    // タイマー関連
    std::chrono::steady_clock::time_point startTime_;
    std::chrono::steady_clock::time_point currentTime_;
    float elapsedTime_ = 0.0f;
    float bestTime_ = 999.0f;
    bool timerActive_ = false;

    // 回転角度（アニメーション用）
    float rotationAngle_ = 0.0f;
};
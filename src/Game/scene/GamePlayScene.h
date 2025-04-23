#pragma once

#include "IScene.h"
#include "Object3d.h"
#include "Model.h"
#include "ParticleEmitter.h"
#include <memory>

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
    void Initialize3DModels();
    void InitializeParticles();
    void DrawImGui();

    // カメラ制御メソッド
    void ControlCamera();
    void ControlFPSCamera();
    
    // パーティクル発射メソッド
    void ShootParticle();

private:
    // シーンの状態管理
    bool initialized_ = false;

    // 3Dモデル
    std::unique_ptr<Model> axisModel_;
    std::unique_ptr<Object3d> axisObject_;

    // 回転角度
    float rotationAngle_ = 0.0f;
    float yRotationAngle_ = 0.0f; // Y軸（横方向）回転用の変数を追加

    // FPS視点関連
    bool isFPSMode_ = true; // FPS視点モード
    float mouseSensitivity_ = 0.002f; // マウス感度
    
    // 前フレームのマウス座標
    int prevMouseX_ = 0;
    int prevMouseY_ = 0;
    
    // カメラの角度（オイラー角）
    float cameraYaw_ = 0.0f;   // 水平方向の回転角（Y軸周り）
    float cameraPitch_ = 0.0f; // 垂直方向の回転角（X軸周り）
    
    // カメラの移動速度
    float cameraSpeed_ = 0.2f;

    // マウスクリック状態
    bool isLeftButtonPressed_ = false;
    bool wasLeftButtonPressed_ = false;
    
    // パーティクル射出用パラメータ
    float particleCooldown_ = 0.0f;
    float particleCooldownMax_ = 0.1f; // 連射間隔（秒）
    
    // パーティクルエミッター
    std::unique_ptr<ParticleEmitter> particleEmitter_;
};
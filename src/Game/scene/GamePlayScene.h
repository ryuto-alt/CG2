#pragma once

#include "IScene.h"
#include "Object3d.h"
#include "Model.h"
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
    void DrawImGui();
    void ControlCamera();

private:
    // シーンの状態管理
    bool initialized_ = false;

    // 3Dモデル
    std::unique_ptr<Model> axisModel_;
    std::unique_ptr<Object3d> axisObject_;

    // 回転角度
    float rotationAngle_ = 0.0f;
    float yRotationAngle_ = 0.0f; // Y軸（横方向）回転用の変数を追加
};
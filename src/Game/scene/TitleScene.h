#pragma once

// UnoEngineのみをインクルード
#include "UnoEngine.h"
#include "IScene.h"

class TitleScene : public IScene {
public:
    // コンストラクタ
    TitleScene();

    // デストラクタ
    ~TitleScene();

    // 初期化
    void Initialize();

    // 更新
    void Update();

    // 描画
    void Draw();

    // 終了処理
    void Finalize();

private:
    // ImGuiの初期化
    void InitializeImGui();

    // 3Dモデルの初期化
    void Initialize3DModels();

    // スプライトの初期化
    void InitializeSprites();

    // デバッグ情報描画
    void DrawImGui();

private:
    // UnoEngineへの参照
    Uno::UnoEngine* engine_ = nullptr;

    // シーンの状態管理
    bool initialized_ = false;

    // タイトルロゴ
    std::unique_ptr<Sprite> titleLogo_;

    // 3Dモデル
    std::unique_ptr<Model> sphereModel_;
    std::unique_ptr<Object3d> sphereObject_;

    // 回転角度
    float rotationAngle_ = 0.0f;
};
#pragma once

// UnoEngineのみをインクルード
#include "UnoEngine.h"
#include "IScene.h"

class GamePlayScene : public IScene {
public:
    // コンストラクタ
    GamePlayScene();

    // デストラクタ
    ~GamePlayScene();

    // 初期化
    void Initialize();

    // 更新
    void Update();

    // 描画
    void Draw();

    // 終了処理
    void Finalize();

private:
    // カメラ操作
    void ControlCamera();
    
    // ImGui描画
    void DrawImGui();

private:
    // UnoEngineへの参照
    Uno::UnoEngine* engine_ = nullptr;

    // 初期化フラグ
    bool initialized_ = false;

    // モデル初期化フラグ
    bool modelInitialized_ = false;

    // カーソル表示フラグ
    bool showCursor_ = true;
    
    // 球体モデル
    Model* sphereModel_ = nullptr;
    
    // 球体オブジェクト
    Object3d* sphereObject_ = nullptr;
    
    // 回転角度
    float rotationAngle_ = 0.0f;
    
    // モデル初期化処理
    bool InitializeModel();
};
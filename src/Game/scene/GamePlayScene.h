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

    // カーソル表示フラグ
    bool showCursor_ = true;
};
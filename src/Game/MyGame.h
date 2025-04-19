// MyGame.h（修正版）
#pragma once

#include "Framework.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "Camera.h"
#include "SrvManager.h"
#include "SceneManager.h"
#include "SceneFactory.h"

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

// ゲーム全体を管理するクラス
class MyGame : public Framework {
public:
    // コンストラクタ・デストラクタ
    MyGame();
    ~MyGame() override;

    // WinAppの設定
    void SetWinApp(WinApp* winApp) { winApp_ = winApp; }

    // WinAppの取得
    WinApp* GetWinApp() const { return winApp_; }

    // Frameworkの関数をオーバーライド
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    // ImGuiの初期化
    void InitializeImGui();

private:
    // 基本システム
    WinApp* winApp_;
    DirectXCommon* dxCommon_;
    Input* input_;
    SpriteCommon* spriteCommon_;
    SrvManager* srvManager_;
    Camera* camera_;

    // シーン管理関連
    SceneManager* sceneManager_;
    GameSceneFactory* sceneFactory_;
};
#pragma once

#include "Framework.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "Camera.h"
#include "SrvManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "AudioManager.h"

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

    // Frameworkの関数をオーバーライド
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    // ImGuiの初期化
    void InitializeImGui();

    // パーティクルエミッタの初期化
    void InitializeParticleEmitters();

    // ImGuiの描画処理
    void DrawImGui();

    // カメラ操作
    void ControlCamera();

private:
    // 基本システム
    WinApp* winApp_;
    DirectXCommon* dxCommon_;
    Input* input_;
    SpriteCommon* spriteCommon_;
    SrvManager* srvManager_;
    Camera* camera_;

    // パーティクル関連
    ParticleEmitter* blueStarEmitter_;
    ParticleEmitter* greenStarEmitter_;
    ParticleEmitter* purpleStarEmitter_;

    // カメラ設定
    float cameraSpeed_;
    float mouseSensitivity_;
    bool showMouseCursor_;

    // オーディオ関連
    bool bgmLoaded_;
    bool seLoaded_;
    bool mp3Loaded_;
    float masterVolume_;
    float bgmVolume_;
    float seVolume_;
    float mp3Volume_;
};
#pragma once

// 基本的なエンジンコンポーネント
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "Camera.h"
#include "SrvManager.h"
#include "SceneManager.h"
#include "SceneFactory.h"
#include "AudioManager.h"
#include "ParticleManager.h"
#include "Object3d.h"
#include "Sprite.h"
#include "Model.h"
#include "PhysicsManager.h"

// ImGuiサポート
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

// 標準ライブラリ
#include <memory>
#include <string>

// 前方宣言
class SceneManager;

// UnoEngineクラス - エンジン機能を統合したシングルトンクラス
class UnoEngine {
private:
    // シングルトンインスタンス
    static UnoEngine* instance_;

    // コンストラクタ（シングルトン）
    UnoEngine();
    
    // デストラクタ（シングルトン）
    ~UnoEngine();
    
    // コピー禁止
    UnoEngine(const UnoEngine&) = delete;
    UnoEngine& operator=(const UnoEngine&) = delete;

public:
    // シングルトンインスタンスの取得
    static UnoEngine* GetInstance();
    
    // 初期化
    void Initialize(WinApp* winApp);
    
    // 更新
    void Update();
    
    // 描画前処理
    void BeginFrame();
    
    // 描画後処理
    void EndFrame();
    
    // 終了処理
    void Finalize();
    
    // シーン切り替え
    void ChangeScene(const std::string& sceneName);
    
    // シーンファクトリー設定
    void SetSceneFactory(SceneFactory* sceneFactory);
    
    // Windowsメッセージ処理（終了判定）
    bool ProcessMessage();
    
    // ゲッター
    WinApp* GetWinApp() const { return winApp_; }
    DirectXCommon* GetDirectXCommon() const { return dxCommon_.get(); }
    Input* GetInput() const { return input_.get(); }
    SpriteCommon* GetSpriteCommon() const { return spriteCommon_.get(); }
    SrvManager* GetSrvManager() const { return srvManager_.get(); }
    Camera* GetCamera() const { return camera_.get(); }
    SceneManager* GetSceneManager() const { return sceneManager_; }
    AudioManager* GetAudioManager() const { return AudioManager::GetInstance(); }
    ParticleManager* GetParticleManager() const { return ParticleManager::GetInstance(); }

private:
    // ImGuiの初期化
    void InitializeImGui();
    
    // パーティクルマネージャの初期化
    void InitializeParticle();

private:
    // 基本システム
    WinApp* winApp_;

    // unique_ptrによるリソース管理
    std::unique_ptr<DirectXCommon> dxCommon_;
    std::unique_ptr<Input> input_;
    std::unique_ptr<SpriteCommon> spriteCommon_;
    std::unique_ptr<SrvManager> srvManager_;
    std::unique_ptr<Camera> camera_;

    // シーン管理関連
    SceneManager* sceneManager_; // シングルトンなのでポインタのみ
    SceneFactory* sceneFactory_; // 外部から設定されるのでポインタのみ

    // 初期化済みフラグ
    bool isInitialized_;
};

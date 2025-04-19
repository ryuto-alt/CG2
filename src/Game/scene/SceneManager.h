// SceneManager.h
// シーン管理クラス（Singletonパターン）
#pragma once

#include <memory>
#include <string>
#include "IScene.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "SpriteCommon.h"
#include "SrvManager.h"
#include "Camera.h"

// 前方宣言
class SceneFactory;

// シーン管理クラス
class SceneManager final {
private:
    // シングルトンインスタンス
    static SceneManager* instance_;

    // コンストラクタ（シングルトン）
    SceneManager() = default;
    // デストラクタ（シングルトン）
    ~SceneManager() = default;
    // コピー禁止
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

public:
    // シングルトンインスタンスの取得
    static SceneManager* GetInstance();

    // 初期化
    void Initialize(SceneFactory* sceneFactory);

    // 更新
    void Update();

    // 描画
    void Draw();

    // 終了処理
    void Finalize();

    // シーン切り替え
    void ChangeScene(const std::string& sceneName);

    // DirectXCommonの設定
    void SetDirectXCommon(DirectXCommon* dxCommon) { dxCommon_ = dxCommon; }

    // Inputの設定
    void SetInput(Input* input) { input_ = input; }

    // SpriteCommonの設定
    void SetSpriteCommon(SpriteCommon* spriteCommon) { spriteCommon_ = spriteCommon; }

    // SrvManagerの設定
    void SetSrvManager(SrvManager* srvManager) { srvManager_ = srvManager; }

    // WinAppの設定
    void SetWinApp(WinApp* winApp) { winApp_ = winApp; }

    // WinAppの取得
    WinApp* GetWinApp() const { return winApp_; }

    // Cameraの設定
    void SetCamera(Camera* camera) { camera_ = camera; }

private:
    // シーンファクトリー
    SceneFactory* sceneFactory_ = nullptr;

    // 現在のシーン
    std::unique_ptr<IScene> currentScene_;

    // 次のシーン名（シーン切り替え用）
    std::string nextScene_;

    // 共通リソース（各シーンで使用）
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
    Camera* camera_ = nullptr;
    WinApp* winApp_ = nullptr;
};
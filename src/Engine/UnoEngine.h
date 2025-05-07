// UnoEngine.h
// 自作エンジンの機能を統合したヘッダファイル
#pragma once

// Windows/DirectX基本ヘッダー
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

// ユーティリティヘッダー
#include "Utility/WinApp.h"
#include "Utility/Logger.h"
#include "Utility/StringUtility.h"

// 数学関連ヘッダー
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix3x3.h"
#include "Math/Matrix4x4.h"
#include "Math/Mymath.h"

// グラフィックス関連ヘッダー
#include "Graphics/DirectXCommon.h"
#include "Graphics/SpriteCommon.h"
#include "Graphics/SRVManager.h"
#include "Graphics/TextureManager.h"
#include "Graphics/Sprite.h"
#include "Graphics/Model.h"
#include "Graphics/Object3d.h"
#include "Graphics/RenderingPipeline.h"
#include "Graphics/ResourceObject.h"
#include "Graphics/D3DResourceCheck.h"

// カメラ関連ヘッダー
#include "Camera/Camera.h"

// 入力関連ヘッダー
#include "Input/Input.h"

// オーディオ関連ヘッダー
#include "Audio/AudioManager.h"
#include "Audio/AudioSource.h"
#include "Audio/WaveFile.h"
#include "Audio/Mp3File.h"

// パーティクル関連ヘッダー
#include "Particle/ParticleManager.h"
#include "Particle/ParticleEmitter.h"

// ImGui関連ヘッダー
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

// シーン関連ヘッダーはシーンクラスで個別にインクルードする

namespace Uno {

    // UnoEngineクラス - エンジン機能の統合管理
    class UnoEngine {
    public:
        // シングルトンインスタンス取得
        static UnoEngine* GetInstance();

        // 初期化
        void Initialize(HINSTANCE hInstance);

        // 終了処理
        void Finalize();

        // 毎フレーム更新
        void BeginFrame();
        void EndFrame();

        // アクセサメソッド
        WinApp* GetWinApp() const { return winApp_.get(); }
        DirectXCommon* GetDirectXCommon() const { return dxCommon_.get(); }
        Input* GetInput() const { return input_.get(); }
        SpriteCommon* GetSpriteCommon() const { return spriteCommon_.get(); }
        SrvManager* GetSrvManager() const { return srvManager_.get(); }
        Camera* GetCamera() const { return camera_.get(); }
        AudioManager* GetAudioManager() const { return AudioManager::GetInstance(); }
        ParticleManager* GetParticleManager() const { return ParticleManager::GetInstance(); }
        TextureManager* GetTextureManager() const { return TextureManager::GetInstance(); }

        // ユーティリティメソッド
        bool ProcessMessage() const;

        // ウィンドウサイズ取得
        static constexpr int GetClientWidth() { return WinApp::kClientWidth; }
        static constexpr int GetClientHeight() { return WinApp::kClientHeight; }

    private:
        // シングルトンインスタンス
        static UnoEngine* instance_;

        // コンストラクタ（シングルトン）
        UnoEngine() = default;
        // デストラクタ（シングルトン）
        ~UnoEngine() = default;
        // コピー禁止
        UnoEngine(const UnoEngine&) = delete;
        UnoEngine& operator=(const UnoEngine&) = delete;

        // ImGui初期化
        void InitializeImGui();

        // メンバ変数
        std::unique_ptr<WinApp> winApp_;
        std::unique_ptr<DirectXCommon> dxCommon_;
        std::unique_ptr<Input> input_;
        std::unique_ptr<SpriteCommon> spriteCommon_;
        std::unique_ptr<SrvManager> srvManager_;
        std::unique_ptr<Camera> camera_;
    };

} // namespace Uno
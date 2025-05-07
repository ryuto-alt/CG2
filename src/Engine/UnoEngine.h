#pragma once

// Audio関連
#include "Audio/AudioManager.h"
#include "Audio/AudioSource.h"
#include "Audio/Mp3File.h"
#include "Audio/WaveFile.h"

// Camera関連
#include "Camera/Camera.h"

// Core関連
#include "Core/Framework.h"

// Graphics関連
#include "Graphics/D3DResourceCheck.h"
#include "Graphics/DirectXCommon.h"
#include "Graphics/Model.h"
#include "Graphics/Object3d.h"
#include "Graphics/RenderingPipeline.h"
#include "Graphics/ResourceObject.h"
#include "Graphics/Sprite.h"
#include "Graphics/SpriteCommon.h"
#include "Graphics/SrvManager.h"
#include "Graphics/TextureManager.h"

// Input関連
#include "Input/Input.h"

// Math関連
#include "Math/Matrix3x3.h"
#include "Math/Matrix4x4.h"
#include "Math/Mymath.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

// Particle関連
#include "Particle/ParticleEmitter.h"
#include "Particle/ParticleManager.h"

// Physics関連
#include "Physics/BoxCollider.h"
#include "Physics/Collider.h"
#include "Physics/CollisionObject3d.h"
#include "Physics/PhysicsManager.h"
#include "Physics/PlaneCollider.h"
#include "Physics/SphereCollider.h"

// Utility関連
#include "Utility/Logger.h"
#include "Utility/StringUtility.h"
#include "Utility/WinApp.h"

/**
 * @brief UnoEngineクラス
 * エンジン機能を一元管理するラッパークラス
 */
class UnoEngine {
public:
    // シングルトンパターンのためのインスタンス取得
    static UnoEngine* GetInstance();

    // 初期化
    void Initialize();

    // 更新
    void Update();

    // 描画前処理
    void PreDraw();

    // 描画後処理
    void PostDraw();

    // 終了処理
    void Finalize();

    // 各マネージャーへのアクセッサ
    WinApp* GetWinApp() { return winApp_.get(); }
    DirectXCommon* GetDirectXCommon() { return dxCommon_.get(); }
    Input* GetInput() { return input_.get(); }
    AudioManager* GetAudioManager() { return AudioManager::GetInstance(); }
    TextureManager* GetTextureManager() { return TextureManager::GetInstance(); }
    SrvManager* GetSrvManager() { return srvManager_.get(); }
    PhysicsManager* GetPhysicsManager() { return PhysicsManager::GetInstance(); }

private:
    // シングルトンパターンのためprivateコンストラクタ
    UnoEngine() = default;
    ~UnoEngine() = default;
    UnoEngine(const UnoEngine&) = delete;
    UnoEngine& operator=(const UnoEngine&) = delete;

    // 各システムコンポーネント
    std::unique_ptr<WinApp> winApp_;
    std::unique_ptr<DirectXCommon> dxCommon_;
    std::unique_ptr<Input> input_;
    // AudioManagerはシングルトンパターンを使用し、デストラクタがprivateなので、
    // unique_ptrではなく、GetInstance()を使用する
    // TextureManagerはシングルトンパターンを使用し、デストラクタがprivateなので、
    // unique_ptrではなく、GetInstance()を使用する
    std::unique_ptr<SrvManager> srvManager_;
    // PhysicsManagerはシングルトンパターンを使用し、デストラクタがprivateなので、
    // unique_ptrではなく、GetInstance()を使用する
};

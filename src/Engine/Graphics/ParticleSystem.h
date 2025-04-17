#pragma once

#include <vector>
#include <random>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Mymath.h"
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;
class SpriteCommon;
class Model;
class SrvManager;

// パーティクル1つの構造体
struct Particle {
    Transform transform;
    Vector3 velocity;
    float life;
    float scale;
    bool isActive;
};

// パーティクルシステムクラス
class ParticleSystem {
public:
    // コンストラクタ
    ParticleSystem();
    // デストラクタ
    ~ParticleSystem();

    // 初期化
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, SrvManager* srvManager, uint32_t particleCount);
    // モデルのセット
    void SetModel(Model* model);
    // 更新処理
    void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix);
    // 描画処理
    void Draw();

    // パーティクルの追加（指定位置に生成）
    void Emit(const Vector3& position, const Vector3& velocity, float scale, float life);
    // パーティクルの全追加（一斉に生成）
    void EmitAll(const Vector3& position, float speed, float scale, float life);

    // マテリアルカラーの設定
    void SetColor(const Vector4& color);

private:
    // DirectXCommon
    DirectXCommon* dxCommon_ = nullptr;
    // SpriteCommon
    SpriteCommon* spriteCommon_ = nullptr;
    // SrvManager
    SrvManager* srvManager_ = nullptr;
    // モデル
    Model* model_ = nullptr;

    // パーティクルの最大数
    uint32_t particleCount_ = 0;
    // パーティクル配列
    std::vector<Particle> particles_;

    // インスタンシング用のリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
    // インスタンシングデータ
    TransformationMatrix* instancingData_ = nullptr;

    // マテリアル用のリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    // マテリアルデータ
    Material* materialData_ = nullptr;

    // 乱数生成器
    std::mt19937 randomEngine_;
    std::uniform_real_distribution<float> distribution_;

    // 時間間隔
    float deltaTime_ = 1.0f / 60.0f;

    // SRV用のインデックス
    uint32_t srvIndex_ = 0;
};
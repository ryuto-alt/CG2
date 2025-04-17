
#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "SpriteCommon.h"
#include "Model.h"
#include "Camera.h"

 // パーティクル1粒のデータ
struct ParticleInstance {
    Vector3 scale = { 1.0f, 1.0f, 1.0f };
    Vector3 rotate = { 0.0f, 0.0f, 0.0f };
    Vector3 translate = { 0.0f, 0.0f, 0.0f };
    Vector3 velocity = { 0.0f, 0.0f, 0.0f }; // 移動のための速度
};

// パーティクル用変換行列（C++側定義 - シェーダーのTransformationMatrixに対応）
// 名前の競合を避けるため別名を使用
struct ParticleTransformData {
    Matrix4x4 WVP;
    Matrix4x4 World;
};

class Particle {
public:
    // コンストラクタ・デストラクタ
    Particle();
    ~Particle();

    // 初期化
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, SrvManager* srvManager, Model* model);

    // パーティクルの作成（指定した数だけ作成）
    void CreateParticles(uint32_t particleCount);

    // 更新
    void Update(const Matrix4x4& viewProjectionMatrix, float deltaTime);

    // 描画
    void Draw();

    // パーティクルデータの取得
    ParticleInstance* GetParticleInstance(uint32_t index);
    uint32_t GetParticleCount() const { return particleCount_; }

    // テクスチャの設定
    void SetTexture(const std::string& textureFilePath);

    // カラーの設定
    void SetColor(const Vector4& color) { material_.color = color; }
    const Vector4& GetColor() const { return material_.color; }

private:
    // DirectXCommon
    DirectXCommon* dxCommon_ = nullptr;
    // SpriteCommon
    SpriteCommon* spriteCommon_ = nullptr;
    // SrvManager
    SrvManager* srvManager_ = nullptr;
    // モデル
    Model* model_ = nullptr;

    // ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
    // パイプラインステート
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;

    // マテリアル
    struct Material {
        Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        int32_t enableLighting = 0;
        float padding[3] = { 0.0f, 0.0f, 0.0f }; // アラインメント調整用
        Matrix4x4 uvTransform = {};
    } material_;

    // マテリアルリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
    Material* materialData_ = nullptr;

    // ディレクショナルライト
    struct DirectionalLight {
        Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        Vector3 direction = { 0.0f, -1.0f, 0.0f };
        float intensity = 1.0f;
    } directionalLight_;

    // ライトリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
    DirectionalLight* directionalLightData_ = nullptr;

    // パーティクルデータ
    std::vector<ParticleInstance> particles_;

    // インスタンシング用リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_ = nullptr;
    ParticleTransformData* instancingData_ = nullptr;

    // SRVのインデックス
    uint32_t srvIndex_ = 0;

    // テクスチャのファイルパス
    std::string textureFilePath_;

    // パーティクル数
    uint32_t particleCount_ = 0;

    // ビュープロジェクション行列
    Matrix4x4 viewProjectionMatrix_;

    // ルートシグネチャの初期化
    void InitializeRootSignature();

    // グラフィックスパイプラインの初期化
    void InitializeGraphicsPipeline();
};
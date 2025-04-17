#include "ParticleSystem.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Model.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "Logger.h" // デバッグ出力用
#include "StringUtility.h"

using namespace StringUtility;

ParticleSystem::ParticleSystem() {
    // 乱数生成器の初期化
    std::random_device seedGenerator;
    randomEngine_ = std::mt19937(seedGenerator());
    distribution_ = std::uniform_real_distribution<float>(-1.0f, 1.0f);
}

ParticleSystem::~ParticleSystem() {
    // リソースの解放は自動的に行われる
}

void ParticleSystem::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, SrvManager* srvManager, uint32_t particleCount) {
    assert(dxCommon);
    assert(spriteCommon);
    assert(srvManager);
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    srvManager_ = srvManager;
    particleCount_ = particleCount;

    // パーティクル配列の確保
    particles_.resize(particleCount_);
    for (uint32_t i = 0; i < particleCount_; i++) {
        particles_[i].isActive = false;
        particles_[i].transform.scale = { 0.1f, 0.1f, 0.1f };  // 明示的に小さいスケールを設定
        particles_[i].transform.rotate = { 0.0f, 0.0f, 0.0f };
        particles_[i].transform.translate = { 0.0f, 0.0f, 0.0f };
    }

    // インスタンシング用のリソースを作成
    instancingResource_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix) * particleCount_);

    // 書き込むためのアドレスを取得
    instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

    // 単位行列を書き込んでおく
    for (uint32_t index = 0; index < particleCount_; ++index) {
        instancingData_[index].WVP = MakeIdentity4x4();
        instancingData_[index].World = MakeIdentity4x4();
    }

    // マテリアル用のリソースを作成
    materialResource_ = dxCommon_->CreateBufferResource(sizeof(Material));

    // マテリアルデータの書き込み
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData_->enableLighting = false;  // ライティングを無効化（パーティクルは自発光）
    materialData_->uvTransform = MakeIdentity4x4();

    // SRVの作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = particleCount_;
    srvDesc.Buffer.StructureByteStride = sizeof(TransformationMatrix);

    // SRVをSrvManagerに登録
    srvIndex_ = srvManager_->Allocate();
    srvManager_->CreateSRVForStructuredBuffer(srvIndex_, instancingResource_, particleCount_, sizeof(TransformationMatrix));

    // パーティクル用の専用パイプラインを作成
    CreateParticlePipeline();

    // デバッグ出力
    Logger::Log("ParticleSystem initialized successfully with " + std::to_string(particleCount_) + " particles!\n");
}

void ParticleSystem::CreateParticlePipeline() {
    // InputLayout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    // BlendStateの設定 - 加算合成
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    // パーティクル用シェーダーをコンパイル
    IDxcBlob* vertexShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Particle.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    IDxcBlob* pixelShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Particle.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    // DepthStencilStateの設定 - デプス書き込みなし
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // デプスへの書き込みを無効
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 手前に表示
    depthStencilDesc.StencilEnable = false;

    // パイプラインステート作成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
    pipelineDesc.pRootSignature = spriteCommon_->GetRootSignature();
    pipelineDesc.InputLayout = inputLayoutDesc;
    pipelineDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    pipelineDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    pipelineDesc.BlendState = blendDesc;
    pipelineDesc.RasterizerState = rasterizerDesc;
    pipelineDesc.NumRenderTargets = 1;
    pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineDesc.SampleDesc.Count = 1;
    pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    pipelineDesc.DepthStencilState = depthStencilDesc;
    pipelineDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // パイプラインステートを作成
    HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&particlePipelineState_));
    if (FAILED(hr)) {
        Logger::Log("Failed to create particle pipeline state. HRESULT: " + std::to_string(hr) + "\n");
    }
    assert(SUCCEEDED(hr));    assert(SUCCEEDED(hr));

    Logger::Log("Particle pipeline state created successfully!\n");
}

void ParticleSystem::SetModel(Model* model) {
    model_ = model;
}

void ParticleSystem::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
    uint32_t activeCount = 0;

    // パーティクルの更新
    for (uint32_t i = 0; i < particleCount_; i++) {
        if (!particles_[i].isActive) {
            continue;
        }

        // 寿命の更新
        particles_[i].life -= deltaTime_;
        if (particles_[i].life <= 0.0f) {
            particles_[i].isActive = false;
            continue;
        }

        // 位置の更新
        particles_[i].transform.translate.x += particles_[i].velocity.x * deltaTime_;
        particles_[i].transform.translate.y += particles_[i].velocity.y * deltaTime_;
        particles_[i].transform.translate.z += particles_[i].velocity.z * deltaTime_;

        // スケールの更新（徐々に小さくなる）
        float lifeRatio = particles_[i].life / particles_[i].scale;
        float currentScale = particles_[i].scale * lifeRatio;
        particles_[i].transform.scale = { currentScale, currentScale, currentScale };

        // 行列の計算
        Matrix4x4 worldMatrix = MakeAffineMatrix(
            particles_[i].transform.scale,
            particles_[i].transform.rotate,
            particles_[i].transform.translate);
        Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

        // インスタンシングデータの更新
        instancingData_[activeCount].WVP = worldViewProjectionMatrix;
        instancingData_[activeCount].World = worldMatrix;

        activeCount++;
    }

    // デバッグ出力
    if (activeCount > 0) {
        Logger::Log("Active particles: " + std::to_string(activeCount) + "\n");
    }
}

void ParticleSystem::Draw() {
    // アクティブなパーティクルがなければ描画しない
    uint32_t activeCount = 0;
    for (uint32_t i = 0; i < particleCount_; i++) {
        if (particles_[i].isActive) {
            activeCount++;
        }
    }

    if (activeCount == 0 || !model_) {
        return;
    }

    // 専用のパイプラインステートを設定
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(spriteCommon_->GetRootSignature());
    dxCommon_->GetCommandList()->SetPipelineState(particlePipelineState_.Get());
    dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // モデルの頂点バッファをセット
    dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &model_->GetVBView());

    // マテリアルCBufferの場所を設定
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

    // インスタンシングSRVのセット - SRVを新しいルートパラメータインデックス(4)にセット
    dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(4, srvManager_->GetGPUDescriptorHandle(srvIndex_));

    // テクスチャの場所を設定
    std::string texturePath = model_->GetTextureFilePath();
    if (!texturePath.empty()) {
        dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2,
            TextureManager::GetInstance()->GetSrvHandleGPU(texturePath));
    }

    // 描画
    dxCommon_->GetCommandList()->DrawInstanced(model_->GetVertexCount(), activeCount, 0, 0);

    // デバッグ出力
    Logger::Log("Drawing " + std::to_string(activeCount) + " particles\n");
}

void ParticleSystem::Emit(const Vector3& position, const Vector3& velocity, float scale, float life) {
    // 非アクティブなパーティクルを探して初期化
    for (uint32_t i = 0; i < particleCount_; i++) {
        if (!particles_[i].isActive) {
            particles_[i].isActive = true;
            particles_[i].transform.translate = position;
            particles_[i].transform.rotate = { 0.0f, 0.0f, 0.0f };
            particles_[i].transform.scale = { scale, scale, scale };
            particles_[i].velocity = velocity;
            particles_[i].life = life;
            particles_[i].scale = scale;

            // デバッグ出力
            Logger::Log("Emitted particle at position: " +
                std::to_string(position.x) + ", " +
                std::to_string(position.y) + ", " +
                std::to_string(position.z) + "\n");
            return;
        }
    }
}

void ParticleSystem::EmitAll(const Vector3& position, float speed, float scale, float life) {
    // デバッグ出力
    Logger::Log("Emitting all particles from position: " +
        std::to_string(position.x) + ", " +
        std::to_string(position.y) + ", " +
        std::to_string(position.z) + "\n");

    for (uint32_t i = 0; i < particleCount_; i++) {
        // より目立つように上向きの速度を設定
        Vector3 velocity = {
            distribution_(randomEngine_) * speed * 0.3f,  // X方向はランダムだが弱め
            std::abs(distribution_(randomEngine_)) * speed * 1.5f,  // Y方向は常に上向きで強め
            distribution_(randomEngine_) * speed * 0.3f   // Z方向はランダムだが弱め
        };

        // サイズを大きくして目立たせる
        float particleScale = scale * 3.0f;  // スケールを3倍に

        // 寿命を長くする
        float particleLife = life * 2.0f;    // 寿命を2倍に

        particles_[i].isActive = true;
        particles_[i].transform.translate = position;
        particles_[i].transform.rotate = { 0.0f, 0.0f, 0.0f };
        particles_[i].transform.scale = { particleScale, particleScale, particleScale };
        particles_[i].velocity = velocity;
        particles_[i].life = particleLife;
        particles_[i].scale = particleScale;
    }
}

void ParticleSystem::SetColor(const Vector4& color) {
    if (materialData_) {
        materialData_->color = color;
    }
}
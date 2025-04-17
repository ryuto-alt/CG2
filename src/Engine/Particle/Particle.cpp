#include "TextureManager.h"
#include "Mymath.h"
#include <cassert>
#include <random>
#include "Particle.h"

Particle::Particle() : dxCommon_(nullptr), spriteCommon_(nullptr), srvManager_(nullptr),
materialData_(nullptr), directionalLightData_(nullptr), instancingData_(nullptr), particleCount_(0) {
}

Particle::~Particle() {
}

void Particle::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, SrvManager* srvManager, Model* model) {
    assert(dxCommon);
    assert(spriteCommon);
    assert(srvManager);
    assert(model);

    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    srvManager_ = srvManager;
    model_ = model;

    // マテリアルリソースの作成
    materialResource_ = dxCommon_->CreateBufferResource(sizeof(Material));
    // マテリアルデータの書き込み
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    *materialData_ = material_;

    // ディレクショナルライトリソースの作成
    directionalLightResource_ = dxCommon_->CreateBufferResource(sizeof(DirectionalLight));
    // ディレクショナルライトデータの書き込み
    directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
    *directionalLightData_ = directionalLight_;

    // ルートシグネチャとパイプラインの初期化
    InitializeRootSignature();
    InitializeGraphicsPipeline();
}

void Particle::CreateParticles(uint32_t particleCount) {
    particleCount_ = particleCount;

    // パーティクルインスタンスの作成
    particles_.resize(particleCount_);

    // インスタンスデータの初期化
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-5.0f, 5.0f);
    std::uniform_real_distribution<float> velDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> scaleDist(0.05f, 0.2f);

    for (uint32_t i = 0; i < particleCount_; i++) {
        particles_[i].translate = { posDist(gen), posDist(gen), posDist(gen) };
        particles_[i].velocity = { velDist(gen), velDist(gen), velDist(gen) };
        float scale = scaleDist(gen);
        particles_[i].scale = { scale, scale, scale };
        particles_[i].rotate = { 0.0f, 0.0f, 0.0f };
    }

    // インスタンシング用リソースの作成
    instancingResource_ = dxCommon_->CreateBufferResource(sizeof(ParticleTransformData) * particleCount_);
    // データの書き込み準備
    instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

    // SRVの作成
    srvIndex_ = srvManager_->Allocate();
    srvManager_->CreateSRVForStructuredBuffer(
        srvIndex_,
        instancingResource_,
        particleCount_,
        sizeof(ParticleTransformData)
    );
}

void Particle::Update(const Matrix4x4& viewProjectionMatrix, float deltaTime) {
    viewProjectionMatrix_ = viewProjectionMatrix;

    // パーティクルの更新
    for (uint32_t i = 0; i < particleCount_; i++) {
        // 移動
        particles_[i].translate.x += particles_[i].velocity.x * deltaTime;
        particles_[i].translate.y += particles_[i].velocity.y * deltaTime;
        particles_[i].translate.z += particles_[i].velocity.z * deltaTime;

        // 回転
        particles_[i].rotate.y += 0.01f;

        // 範囲外に出たら反対側から戻す
        const float kBoundary = 10.0f;
        if (particles_[i].translate.x < -kBoundary) particles_[i].translate.x = kBoundary;
        if (particles_[i].translate.x > kBoundary) particles_[i].translate.x = -kBoundary;
        if (particles_[i].translate.y < -kBoundary) particles_[i].translate.y = kBoundary;
        if (particles_[i].translate.y > kBoundary) particles_[i].translate.y = -kBoundary;
        if (particles_[i].translate.z < -kBoundary) particles_[i].translate.z = kBoundary;
        if (particles_[i].translate.z > kBoundary) particles_[i].translate.z = -kBoundary;

        // 変換行列を計算
        Matrix4x4 worldMatrix = MakeAffineMatrix(
            particles_[i].scale,
            particles_[i].rotate,
            particles_[i].translate
        );

        // WVP行列の計算
        Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix_);

        // バッファに書き込み
        instancingData_[i].World = worldMatrix;
        instancingData_[i].WVP = worldViewProjectionMatrix;
    }

    // マテリアルの更新
    *materialData_ = material_;

    // ライトの更新
    *directionalLightData_ = directionalLight_;
}

void Particle::Draw() {
    // パイプラインステートとルートシグネチャの設定
    dxCommon_->GetCommandList()->SetPipelineState(pipelineState_.Get());
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
    dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 頂点バッファの設定
    dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &model_->GetVBView());

    // マテリアルの設定
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

    // ディレクショナルライトの設定
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, directionalLightResource_->GetGPUVirtualAddress());

    // StructuredBufferの設定
    dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(srvIndex_));

    // テクスチャの設定（テクスチャがセットされている場合）
    if (!textureFilePath_.empty()) {
        dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(3,
            TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));
    }

    // インスタンシング描画
    dxCommon_->GetCommandList()->DrawInstanced(model_->GetVertexCount(), particleCount_, 0, 0);
}

ParticleInstance* Particle::GetParticleInstance(uint32_t index) {
    assert(index < particleCount_);
    return &particles_[index];
}

void Particle::SetTexture(const std::string& textureFilePath) {
    // テクスチャの読み込み
    TextureManager::GetInstance()->LoadTexture(textureFilePath);
    textureFilePath_ = textureFilePath;
}

void Particle::InitializeRootSignature() {
    // RootSignature作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // ディスクリプタレンジを作成
    D3D12_DESCRIPTOR_RANGE descriptorRanges[2] = {};
    // StructuredBuffer用のレンジ (TransformationMatrix)
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].BaseShaderRegister = 0; // t0
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    // テクスチャ用のレンジ
    descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[1].NumDescriptors = 1;
    descriptorRanges[1].BaseShaderRegister = 0; // t0（ピクセルシェーダー用）
    descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // ルートパラメータを作成
    D3D12_ROOT_PARAMETER rootParameters[4] = {};
    // マテリアル用のCBV
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0; // b0
    // ディレクショナルライト用のCBV
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[1].Descriptor.ShaderRegister = 0; // b0（ピクセルシェーダー用）
    // TransformationMatrixのSRV (StructuredBuffer)
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    // テクスチャ用のSRV
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;

    // サンプラーを設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // ルートシグネチャの設定を完成させる
    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    // シリアライズしてバイナリにする
    ID3DBlob* signatureBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
        D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        assert(false);
    }

    // バイナリをもとに生成
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void Particle::InitializeGraphicsPipeline() {
    // 頂点レイアウト
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

    // ブレンド設定
    D3D12_BLEND_DESC blendDesc{};
    // 加算合成
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // ラスタライザー設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    // シェーダーをコンパイル
    IDxcBlob* vertexShaderBlob = dxCommon_->CompileShader(
        L"Resources/Shaders/particle.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);

    IDxcBlob* pixelShaderBlob = dxCommon_->CompileShader(
        L"Resources/Shaders/particle.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    // 深度ステンシル設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 深度書き込みなし
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // パイプラインステートオブジェクトの設定
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    graphicsPipelineStateDesc.BlendState = blendDesc;
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // パイプラインステートオブジェクトを生成
   HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(
        &graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState_));
    assert(SUCCEEDED(hr));
}
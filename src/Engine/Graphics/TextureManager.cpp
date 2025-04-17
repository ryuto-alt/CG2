#include "TextureManager.h"
#include "StringUtility.h"
#include "SrvManager.h"

using namespace StringUtility;

TextureManager* TextureManager::instance = nullptr;

TextureManager* TextureManager::GetInstance()
{
    if (instance == nullptr) {
        instance = new TextureManager;
    }
    return instance;
}

void TextureManager::Finalize()
{
    delete instance;
    instance = nullptr;
}

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
    assert(dxCommon);
    assert(srvManager);
    dxCommon_ = dxCommon;
    srvManager_ = srvManager;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
    // ファイルパスをキーに持つテクスチャデータを取得
    assert(textureDatas.count(filePath) > 0);
    return textureDatas[filePath].metadata;
}

void TextureManager::LoadTexture(const std::string& filePath)
{
    // 読み込み済みテクスチャを検索
    if (textureDatas.count(filePath) > 0) {
        return; // 読み込み済みなら早期return
    }

    // 最大数チェック
    assert(!srvManager_->IsMaxCount());

    // テクスチャファイルを読んでプログラムで扱えるようにする
    DirectX::ScratchImage image{};
    std::wstring filePathW = ConvertString(filePath);
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    assert(SUCCEEDED(hr));

    // ミニマップの作成
    DirectX::ScratchImage mipImages{};
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
    assert(SUCCEEDED(hr));

    // テクスチャデータを追加
    TextureData textureData;
    textureData.filePath = filePath;
    textureData.metadata = mipImages.GetMetadata();
    textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);

    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon_->UploadTextureData(textureData.resource, mipImages);
    dxCommon_->CommandKick();

    // SRVを作成
    textureData.srvIndex = srvManager_->Allocate();
    textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
    textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

    // SRVの設定
    srvManager_->CreateSRVForTexture2D(
        textureData.srvIndex,
        textureData.resource,
        textureData.metadata.format,
        static_cast<UINT>(textureData.metadata.mipLevels)
    );

    // マップに追加
    textureDatas[filePath] = textureData;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
    // ファイルパスをキーに持つテクスチャデータを取得
    assert(textureDatas.count(filePath) > 0);
    return textureDatas[filePath].srvHandleGPU;
}

// テクスチャのSRVインデックスを取得（追加）
uint32_t TextureManager::GetSrvIndex(const std::string& filePath)
{
    // ファイルパスをキーに持つテクスチャデータを取得
    assert(textureDatas.count(filePath) > 0);
    return textureDatas[filePath].srvIndex;
}
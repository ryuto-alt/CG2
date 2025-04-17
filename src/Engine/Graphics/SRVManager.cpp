#include "SrvManager.h"
#include "DirectXCommon.h"
#include <cassert>

void SrvManager::Initialize(DirectXCommon* dxCommon) {
    assert(dxCommon);
    dxCommon_ = dxCommon;

    // ディスクリプタヒープの作成
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.NumDescriptors = kMaxSRVCount;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    HRESULT hr = dxCommon_->GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
    assert(SUCCEEDED(hr));

    // ディスクリプタサイズを取得
    descriptorSize = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // ImGuiなどのシステム用に最初のいくつかのインデックスを予約
    useIndex_ = 1; // 0番は予約済みとする
}

uint32_t SrvManager::Allocate() {
    // 最大数チェック
    assert(!IsMaxCount());

    // インデックスを確保してから加算
    uint32_t index = useIndex_;
    useIndex_++;

    // 確保したインデックスを返す
    return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index) {
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize * index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorSize * index);
    return handleGPU;
}

void SrvManager::CreateSRVForTexture2D(uint32_t srvIndex, Microsoft::WRL::ComPtr<ID3D12Resource> pResource, DXGI_FORMAT format, UINT mipLevels) {
    // SRVの設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = mipLevels;

    // SRVの作成
    dxCommon_->GetDevice()->CreateShaderResourceView(
        pResource.Get(),
        &srvDesc,
        GetCPUDescriptorHandle(srvIndex)
    );
}

void SrvManager::CreateSRVForStructuredBuffer(uint32_t srvIndex, Microsoft::WRL::ComPtr<ID3D12Resource> pResource, UINT numElements, UINT structureByteStride) {
    // SRVの設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = numElements;
    srvDesc.Buffer.StructureByteStride = structureByteStride;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    // SRVの作成
    dxCommon_->GetDevice()->CreateShaderResourceView(
        pResource.Get(),
        &srvDesc,
        GetCPUDescriptorHandle(srvIndex)
    );
}

void SrvManager::PreDraw() {
    // 描画用のDescriptorHeapの設定
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { descriptorHeap };
    dxCommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT rootParameterIndex, uint32_t srvIndex) {
    // SRVのGPUハンドルを取得
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = GetGPUDescriptorHandle(srvIndex);

    // ルートパラメータにSRVをセット
    dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(rootParameterIndex, handleGPU);
}

bool SrvManager::IsMaxCount() {
    return useIndex_ >= kMaxSRVCount;
}
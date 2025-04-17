#pragma once
#include "DirectXCommon.h"

class SpriteCommon
{

public:
    // 初期化
    void Initialize(DirectXCommon* dxCommon);

    // 共通描画設定
    void CommonDraw();

    DirectXCommon* GetDxCommon()const { return dxCommon_; }

    // ルートシグネチャを取得するメソッドを追加
    ID3D12RootSignature* GetRootSignature() const { return rootSignature.Get(); }


private:
    // ルートシグネチャの作成
    void RootSignatureInitialize();

    // グラフィックスパイプライン
    void GraphicsPipelineInitialize();

private:
    DirectXCommon* dxCommon_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};
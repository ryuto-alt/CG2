#pragma once
#include "../Engine/UnoEngine.h"
#include <memory>

// 地面クラス
class Ground {
public:
    // コンストラクタ
    Ground();
    // デストラクタ
    ~Ground();

    // 初期化
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, Model* model);
    // 更新
    void Update();
    // 描画
    void Draw();

    // 位置の取得
    const Vector3& GetPosition() const;
    // 位置の設定
    void SetPosition(const Vector3& position);
    
    // スケールの設定
    void SetScale(const Vector3& scale);
    // スケールの取得
    const Vector3& GetScale() const;

private:
    // 3Dオブジェクト
    std::unique_ptr<CollisionObject3d> object_;
    // コライダー
    std::unique_ptr<BoxCollider> collider_;
};
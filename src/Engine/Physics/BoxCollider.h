#pragma once
#include "Collider.h"

// 箱型コライダー
class BoxCollider : public Collider {
public:
    // コンストラクタ
    BoxCollider() = default;
    // 半分のサイズ指定コンストラクタ
    BoxCollider(const Vector3& halfSize) : halfSize_(halfSize) {}
    // デストラクタ
    ~BoxCollider() override = default;

    // 半分のサイズを設定
    void SetHalfSize(const Vector3& halfSize) { halfSize_ = halfSize; }
    // 半分のサイズを取得
    const Vector3& GetHalfSize() const { return halfSize_; }

    // 衝突判定を行う
    CollisionInfo CheckCollision(const Collider* other) const override;

private:
    Vector3 halfSize_ = { 0.5f, 0.5f, 0.5f }; // 箱の半分のサイズ（中心から各辺までの距離）
};
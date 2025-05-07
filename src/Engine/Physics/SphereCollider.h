#pragma once
#include "Collider.h"

// 球型コライダー
class SphereCollider : public Collider {
public:
    // コンストラクタ
    SphereCollider() = default;
    // 半径指定コンストラクタ
    SphereCollider(float radius) : radius_(radius) {}
    // デストラクタ
    ~SphereCollider() override = default;

    // 半径を設定
    void SetRadius(float radius) { radius_ = radius; }
    // 半径を取得
    float GetRadius() const { return radius_; }

    // 衝突判定を行う
    CollisionInfo CheckCollision(const Collider* other) const override;

private:
    float radius_ = 0.5f; // 球の半径
};
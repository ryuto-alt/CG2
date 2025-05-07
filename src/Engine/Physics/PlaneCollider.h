#pragma once
#include "Collider.h"

// 平面コライダー（地面などに使用）
class PlaneCollider : public Collider {
public:
    // コンストラクタ
    PlaneCollider() = default;
    // 法線と距離を指定するコンストラクタ
    PlaneCollider(const Vector3& normal, float distance) : normal_(normal), distance_(distance) {}
    // デストラクタ
    ~PlaneCollider() override = default;

    // 法線を設定
    void SetNormal(const Vector3& normal) { normal_ = normal; }
    // 法線を取得
    const Vector3& GetNormal() const { return normal_; }

    // 原点からの距離を設定
    void SetDistance(float distance) { distance_ = distance; }
    // 原点からの距離を取得
    float GetDistance() const { return distance_; }

    // 衝突判定を行う
    CollisionInfo CheckCollision(const Collider* other) const override;

private:
    Vector3 normal_ = { 0.0f, 1.0f, 0.0f }; // 平面の法線ベクトル（デフォルトはY上向き）
    float distance_ = 0.0f;                 // 原点からの距離
};
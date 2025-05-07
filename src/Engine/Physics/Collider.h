#pragma once
#include "Vector3.h"
#include <memory>

// 当たり判定の基本クラス
class Collider {
public:
    // コンストラクタ
    Collider() = default;
    // 仮想デストラクタ
    virtual ~Collider() = default;

    // 衝突判定情報
    struct CollisionInfo {
        bool isColliding = false;     // 衝突しているか
        Vector3 collisionPoint = {};  // 衝突点
        Vector3 normal = {};          // 衝突面の法線ベクトル
        float penetration = 0.0f;     // めり込み量
    };

    // 位置の設定
    void SetPosition(const Vector3& position) { position_ = position; }
    // 位置の取得
    const Vector3& GetPosition() const { return position_; }

    // スケールの設定
    void SetScale(const Vector3& scale) { scale_ = scale; }
    // スケールの取得
    const Vector3& GetScale() const { return scale_; }

    // 衝突判定を行う（継承先で実装）
    virtual CollisionInfo CheckCollision(const Collider* other) const = 0;

protected:
    Vector3 position_ = { 0.0f, 0.0f, 0.0f }; // コライダーの位置
    Vector3 scale_ = { 1.0f, 1.0f, 1.0f };    // コライダーのスケール
};
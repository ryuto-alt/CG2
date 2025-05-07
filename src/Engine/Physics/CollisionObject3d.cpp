#include "CollisionObject3d.h"
#include "PhysicsManager.h"
#include <cassert>

CollisionObject3d::CollisionObject3d() : Object3d() {
    // 初期化時点ではコライダーはnullptr
}

CollisionObject3d::~CollisionObject3d() {
    // コライダーがあれば物理マネージャーから登録解除
    if (collider_) {
        PhysicsManager::GetInstance()->UnregisterCollider(collider_);
    }
}

void CollisionObject3d::SetCollider(Collider* collider) {
    // 以前のコライダーがあれば登録解除
    if (collider_) {
        PhysicsManager::GetInstance()->UnregisterCollider(collider_);
    }

    // 新しいコライダーを設定
    collider_ = collider;

    // 新しいコライダーがあれば物理マネージャーに登録
    if (collider_) {
        // コライダーの位置を同期
        collider_->SetPosition(GetPosition());
        collider_->SetScale(GetScale());

        // 物理マネージャーに登録
        PhysicsManager::GetInstance()->RegisterCollider(collider_);
    }
}

Collider* CollisionObject3d::GetCollider() {
    return collider_;
}

void CollisionObject3d::Update() {
    // 親クラスの更新処理
    Object3d::Update();

    // コライダーがあれば位置を同期
    if (collider_) {
        collider_->SetPosition(GetPosition());
        collider_->SetScale(GetScale());
    }

    // 衝突判定と結果処理
    ProcessCollisions();
}

void CollisionObject3d::ProcessCollisions() {
    // コライダーがなければ何もしない
    if (!collider_) {
        return;
    }

    // 衝突判定を行う
    std::vector<Collider::CollisionInfo> collisions = CheckCollisions();

    // コールバックが設定されている場合、各衝突情報に対してコールバックを呼び出す
    if (onCollisionCallback_) {
        for (const auto& info : collisions) {
            onCollisionCallback_(info);
        }
    }
}

std::vector<Collider::CollisionInfo> CollisionObject3d::CheckCollisions() {
    // コライダーがなければ空のリストを返す
    if (!collider_) {
        return {};
    }

    // 物理マネージャーで衝突判定を行う
    return PhysicsManager::GetInstance()->CheckCollisions(collider_);
}

void CollisionObject3d::SetOnCollisionCallback(CollisionCallback callback) {
    onCollisionCallback_ = callback;
}
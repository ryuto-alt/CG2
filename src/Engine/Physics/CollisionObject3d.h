#pragma once
#include "Object3d.h"
#include "Collider.h"
#include <memory>
#include <functional>
#include <vector>

// 衝突判定機能付きObject3d拡張クラス
class CollisionObject3d : public Object3d {
public:
    // コンストラクタ
    CollisionObject3d();
    // デストラクタ
    ~CollisionObject3d() override;

    // コライダーをセット
    void SetCollider(Collider* collider);
    // コライダーを取得
    Collider* GetCollider();

    // 更新処理（オーバーライド）
    void Update() override;

    // 衝突判定を行う
    std::vector<Collider::CollisionInfo> CheckCollisions();
    
    // 衝突判定と処理
    void ProcessCollisions();

    // 衝突したときのコールバック関数の型
    using CollisionCallback = std::function<void(const Collider::CollisionInfo&)>;
    
    // 衝突したときのコールバック関数を設定
    void SetOnCollisionCallback(CollisionCallback callback);

private:
    // コライダー
    Collider* collider_ = nullptr;
    
    // 衝突時のコールバック関数
    CollisionCallback onCollisionCallback_ = nullptr;
};
#pragma once
#include "Collider.h"
#include <vector>
#include <memory>
#include <map>

// 物理システム管理クラス
class PhysicsManager {
public:
    // シングルトンインスタンスの取得
    static PhysicsManager* GetInstance();

    // コライダーの登録
    void RegisterCollider(Collider* collider);
    // コライダーの登録解除
    void UnregisterCollider(Collider* collider);

    // 全ての衝突判定を更新（毎フレーム呼ばれる）
    void Update();

    // 指定されたコライダーと他のコライダーとの衝突判定を行う
    std::vector<Collider::CollisionInfo> CheckCollisions(Collider* collider);

private:
    // コンストラクタ（シングルトンなのでprivate）
    PhysicsManager() = default;
    // デストラクタ
    ~PhysicsManager() = default;

    // コピー禁止
    PhysicsManager(const PhysicsManager&) = delete;
    PhysicsManager& operator=(const PhysicsManager&) = delete;

    // シングルトンインスタンス
    static PhysicsManager* instance_;

    // 登録されたコライダーのリスト
    std::vector<Collider*> colliders_;
    
    // 衝突情報を保存するマップ
    std::map<Collider*, std::vector<Collider::CollisionInfo>> collisions_;
};
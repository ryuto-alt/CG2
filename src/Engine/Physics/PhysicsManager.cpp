#include "PhysicsManager.h"
#include <cassert>
#include <algorithm>

// 静的メンバの初期化
PhysicsManager* PhysicsManager::instance_ = nullptr;

PhysicsManager* PhysicsManager::GetInstance() {
    // インスタンスがまだ作成されていない場合は作成
    if (!instance_) {
        instance_ = new PhysicsManager();
    }
    return instance_;
}

void PhysicsManager::RegisterCollider(Collider* collider) {
    // nullptrチェック
    assert(collider);
    
    // すでに登録されているか確認
    auto it = std::find(colliders_.begin(), colliders_.end(), collider);
    if (it == colliders_.end()) {
        // 登録されていなければ追加
        colliders_.push_back(collider);
    }
}

void PhysicsManager::UnregisterCollider(Collider* collider) {
    // nullptrチェック
    assert(collider);
    
    // 登録されているか確認
    auto it = std::find(colliders_.begin(), colliders_.end(), collider);
    if (it != colliders_.end()) {
        // 登録されていれば削除
        colliders_.erase(it);
    }
}

void PhysicsManager::Update() {
    // 前のフレームの衝突情報をクリア
    collisions_.clear();
    
    // すべての当たり判定を行う
    for (size_t i = 0; i < colliders_.size(); i++) {
        for (size_t j = i + 1; j < colliders_.size(); j++) {
            // 二つのコライダーを取得
            Collider* colliderA = colliders_[i];
            Collider* colliderB = colliders_[j];
            
            // 両方が有効であることを確認
            if (colliderA && colliderB) {
                // 衝突判定
                Collider::CollisionInfo info = colliderA->CheckCollision(colliderB);
                
                // 衝突していた場合
                if (info.isColliding) {
                    // コライダーAの衝突情報を追加
                    collisions_[colliderA].push_back(info);
                    
                    // コライダーBの衝突情報を逆向きにして追加
                    Collider::CollisionInfo reverseInfo = info;
                    reverseInfo.normal.x = -info.normal.x;
                    reverseInfo.normal.y = -info.normal.y;
                    reverseInfo.normal.z = -info.normal.z;
                    collisions_[colliderB].push_back(reverseInfo);
                }
            }
        }
    }
}

std::vector<Collider::CollisionInfo> PhysicsManager::CheckCollisions(Collider* collider) {
    // 結果格納用のベクトル
    std::vector<Collider::CollisionInfo> results;
    
    // nullptrチェック
    if (!collider) return results;
    
    // すべてのコライダーと衝突判定
    for (auto* other : colliders_) {
        // 自分自身とは判定しない
        if (other == collider) continue;
        
        // 両方が有効であることを確認
        if (collider && other) {
            // 衝突判定
            Collider::CollisionInfo info = collider->CheckCollision(other);
            
            // 衝突していた場合は結果に追加
            if (info.isColliding) {
                results.push_back(info);
            }
        }
    }
    
    return results;
}
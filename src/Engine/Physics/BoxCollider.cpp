#include "BoxCollider.h"
#include "SphereCollider.h"
#include "PlaneCollider.h"
#include <cmath>
#include <algorithm>

Collider::CollisionInfo BoxCollider::CheckCollision(const Collider* other) const {
    // 他のコライダーの型に応じて判定を分岐
    const SphereCollider* otherSphere = dynamic_cast<const SphereCollider*>(other);
    const BoxCollider* otherBox = dynamic_cast<const BoxCollider*>(other);
    const PlaneCollider* otherPlane = dynamic_cast<const PlaneCollider*>(other);

    CollisionInfo info;
    
    // Box vs Sphere（すでにSphereColliderで実装済みなので、逆にして呼び出し）
    if (otherSphere) {
        info = otherSphere->CheckCollision(this);
        
        // 法線を反転（BoxからSphereへの方向に）
        info.normal.x = -info.normal.x;
        info.normal.y = -info.normal.y;
        info.normal.z = -info.normal.z;
        
        return info;
    }
    // Box vs Box
    else if (otherBox) {
        // 各軸ごとの半分のサイズ（スケール適用後）
        Vector3 thisHalfExtents = {
            halfSize_.x * scale_.x,
            halfSize_.y * scale_.y,
            halfSize_.z * scale_.z
        };
        
        Vector3 otherHalfExtents = {
            otherBox->GetHalfSize().x * otherBox->GetScale().x,
            otherBox->GetHalfSize().y * otherBox->GetScale().y,
            otherBox->GetHalfSize().z * otherBox->GetScale().z
        };
        
        // 中心間の距離
        Vector3 delta = {
            otherBox->GetPosition().x - position_.x,
            otherBox->GetPosition().y - position_.y,
            otherBox->GetPosition().z - position_.z
        };
        
        // 各軸方向の重なり具合
        float overlapX = thisHalfExtents.x + otherHalfExtents.x - std::abs(delta.x);
        float overlapY = thisHalfExtents.y + otherHalfExtents.y - std::abs(delta.y);
        float overlapZ = thisHalfExtents.z + otherHalfExtents.z - std::abs(delta.z);
        
        // すべての軸で重なりがあれば衝突
        if (overlapX > 0 && overlapY > 0 && overlapZ > 0) {
            info.isColliding = true;
            
            // 最も浅い軸方向のめり込みをめり込み量とする
            if (overlapX < overlapY && overlapX < overlapZ) {
                // X軸方向のめり込みが最小
                info.penetration = overlapX;
                
                // 法線はX軸方向（相手から自分への方向）
                info.normal = { (delta.x > 0) ? 1.0f : -1.0f, 0.0f, 0.0f };
                
                // 衝突点はめり込みの中心
                float sign = (delta.x > 0) ? 1.0f : -1.0f;
                info.collisionPoint = {
                    position_.x + sign * thisHalfExtents.x,
                    position_.y,
                    position_.z
                };
            }
            else if (overlapY < overlapZ) {
                // Y軸方向のめり込みが最小
                info.penetration = overlapY;
                
                // 法線はY軸方向（相手から自分への方向）
                info.normal = { 0.0f, (delta.y > 0) ? 1.0f : -1.0f, 0.0f };
                
                // 衝突点はめり込みの中心
                float sign = (delta.y > 0) ? 1.0f : -1.0f;
                info.collisionPoint = {
                    position_.x,
                    position_.y + sign * thisHalfExtents.y,
                    position_.z
                };
            }
            else {
                // Z軸方向のめり込みが最小
                info.penetration = overlapZ;
                
                // 法線はZ軸方向（相手から自分への方向）
                info.normal = { 0.0f, 0.0f, (delta.z > 0) ? 1.0f : -1.0f };
                
                // 衝突点はめり込みの中心
                float sign = (delta.z > 0) ? 1.0f : -1.0f;
                info.collisionPoint = {
                    position_.x,
                    position_.y,
                    position_.z + sign * thisHalfExtents.z
                };
            }
        }
    }
    // Box vs Plane
    else if (otherPlane) {
        // 平面の法線と距離
        const Vector3& planeNormal = otherPlane->GetNormal();
        float planeDistance = otherPlane->GetDistance();
        
        // ボックスの8つの頂点を計算
        Vector3 halfExtents = {
            halfSize_.x * scale_.x,
            halfSize_.y * scale_.y,
            halfSize_.z * scale_.z
        };
        
        Vector3 vertices[8] = {
            { position_.x - halfExtents.x, position_.y - halfExtents.y, position_.z - halfExtents.z },
            { position_.x + halfExtents.x, position_.y - halfExtents.y, position_.z - halfExtents.z },
            { position_.x - halfExtents.x, position_.y + halfExtents.y, position_.z - halfExtents.z },
            { position_.x + halfExtents.x, position_.y + halfExtents.y, position_.z - halfExtents.z },
            { position_.x - halfExtents.x, position_.y - halfExtents.y, position_.z + halfExtents.z },
            { position_.x + halfExtents.x, position_.y - halfExtents.y, position_.z + halfExtents.z },
            { position_.x - halfExtents.x, position_.y + halfExtents.y, position_.z + halfExtents.z },
            { position_.x + halfExtents.x, position_.y + halfExtents.y, position_.z + halfExtents.z }
        };
        
        // 平面の表側にある頂点と裏側にある頂点をカウント
        int numPositive = 0;
        int numNegative = 0;
        float minDistance = 1e10f;
        int closestVertex = -1;
        
        for (int i = 0; i < 8; ++i) {
            float distance = 
                planeNormal.x * vertices[i].x + 
                planeNormal.y * vertices[i].y + 
                planeNormal.z * vertices[i].z - planeDistance;
            
            if (distance > 0) {
                numPositive++;
            }
            else {
                numNegative++;
            }
            
            // 平面に最も近い頂点を記録
            if (std::abs(distance) < std::abs(minDistance)) {
                minDistance = distance;
                closestVertex = i;
            }
        }
        
        // 少なくとも1つの頂点が平面を横切っていれば衝突
        if (numPositive > 0 && numNegative > 0) {
            info.isColliding = true;
            
            // 最も平面に近い頂点を衝突点とする
            info.collisionPoint = vertices[closestVertex];
            
            // 法線は平面の法線（平面からボックスへの方向）
            if (minDistance < 0) {
                // 最近接点が平面の裏側にある場合は法線を反転
                info.normal = {
                    -planeNormal.x,
                    -planeNormal.y,
                    -planeNormal.z
                };
            }
            else {
                // 最近接点が平面の表側にある場合はそのまま
                info.normal = planeNormal;
            }
            
            // めり込み量は最近接頂点からの距離
            info.penetration = std::abs(minDistance);
        }
    }
    
    return info;
}
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "PlaneCollider.h"
#include <cmath>

Collider::CollisionInfo SphereCollider::CheckCollision(const Collider* other) const {
    // 他のコライダーの型に応じて判定を分岐
    const SphereCollider* otherSphere = dynamic_cast<const SphereCollider*>(other);
    const BoxCollider* otherBox = dynamic_cast<const BoxCollider*>(other);
    const PlaneCollider* otherPlane = dynamic_cast<const PlaneCollider*>(other);

    CollisionInfo info;
    
    // Sphere vs Sphere
    if (otherSphere) {
        // 中心点間の距離を計算
        Vector3 direction = {
            otherSphere->GetPosition().x - position_.x,
            otherSphere->GetPosition().y - position_.y,
            otherSphere->GetPosition().z - position_.z
        };
        
        // 距離の二乗
        float distanceSquared = 
            direction.x * direction.x + 
            direction.y * direction.y + 
            direction.z * direction.z;
        
        // 二つの球の半径の和
        float radiusSum = radius_ * scale_.x + otherSphere->GetRadius() * otherSphere->GetScale().x;
        
        // 衝突判定（距離の二乗で比較して平方根計算を省略）
        if (distanceSquared < radiusSum * radiusSum) {
            // 衝突している
            info.isColliding = true;
            
            // 距離
            float distance = std::sqrt(distanceSquared);
            
            // 衝突点（相手の球の中心から自分の球の方向に半径分移動した点）
            if (distance > 0.0f) {
                float invDistance = 1.0f / distance;
                Vector3 normalized = {
                    direction.x * invDistance,
                    direction.y * invDistance,
                    direction.z * invDistance
                };
                
                info.normal = {
                    -normalized.x,  // 相手から自分への法線なので反転
                    -normalized.y,
                    -normalized.z
                };
                
                info.collisionPoint = {
                    otherSphere->GetPosition().x + normalized.x * otherSphere->GetRadius() * otherSphere->GetScale().x,
                    otherSphere->GetPosition().y + normalized.y * otherSphere->GetRadius() * otherSphere->GetScale().y,
                    otherSphere->GetPosition().z + normalized.z * otherSphere->GetRadius() * otherSphere->GetScale().z
                };
                
                // めり込み量
                info.penetration = radiusSum - distance;
            }
            else {
                // 完全に重なっている場合
                info.normal = { 0.0f, 1.0f, 0.0f };  // 適当な上向きの法線
                info.collisionPoint = position_;      // 自分の位置を衝突点とする
                info.penetration = radius_ * scale_.x;  // 自分の半径をめり込み量とする
            }
        }
    }
    // Sphere vs Plane
    else if (otherPlane) {
        // 球の中心から平面までの距離を計算
        const Vector3& planeNormal = otherPlane->GetNormal();
        float planeDistance = otherPlane->GetDistance();
        
        // 中心から平面までの距離
        float distance = 
            planeNormal.x * position_.x + 
            planeNormal.y * position_.y + 
            planeNormal.z * position_.z - planeDistance;
        
        // 球の半径
        float scaledRadius = radius_ * scale_.x;
        
        // 衝突判定（平面の法線方向への距離が半径以下なら衝突）
        if (std::abs(distance) <= scaledRadius) {
            info.isColliding = true;
            
            // 衝突点（球の中心から平面の法線方向に半径分移動した点）
            info.collisionPoint = {
                position_.x - planeNormal.x * distance,
                position_.y - planeNormal.y * distance,
                position_.z - planeNormal.z * distance
            };
            
            // 法線は平面の法線（平面から球への方向）
            if (distance < 0) {
                // 球が平面の裏側にある場合は法線を反転
                info.normal = {
                    -planeNormal.x,
                    -planeNormal.y,
                    -planeNormal.z
                };
            }
            else {
                // 球が平面の表側にある場合はそのまま
                info.normal = planeNormal;
            }
            
            // めり込み量
            info.penetration = scaledRadius - std::abs(distance);
        }
    }
    // Sphere vs Box
    else if (otherBox) {
        // ボックスのローカル座標系で球の最近接点を求める
        const Vector3& boxPos = otherBox->GetPosition();
        const Vector3& boxScale = otherBox->GetScale();
        const Vector3& boxHalfSize = otherBox->GetHalfSize();
        
        // ボックスのスケール適用後のサイズ
        Vector3 scaledHalfSize = {
            boxHalfSize.x * boxScale.x,
            boxHalfSize.y * boxScale.y,
            boxHalfSize.z * boxScale.z
        };
        
        // 球からボックスへのベクトル（ボックス中心を原点とした座標）
        Vector3 sphereToBox = {
            position_.x - boxPos.x,
            position_.y - boxPos.y,
            position_.z - boxPos.z
        };
        
        // ボックス上の最近接点を求める（各軸でクランプ）
        Vector3 closestPoint = {
            std::max(-scaledHalfSize.x, std::min(sphereToBox.x, scaledHalfSize.x)),
            std::max(-scaledHalfSize.y, std::min(sphereToBox.y, scaledHalfSize.y)),
            std::max(-scaledHalfSize.z, std::min(sphereToBox.z, scaledHalfSize.z))
        };
        
        // 球中心と最近接点の距離の二乗
        Vector3 delta = {
            sphereToBox.x - closestPoint.x,
            sphereToBox.y - closestPoint.y,
            sphereToBox.z - closestPoint.z
        };
        
        float distanceSquared = 
            delta.x * delta.x + 
            delta.y * delta.y + 
            delta.z * delta.z;
        
        // 球の半径の二乗
        float scaledRadiusSquared = radius_ * scale_.x * radius_ * scale_.x;
        
        // 衝突判定
        if (distanceSquared < scaledRadiusSquared) {
            info.isColliding = true;
            
            // 距離
            float distance = std::sqrt(distanceSquared);
            
            if (distance > 0.0f) {
                // 法線ベクトル
                float invDistance = 1.0f / distance;
                info.normal = {
                    delta.x * invDistance,
                    delta.y * invDistance,
                    delta.z * invDistance
                };
                
                // ボックス座標系の最近接点をワールド座標系に変換
                info.collisionPoint = {
                    boxPos.x + closestPoint.x,
                    boxPos.y + closestPoint.y,
                    boxPos.z + closestPoint.z
                };
                
                // めり込み量
                info.penetration = radius_ * scale_.x - distance;
            }
            else {
                // 完全に重なっている場合
                info.normal = { 0.0f, 1.0f, 0.0f };  // 適当な上向きの法線
                info.collisionPoint = boxPos;        // ボックスの位置を衝突点とする
                info.penetration = scaledHalfSize.y + radius_ * scale_.x;  // Y方向のめり込み量
            }
        }
    }
    
    return info;
}
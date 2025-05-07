#include "PlaneCollider.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include <cmath>

Collider::CollisionInfo PlaneCollider::CheckCollision(const Collider* other) const {
    // 他のコライダーの型に応じて判定を分岐
    const SphereCollider* otherSphere = dynamic_cast<const SphereCollider*>(other);
    const BoxCollider* otherBox = dynamic_cast<const BoxCollider*>(other);
    const PlaneCollider* otherPlane = dynamic_cast<const PlaneCollider*>(other);

    CollisionInfo info;
    
    // Plane vs Sphere（すでにSphereColliderで実装済みなので、逆にして呼び出し）
    if (otherSphere) {
        info = otherSphere->CheckCollision(this);
        
        // 法線を反転（PlaneからSphereへの方向に）
        info.normal.x = -info.normal.x;
        info.normal.y = -info.normal.y;
        info.normal.z = -info.normal.z;
        
        return info;
    }
    // Plane vs Box（すでにBoxColliderで実装済みなので、逆にして呼び出し）
    else if (otherBox) {
        info = otherBox->CheckCollision(this);
        
        // 法線を反転（PlaneからBoxへの方向に）
        info.normal.x = -info.normal.x;
        info.normal.y = -info.normal.y;
        info.normal.z = -info.normal.z;
        
        return info;
    }
    // Plane vs Plane（常に接触しないと仮定）
    else if (otherPlane) {
        // 平行でない限り、交差線は存在するが、衝突点の特定は複雑なので
        // 今回は平面同士の衝突判定は実装しない（一般的なゲームでも不要）
        return info;
    }
    
    return info;
}
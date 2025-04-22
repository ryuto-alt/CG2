// FPSPlayerController.cpp
#include "FPSPlayerController.h"
#include <algorithm>
#include <cmath>
#include <Windows.h> // OutputDebugStringA用

// コンストラクタ
FPSPlayerController::FPSPlayerController()
    : position_({ 0.0f, 0.0f, 0.0f }),
    velocity_({ 0.0f, 0.0f, 0.0f }),
    previousPosition_({ 0.0f, 0.0f, 0.0f }),
    pitch_(0.0f),
    yaw_(0.0f) {
}

// デストラクタ
FPSPlayerController::~FPSPlayerController() {
}

// 初期化
void FPSPlayerController::Initialize(Input* input, Camera* camera) {
    input_ = input;
    camera_ = camera;

    // リスポーン位置を初期位置に設定
    respawnPosition_ = position_;

    // デバッグ出力
    OutputDebugStringA("FPSPlayerController initialized\n");
}

// 更新
void FPSPlayerController::Update() {
    // 前フレームの座標を保存
    previousPosition_ = position_;

    // 地面判定
    isGrounded_ = CheckGround();

    // 入力処理：移動
    ProcessMovement();

    // 入力処理：視点
    ProcessView();

    // 衝突判定と応答
    ProcessCollisions();

    // 落下判定
    if (position_.y < fallLimit_) {
        Respawn();
    }

    // カメラの更新
    if (camera_) {
        // 視点位置の設定（プレイヤー座標 + 目の高さ）
        Vector3 cameraPos = position_;
        cameraPos.y += physics_.eyeHeight;

        // カメラの位置と回転を設定
        camera_->SetTranslate(cameraPos);

        // 回転をカメラに設定（X軸回転はピッチ、Y軸回転はヨー）
        camera_->SetRotate({ -pitch_, yaw_, 0.0f }); // ピッチは反転させる

        // カメラの更新
        camera_->Update();
    }
}

// 移動処理
void FPSPlayerController::ProcessMovement() {
    // 速度を初期化
    Vector3 moveVelocity = { 0.0f, 0.0f, 0.0f };

    // キーボード入力から移動方向を計算
    if (input_->PushKey(DIK_W)) {
        // 前進（カメラの前方向に移動）
        float yRad = yaw_ * 3.14159f / 180.0f;
        moveVelocity.x += std::sin(yRad) * physics_.moveSpeed;
        moveVelocity.z += std::cos(yRad) * physics_.moveSpeed;
    }
    if (input_->PushKey(DIK_S)) {
        // 後退（カメラの後ろ方向に移動）
        float yRad = yaw_ * 3.14159f / 180.0f;
        moveVelocity.x -= std::sin(yRad) * physics_.moveSpeed;
        moveVelocity.z -= std::cos(yRad) * physics_.moveSpeed;
    }
    if (input_->PushKey(DIK_A)) {
        // 左移動（カメラの左方向に移動）
        float yRad = yaw_ * 3.14159f / 180.0f;
        moveVelocity.x -= std::cos(yRad) * physics_.moveSpeed;
        moveVelocity.z += std::sin(yRad) * physics_.moveSpeed;
    }
    if (input_->PushKey(DIK_D)) {
        // 右移動（カメラの右方向に移動）
        float yRad = yaw_ * 3.14159f / 180.0f;
        moveVelocity.x += std::cos(yRad) * physics_.moveSpeed;
        moveVelocity.z -= std::sin(yRad) * physics_.moveSpeed;
    }

    // ジャンプ
    if (input_->TriggerKey(DIK_SPACE) && isGrounded_) {
        velocity_.y = physics_.jumpPower;
        isJumping_ = true;
        isGrounded_ = false;
    }

    // 重力の適用
    if (!isGrounded_) {
        velocity_.y -= physics_.gravity * 0.01f; // 重力加速度を適用（フレームレート考慮で調整）
    }
    else {
        // 地面にいる場合はY速度をリセット
        velocity_.y = 0.0f;
    }

    // 速度を適用
    velocity_.x = moveVelocity.x;
    velocity_.z = moveVelocity.z;

    // 位置を更新
    position_.x += velocity_.x;
    position_.y += velocity_.y;
    position_.z += velocity_.z;
}

// 視点処理
void FPSPlayerController::ProcessView() {
    // マウス入力の取得
    DIMOUSESTATE mouseState;
    if (SUCCEEDED(input_->GetMouseState(&mouseState))) {
        // マウス感度
        float sensitivity = 0.2f;

        // ヨー（左右回転）の更新
        yaw_ += mouseState.lX * sensitivity;

        // ピッチ（上下回転）の更新
        pitch_ += mouseState.lY * sensitivity;

        // ピッチの制限（-89度〜89度）
        pitch_ = (pitch_ > 89.0f) ? 89.0f : ((pitch_ < -89.0f) ? -89.0f : pitch_);

        // ヨーが360度を超えたら0に戻す
        if (yaw_ > 360.0f) {
            yaw_ -= 360.0f;
        }
        else if (yaw_ < 0.0f) {
            yaw_ += 360.0f;
        }
    }
}

// 衝突判定と応答
void FPSPlayerController::ProcessCollisions() {
    // すべての衝突オブジェクトについて処理
    for (auto& object : collisionObjects_) {
        // 動く障害物の更新
        if (object.isMoving) {
            // 経過時間を更新
            object.moveTime += 1.0f / 60.0f; // 60FPS想定

            // サイン波で移動させる
            float offset = std::sin(object.moveTime) * object.moveRange;
            object.position = object.initialPosition;
            object.position.x += object.moveDirection.x * offset;
            object.position.y += object.moveDirection.y * offset;
            object.position.z += object.moveDirection.z * offset;
        }

        // 衝突判定
        if (CheckCollision(object)) {
            // 衝突応答（形状に応じた処理）
            Vector3 newPosition;

            switch (object.shape) {
            case CollisionShape::Box:
                newPosition = ResolveBoxCollision(position_, object);
                break;

            case CollisionShape::Sphere:
                newPosition = ResolveSphereCollision(position_, object);
                break;

            case CollisionShape::Cylinder:
                newPosition = ResolveCylinderCollision(position_, object);
                break;
            }

            // 位置を更新
            position_ = newPosition;

            // ジャンプ台の処理
            if (object.isJumpPad && position_.y >= object.position.y) {
                velocity_.y = object.jumpPadPower;
                isGrounded_ = false;
            }
        }
    }
}

// 地面判定
bool FPSPlayerController::CheckGround() {
    // 地面判定用のレイを下方向に飛ばす
    Vector3 rayStart = position_;
    Vector3 rayEnd = position_;
    rayEnd.y -= physics_.groundCheckDistance;

    // すべての衝突オブジェクトについて判定
    for (const auto& object : collisionObjects_) {
        // ボックスのみ地面と判定
        if (object.shape == CollisionShape::Box) {
            // ボックスの上面と交差判定
            float boxTop = object.position.y + object.scale.y * 0.5f;
            float boxLeft = object.position.x - object.scale.x * 0.5f;
            float boxRight = object.position.x + object.scale.x * 0.5f;
            float boxFront = object.position.z - object.scale.z * 0.5f;
            float boxBack = object.position.z + object.scale.z * 0.5f;

            // レイキャスト判定
            if (rayStart.y >= boxTop && rayEnd.y <= boxTop &&
                rayStart.x >= boxLeft && rayStart.x <= boxRight &&
                rayStart.z >= boxFront && rayStart.z <= boxBack) {

                // 交差点で位置を調整
                position_.y = boxTop;
                return true;
            }
        }
    }

    // 地面に接していない
    return false;
}

// 衝突判定
bool FPSPlayerController::CheckCollision(const CollisionObject& object) {
    switch (object.shape) {
    case CollisionShape::Box:
    {
        // ボックスの範囲
        float boxLeft = object.position.x - object.scale.x * 0.5f;
        float boxRight = object.position.x + object.scale.x * 0.5f;
        float boxBottom = object.position.y - object.scale.y * 0.5f;
        float boxTop = object.position.y + object.scale.y * 0.5f;
        float boxFront = object.position.z - object.scale.z * 0.5f;
        float boxBack = object.position.z + object.scale.z * 0.5f;

        // プレイヤーの円筒の判定
        float playerLeft = position_.x - physics_.collisionRadius;
        float playerRight = position_.x + physics_.collisionRadius;
        float playerBottom = position_.y;
        float playerTop = position_.y + physics_.eyeHeight;
        float playerFront = position_.z - physics_.collisionRadius;
        float playerBack = position_.z + physics_.collisionRadius;

        // 交差判定
        return (playerRight >= boxLeft && playerLeft <= boxRight &&
            playerTop >= boxBottom && playerBottom <= boxTop &&
            playerBack >= boxFront && playerFront <= boxBack);
    }

    case CollisionShape::Sphere:
    {
        // 球体の中心とプレイヤーの距離
        float dx = position_.x - object.position.x;
        float dz = position_.z - object.position.z;
        float distanceSquared = dx * dx + dz * dz;

        // プレイヤーと球体の半径合計
        float radiusSum = physics_.collisionRadius + object.scale.x * 0.5f;

        // Y軸方向の判定
        float dy = position_.y - object.position.y;
        float minY = -object.scale.y * 0.5f;
        float maxY = object.scale.y * 0.5f;

        // 交差判定
        return (distanceSquared <= radiusSum * radiusSum &&
            dy >= minY && dy <= maxY);
    }

    case CollisionShape::Cylinder:
    {
        // シリンダーの中心とプレイヤーの距離
        float dx = position_.x - object.position.x;
        float dz = position_.z - object.position.z;
        float distanceSquared = dx * dx + dz * dz;

        // プレイヤーとシリンダーの半径合計
        float radiusSum = physics_.collisionRadius + object.scale.x * 0.5f;

        // Y軸方向の判定
        float cylinderBottom = object.position.y - object.scale.y * 0.5f;
        float cylinderTop = object.position.y + object.scale.y * 0.5f;
        float playerBottom = position_.y;
        float playerTop = position_.y + physics_.eyeHeight;

        // 交差判定
        return (distanceSquared <= radiusSum * radiusSum &&
            playerTop >= cylinderBottom && playerBottom <= cylinderTop);
    }
    }

    return false;
}

// 衝突応答（ボックス）
Vector3 FPSPlayerController::ResolveBoxCollision(const Vector3& newPosition, const CollisionObject& object) {
    Vector3 resolvedPosition = newPosition;

    // ボックスの範囲
    float boxLeft = object.position.x - object.scale.x * 0.5f;
    float boxRight = object.position.x + object.scale.x * 0.5f;
    float boxBottom = object.position.y - object.scale.y * 0.5f;
    float boxTop = object.position.y + object.scale.y * 0.5f;
    float boxFront = object.position.z - object.scale.z * 0.5f;
    float boxBack = object.position.z + object.scale.z * 0.5f;

    // プレイヤーの円筒の判定
    float playerLeft = resolvedPosition.x - physics_.collisionRadius;
    float playerRight = resolvedPosition.x + physics_.collisionRadius;
    float playerBottom = resolvedPosition.y;
    float playerTop = resolvedPosition.y + physics_.eyeHeight;
    float playerFront = resolvedPosition.z - physics_.collisionRadius;
    float playerBack = resolvedPosition.z + physics_.collisionRadius;

    // X軸方向の貫通量を計算
    float penetrationX = 0.0f;
    if (playerRight > boxLeft && previousPosition_.x <= boxLeft) {
        penetrationX = playerRight - boxLeft;
    }
    else if (playerLeft < boxRight && previousPosition_.x >= boxRight) {
        penetrationX = playerLeft - boxRight;
    }

    // Z軸方向の貫通量を計算
    float penetrationZ = 0.0f;
    if (playerBack > boxFront && previousPosition_.z <= boxFront) {
        penetrationZ = playerBack - boxFront;
    }
    else if (playerFront < boxBack && previousPosition_.z >= boxBack) {
        penetrationZ = playerFront - boxBack;
    }

    // Y軸方向の貫通量を計算
    float penetrationY = 0.0f;
    if (playerTop > boxBottom && previousPosition_.y + physics_.eyeHeight <= boxBottom) {
        penetrationY = playerTop - boxBottom;
    }
    else if (playerBottom < boxTop && previousPosition_.y >= boxTop) {
        penetrationY = playerBottom - boxTop;
    }

    // 最小貫通方向を選択
    if (std::abs(penetrationX) > 0.0f &&
        (std::abs(penetrationX) <= std::abs(penetrationZ) || std::abs(penetrationZ) == 0.0f) &&
        (std::abs(penetrationX) <= std::abs(penetrationY) || std::abs(penetrationY) == 0.0f)) {
        resolvedPosition.x -= penetrationX;
    }
    else if (std::abs(penetrationZ) > 0.0f &&
        (std::abs(penetrationZ) <= std::abs(penetrationY) || std::abs(penetrationY) == 0.0f)) {
        resolvedPosition.z -= penetrationZ;
    }
    else if (std::abs(penetrationY) > 0.0f) {
        resolvedPosition.y -= penetrationY;
        if (penetrationY < 0.0f) {
            // 頭をぶつけた場合
            velocity_.y = 0.0f;
        }
    }

    return resolvedPosition;
}

// 衝突応答（球体）
Vector3 FPSPlayerController::ResolveSphereCollision(const Vector3& newPosition, const CollisionObject& object) {
    Vector3 resolvedPosition = newPosition;

    // 球体の中心とプレイヤーの距離
    float dx = resolvedPosition.x - object.position.x;
    float dz = resolvedPosition.z - object.position.z;
    float distance = std::sqrt(dx * dx + dz * dz);

    if (distance > 0.0f) {
        // プレイヤーと球体の半径合計
        float radiusSum = physics_.collisionRadius + object.scale.x * 0.5f;

        if (distance < radiusSum) {
            // 貫通量
            float penetration = radiusSum - distance;

            // 正規化した方向ベクトル
            float nx = dx / distance;
            float nz = dz / distance;

            // 位置の修正
            resolvedPosition.x += nx * penetration;
            resolvedPosition.z += nz * penetration;
        }
    }

    return resolvedPosition;
}

// 衝突応答（シリンダー）
Vector3 FPSPlayerController::ResolveCylinderCollision(const Vector3& newPosition, const CollisionObject& object) {
    Vector3 resolvedPosition = newPosition;

    // シリンダーの中心とプレイヤーの距離
    float dx = resolvedPosition.x - object.position.x;
    float dz = resolvedPosition.z - object.position.z;
    float distance = std::sqrt(dx * dx + dz * dz);

    if (distance > 0.0f) {
        // プレイヤーとシリンダーの半径合計
        float radiusSum = physics_.collisionRadius + object.scale.x * 0.5f;

        if (distance < radiusSum) {
            // 貫通量
            float penetration = radiusSum - distance;

            // 正規化した方向ベクトル
            float nx = dx / distance;
            float nz = dz / distance;

            // 位置の修正
            resolvedPosition.x += nx * penetration;
            resolvedPosition.z += nz * penetration;
        }
    }

    return resolvedPosition;
}

// 衝突オブジェクトの追加
void FPSPlayerController::AddCollisionObject(const CollisionObject& object) {
    CollisionObject newObject = object;

    // 動く障害物の初期位置を保存
    if (object.isMoving) {
        newObject.initialPosition = object.position;
        newObject.moveTime = 0.0f;
    }

    collisionObjects_.push_back(newObject);
}

// 衝突オブジェクトの全クリア
void FPSPlayerController::ClearCollisionObjects() {
    collisionObjects_.clear();
}

// 座標の設定
void FPSPlayerController::SetPosition(const Vector3& position) {
    position_ = position;
    previousPosition_ = position;
}

// ゴール判定
bool FPSPlayerController::CheckGoal(const Vector3& goalPosition, float goalRadius) {
    // プレイヤーとゴールの距離
    float dx = position_.x - goalPosition.x;
    float dy = position_.y - goalPosition.y;
    float dz = position_.z - goalPosition.z;
    float distanceSquared = dx * dx + dy * dy + dz * dz;

    // 距離の二乗で判定
    return distanceSquared <= goalRadius * goalRadius;
}

// リスポーン
void FPSPlayerController::Respawn() {
    // リスポーン位置にプレイヤーを移動
    position_ = respawnPosition_;
    previousPosition_ = respawnPosition_;

    // 速度をリセット
    velocity_ = { 0.0f, 0.0f, 0.0f };

    // デバッグ出力
    OutputDebugStringA("Player respawned\n");
}
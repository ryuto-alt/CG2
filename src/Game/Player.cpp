#include "Player.h"
#include <cassert>

Player::Player() {
    // 必要なオブジェクトの作成
    object_ = std::make_unique<CollisionObject3d>();
    collider_ = std::make_unique<SphereCollider>(0.5f); // 半径0.5の球体コライダー
}

Player::~Player() {
    // デストラクタでは特に処理はない
}

void Player::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, Model* model, Input* input) {
    assert(dxCommon);
    assert(spriteCommon);
    assert(model);
    assert(input);

    // 入力の設定
    input_ = input;

    // 3Dオブジェクトの初期化
    object_->Initialize(dxCommon, spriteCommon);
    object_->SetModel(model);

    // コライダーの設定
    object_->SetCollider(collider_.get());

    // 衝突時のコールバック設定
    object_->SetOnCollisionCallback([this](const Collider::CollisionInfo& info) {
        OnCollision(info);
    });

    // 初期位置と大きさの設定
    object_->SetPosition({ 0.0f, 1.0f, 0.0f });
    object_->SetScale({ 1.0f, 1.0f, 1.0f });
}

void Player::Update() {
    // 入力による移動
    Move();

    // 物理更新
    UpdatePhysics();

    // オブジェクトの更新
    object_->Update();
}

void Player::Draw() {
    object_->Draw();
}

const Vector3& Player::GetPosition() const {
    return object_->GetPosition();
}

void Player::SetPosition(const Vector3& position) {
    object_->SetPosition(position);
}

void Player::SetVelocity(const Vector3& velocity) {
    velocity_ = velocity;
}

const Vector3& Player::GetVelocity() const {
    return velocity_;
}

bool Player::IsGrounded() const {
    return isGrounded_;
}

void Player::Move() {
    // 現在の位置を取得
    Vector3 position = object_->GetPosition();
    Vector3 moveVec = { 0.0f, 0.0f, 0.0f };

    // WASD入力による移動方向の決定
    if (input_->PushKey(DIK_W)) {
        moveVec.z += 1.0f;
    }
    if (input_->PushKey(DIK_S)) {
        moveVec.z -= 1.0f;
    }
    if (input_->PushKey(DIK_A)) {
        moveVec.x -= 1.0f;
    }
    if (input_->PushKey(DIK_D)) {
        moveVec.x += 1.0f;
    }

    // 移動ベクトルの正規化（斜め移動でも速度を一定に）
    float moveLength = std::sqrt(moveVec.x * moveVec.x + moveVec.z * moveVec.z);
    if (moveLength > 0.0f) {
        moveVec.x /= moveLength;
        moveVec.z /= moveLength;

        // 水平方向の速度設定
        velocity_.x = moveVec.x * moveSpeed_;
        velocity_.z = moveVec.z * moveSpeed_;
    }
    else {
        // 入力がない場合は減速
        velocity_.x *= 0.9f;
        velocity_.z *= 0.9f;
    }

    // ジャンプ
    if (input_->TriggerKey(DIK_SPACE) && isGrounded_) {
        velocity_.y = jumpPower_;
        isGrounded_ = false;
    }
}

void Player::UpdatePhysics() {
    // 重力の適用
    if (!isGrounded_) {
        velocity_.y -= gravity_;
    }

    // 位置の更新
    Vector3 position = object_->GetPosition();
    position.x += velocity_.x;
    position.y += velocity_.y;
    position.z += velocity_.z;

    // 位置の反映
    object_->SetPosition(position);

    // 接地判定のリセット（衝突判定で再設定される）
    isGrounded_ = false;
}

void Player::OnCollision(const Collider::CollisionInfo& info) {
    // 衝突した場所が下側ならプレイヤーは地面に接地している
    if (info.normal.y > 0.1f) {
        // 接地判定を有効にする（条件を緩和）
        isGrounded_ = true;
        
        // Y方向の速度をリセット
        velocity_.y = 0.0f;
        
        // めり込み解消
        Vector3 position = object_->GetPosition();
        position.y += info.penetration;
        object_->SetPosition(position);
    }
    // 側面衝突の場合
    else if (std::abs(info.normal.y) < 0.8f) {
        // 対応する方向の速度を0にしてめり込みを解消
        Vector3 position = object_->GetPosition();
        
        // X方向の衝突
        if (std::abs(info.normal.x) > 0.1f) {
            velocity_.x = 0.0f;
            position.x += info.normal.x * info.penetration;
        }
        
        // Z方向の衝突
        if (std::abs(info.normal.z) > 0.1f) {
            velocity_.z = 0.0f;
            position.z += info.normal.z * info.penetration;
        }
        
        // 位置の更新
        object_->SetPosition(position);
    }
    // 天井衝突の場合
    else if (info.normal.y < -0.1f) {
        // Y方向の速度を反転（わずかに）
        velocity_.y = -velocity_.y * 0.1f;
        
        // めり込み解消
        Vector3 position = object_->GetPosition();
        position.y += info.normal.y * info.penetration;
        object_->SetPosition(position);
    }
}
#include "Player.h"
#include <cassert>
#include <algorithm>

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

    // 物理更新の中でobject_->Update()を呼び出すように修正したので
    // ここでは適切な順番で実行されるようにする
    UpdatePhysics();
    
    // 最終位置の接地チェックなどの追加処理に必要ならここに実装
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
    // 接地している場合のみジャンプ可能
    if (input_->TriggerKey(DIK_SPACE) && isGrounded_) {
        // ジャンプ力を少し強くして明確なジャンプを可能に
        velocity_.y = jumpPower_ * 1.5f;
        // 直後に接地してしまわないように、即座に接地フラグをリセット
        isGrounded_ = false;
    }
}

void Player::UpdatePhysics() {
    // 接地判定の保存
    bool wasGrounded = isGrounded_;
    
    // 位置を取得
    Vector3 position = object_->GetPosition();
    
    // ground.objの範囲を超えているかどうかをチェック
    float groundHalfWidth = 30.0f;
    if (std::abs(position.x) > groundHalfWidth || std::abs(position.z) > groundHalfWidth) {
        // 地面の外にいる場合は強制的に接地フラグをオフにする
        isGrounded_ = false;
    }
    
    // 接地していない場合のみ重力を適用
    if (!isGrounded_) {
        // 重力を大きくして落下感を増す
        velocity_.y -= gravity_;
    } else {
        // 接地中はY速度を完全に0にして安定させる
        velocity_.y = 0.0f;
    }

    // 速度に上限と下限を設定
    const float maxVelocity = 1.5f;  // 最大速度を増加
    const float minVelocity = -1.5f; // 最小速度を増加
    velocity_.y = std::clamp(velocity_.y, minVelocity, maxVelocity);

    // 接地している場合は、位置を固定して安定させる
    if (isGrounded_ && wasGrounded) {
        // Y座標を安定させる（前のフレームからも接地している場合）
        position.y = 0.5f; // 球体半径分の高さ
        object_->SetPosition(position);
        
        // XZ方向の移動のみを適用
        position.x += velocity_.x;
        position.z += velocity_.z;
        object_->SetPosition(position);
        
        // 再度地面の範囲チェック
        if (std::abs(position.x) > groundHalfWidth || std::abs(position.z) > groundHalfWidth) {
            // 地面の外に出た場合は接地フラグをオフに
            isGrounded_ = false;
        }
        
        // 衝突判定を行う
        object_->Update();
        
        // 地面外に出た場合は継続して落下処理を行う
        if (!isGrounded_) {
            // すでに位置更新済みなので、Y方向のみ計算継続
            velocity_.y -= gravity_;
        } else {
            return; // ここで処理終了（地面内で安定している場合）
        }
    }
    
    // 非接地の場合は通常かつ行分割した移動処理
    // 移動ステップ数を増やしてより細かくチェック
    const int steps = 8; 
    Vector3 stepVelocity = {
        velocity_.x / steps,
        velocity_.y / steps,
        velocity_.z / steps
    };
    
    // 分割した各ステップで移動と衝突判定を行う
    for (int i = 0; i < steps; i++) {
        // 前の反復で接地した場合、かつ地面の範囲内なら処理を終了
        if (isGrounded_ && 
            std::abs(position.x) <= groundHalfWidth && 
            std::abs(position.z) <= groundHalfWidth) {
            break;
        }
        
        // 位置を更新
        position.x += stepVelocity.x;
        position.y += stepVelocity.y;
        position.z += stepVelocity.z;
        
        // 位置を反映
        object_->SetPosition(position);
        
        // 地面の範囲を超えているかどうかを再度チェック
        if (std::abs(position.x) > groundHalfWidth || std::abs(position.z) > groundHalfWidth) {
            // 地面の外にいる場合は強制的に接地フラグをオフにする
            isGrounded_ = false;
        }
        
        // 衝突判定を行う
        object_->Update();
    }
}

void Player::OnCollision(const Collider::CollisionInfo& info) {
    // プレイヤーの位置を取得
    Vector3 position = object_->GetPosition();
    
    // プレイヤーが地面の外にいるか確認
    // ground.objの横幅は-30〜30なので、その範囲外なら接地させない
    float groundHalfWidth = 30.0f;
    if (std::abs(position.x) > groundHalfWidth || std::abs(position.z) > groundHalfWidth) {
        // 地面の外にいる場合は接地させない
        isGrounded_ = false;
        return;
    }
    
    // 天井との衝突の場合（法線が下向き）
    if (info.normal.y < -0.7f) {
        // Y方向の速度を反転
        velocity_.y = -velocity_.y * 0.2f;
        
        // めり込み解消
        position.y += info.normal.y * (info.penetration + 0.1f);
        object_->SetPosition(position);
        return;
    }
    
    // 側面衝突の場合（法線のY成分が小さい）
    if (std::abs(info.normal.y) < 0.7f) {
        // X方向の衝突
        if (std::abs(info.normal.x) > 0.3f) {
            velocity_.x = 0.0f;
            position.x += info.normal.x * (info.penetration + 0.1f);
        }
        
        // Z方向の衝突
        if (std::abs(info.normal.z) > 0.3f) {
            velocity_.z = 0.0f;
            position.z += info.normal.z * (info.penetration + 0.1f);
        }
        
        object_->SetPosition(position);
        return;
    }
    
    // 地面との衝突（法線が上向き）
    if (info.normal.y > 0.7f) {
        // 接地判定を有効にする
        isGrounded_ = true;
        
        // Y方向の速度を完全に0にする
        velocity_.y = 0.0f;
        
        // 地面の上に正確に配置（半径分の高さ）
        position.y = 0.5f;
        object_->SetPosition(position);
        return;
    }
}
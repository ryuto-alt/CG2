#pragma once
#include "CollisionObject3d.h"
#include "SphereCollider.h"
#include "Input.h"
#include <memory>

// プレイヤークラス
class Player {
public:
    // コンストラクタ
    Player();
    // デストラクタ
    ~Player();

    // 初期化
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, Model* model, Input* input);
    // 更新
    void Update();
    // 描画
    void Draw();

    // 位置の取得
    const Vector3& GetPosition() const;
    // 位置の設定
    void SetPosition(const Vector3& position);

    // 速度の設定
    void SetVelocity(const Vector3& velocity);
    // 速度の取得
    const Vector3& GetVelocity() const;

    // 地面に接地しているか
    bool IsGrounded() const;

private:
    // 移動処理
    void Move();
    // 物理更新
    void UpdatePhysics();
    // 衝突時のコールバック
    void OnCollision(const Collider::CollisionInfo& info);

private:
    // 3Dオブジェクト
    std::unique_ptr<CollisionObject3d> object_;
    // コライダー
    std::unique_ptr<SphereCollider> collider_;
    // 入力
    Input* input_ = nullptr;

    // 物理関連パラメータ
    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };
    float gravity_ = 0.01f;
    float jumpPower_ = 0.2f;
    float moveSpeed_ = 0.1f;
    bool isGrounded_ = false;
};
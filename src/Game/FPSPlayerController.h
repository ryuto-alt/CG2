// FPSPlayerController.h
#pragma once

#include "Vector3.h"
#include "Matrix4x4.h"
#include "Mymath.h"
#include "Camera.h"
#include "Input.h"
#include "Model.h"
#include <memory>
#include <vector>

// 物理演算用のパラメータ
struct PhysicsParameters {
    // 重力加速度
    float gravity = 9.8f;
    // 移動速度
    float moveSpeed = 0.15f;
    // ジャンプ力
    float jumpPower = 0.3f;
    // 地面までの距離（プレイヤーの足元から地面までの距離）
    float groundCheckDistance = 0.1f;
    // 地面からの高さ（カメラの高さ調整用）
    float eyeHeight = 1.7f;
    // 衝突判定用の半径
    float collisionRadius = 0.5f;
};

// コリジョン形状
enum class CollisionShape {
    Box,
    Sphere,
    Cylinder
};

// コリジョンオブジェクト
struct CollisionObject {
    // 座標
    Vector3 position;
    // スケール（サイズ）
    Vector3 scale;
    // 回転
    Vector3 rotation;
    // 形状
    CollisionShape shape;
    // ジャンプ台かどうか
    bool isJumpPad = false;
    // ジャンプ台の強さ
    float jumpPadPower = 0.5f;
    // 動く障害物かどうか
    bool isMoving = false;
    // 移動方向（正規化済み）
    Vector3 moveDirection = { 0.0f, 0.0f, 0.0f };
    // 移動速度
    float moveSpeed = 0.0f;
    // 移動範囲
    float moveRange = 0.0f;
    // 初期位置
    Vector3 initialPosition;
    // 移動経過時間
    float moveTime = 0.0f;
};

// FPSプレイヤーコントローラークラス
class FPSPlayerController {
public:
    // コンストラクタ
    FPSPlayerController();
    // デストラクタ
    ~FPSPlayerController();

    // 初期化
    void Initialize(Input* input, Camera* camera);
    // 更新
    void Update();
    // 衝突オブジェクトの追加
    void AddCollisionObject(const CollisionObject& object);
    // 衝突オブジェクトの全クリア
    void ClearCollisionObjects();
    // 座標の設定
    void SetPosition(const Vector3& position);
    // 座標の取得
    const Vector3& GetPosition() const { return position_; }
    // ゴール判定
    bool CheckGoal(const Vector3& goalPosition, float goalRadius);
    // リスポーン
    void Respawn();
    // リスポーン位置設定
    void SetRespawnPosition(const Vector3& position) { respawnPosition_ = position; }

private:
    // 移動処理
    void ProcessMovement();
    // 視点処理
    void ProcessView();
    // 衝突判定と応答
    void ProcessCollisions();
    // 地面判定
    bool CheckGround();
    // 衝突判定（プレイヤーと1つのオブジェクト）
    bool CheckCollision(const CollisionObject& object);
    // 衝突応答（ボックス）
    Vector3 ResolveBoxCollision(const Vector3& newPosition, const CollisionObject& object);
    // 衝突応答（球体）
    Vector3 ResolveSphereCollision(const Vector3& newPosition, const CollisionObject& object);
    // 衝突応答（シリンダー）
    Vector3 ResolveCylinderCollision(const Vector3& newPosition, const CollisionObject& object);

private:
    // 座標
    Vector3 position_;
    // 速度
    Vector3 velocity_;
    // 前回フレームの座標
    Vector3 previousPosition_;
    // 視点回転（ピッチ、ヨー）
    float pitch_ = 0.0f;
    float yaw_ = 0.0f;
    // 物理パラメータ
    PhysicsParameters physics_;
    // 地面フラグ
    bool isGrounded_ = false;
    // ジャンプフラグ
    bool isJumping_ = false;
    // 入力
    Input* input_ = nullptr;
    // カメラ
    Camera* camera_ = nullptr;
    // 衝突オブジェクトリスト
    std::vector<CollisionObject> collisionObjects_;
    // リスポーン位置
    Vector3 respawnPosition_ = { 0.0f, 0.0f, 0.0f };
    // 落下限界値（この座標よりも下に落ちたらリスポーン）
    float fallLimit_ = -10.0f;
    // デバッグフラグ（無敵モードなど）
    bool isDebugMode_ = false;
};
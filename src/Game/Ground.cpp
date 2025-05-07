#include "Ground.h"
#include <cassert>

Ground::Ground() {
    // 必要なオブジェクトの作成
    object_ = std::make_unique<CollisionObject3d>();
    collider_ = std::make_unique<BoxCollider>();
}

Ground::~Ground() {
    // デストラクタでは特に処理はない
}

void Ground::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, Model* model) {
    assert(dxCommon);
    assert(spriteCommon);
    assert(model);

    // 3Dオブジェクトの初期化
    object_->Initialize(dxCommon, spriteCommon);
    object_->SetModel(model);

    // コライダーの設定（OBJモデルの大きさにあわせて調整）
    collider_->SetHalfSize({ 30.0f, 1.0f, 30.0f }); // OBJファイルのサイズに合わせて設定
    
    // コライダーを設定
    object_->SetCollider(collider_.get());

    // 初期位置と大きさの設定
    object_->SetPosition({ 0.0f, 0.0f, 0.0f });
    object_->SetScale({ 1.0f, 1.0f, 1.0f }); // OBJファイルのスケールを維持
}

void Ground::Update() {
    // オブジェクトの更新
    object_->Update();
}

void Ground::Draw() {
    object_->Draw();
}

const Vector3& Ground::GetPosition() const {
    return object_->GetPosition();
}

void Ground::SetPosition(const Vector3& position) {
    object_->SetPosition(position);
}

void Ground::SetScale(const Vector3& scale) {
    object_->SetScale(scale);
    
    // スケールに合わせてコライダーのサイズも更新
    // Y方向は少し大きめに設定してすり抜けを防止
    collider_->SetHalfSize({ scale.x * 30.0f, scale.y * 1.0f, scale.z * 30.0f });
}

const Vector3& Ground::GetScale() const {
    return object_->GetScale();
}
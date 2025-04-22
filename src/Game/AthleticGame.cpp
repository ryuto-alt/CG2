// AthleticGame.cpp
#include "AthleticGame.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "TextureManager.h"
#include "ParticleManager.h"
#include <cassert>
#include <string>
#include <sstream>
#include <iomanip>

// コンストラクタ
AthleticGame::AthleticGame() {
	// コンストラクタでの初期化は最小限にする
}

// デストラクタ
AthleticGame::~AthleticGame() {
	// デストラクタでは特に何もしない
}

// 初期化
void AthleticGame::Initialize() {
	try {
		// リソースのnullチェック
		assert(dxCommon_);
		assert(input_);
		assert(spriteCommon_);
		assert(srvManager_);
		assert(camera_);

		// ImGuiの初期化
		InitializeImGui();

		// プレイヤーコントローラーの初期化
		playerController_ = std::make_unique<FPSPlayerController>();
		playerController_->Initialize(input_, camera_);

		// プレイヤーの初期位置
		playerController_->SetPosition({ 0.0f, 5.0f, 0.0f });
		playerController_->SetRespawnPosition({ 0.0f, 5.0f, 0.0f });

		// ステージの初期化
		InitializeStage();

		// UIの初期化
		InitializeUI();

		// パーティクルの設定
		SetupParticles();

		// 初期化完了フラグ
		initialized_ = true;

		// ゲーム状態を設定
		gameState_ = GameState::Title;

		// デバッグ出力
		OutputDebugStringA("AthleticGame: Successfully initialized\n");
	}
	catch (const std::exception& e) {
		OutputDebugStringA(("ERROR: Failed to initialize AthleticGame: " + std::string(e.what()) + "\n").c_str());
		throw; // 再スローして、上位の例外ハンドラで処理できるようにする
	}
}

// ステージの初期化
void AthleticGame::InitializeStage() {
	// 地面の生成
	Model* floorModel = new Model();
	floorModel->Initialize(dxCommon_);
	floorModel->LoadFromObj("Resources/models", "cube.obj");
	stageModels_.emplace_back(floorModel);

	Object3d* floorObject = new Object3d();
	floorObject->Initialize(dxCommon_, spriteCommon_);
	floorObject->SetModel(floorModel);
	floorObject->SetScale({ 20.0f, 1.0f, 20.0f });
	floorObject->SetPosition({ 0.0f, 0.0f, 0.0f });
	stageObjects_.emplace_back(floorObject);

	// 地面の衝突判定
	CollisionObject floorCollision;
	floorCollision.position = { 0.0f, 0.0f, 0.0f };
	floorCollision.scale = { 20.0f, 1.0f, 20.0f };
	floorCollision.rotation = { 0.0f, 0.0f, 0.0f };
	floorCollision.shape = CollisionShape::Box;
	stageCollisions_.push_back(floorCollision);

	// ジャンプ台1
	Model* jumpPadModel = new Model();
	jumpPadModel->Initialize(dxCommon_);
	jumpPadModel->LoadFromObj("Resources/models", "cube.obj");
	stageModels_.emplace_back(jumpPadModel);

	Object3d* jumpPadObject = new Object3d();
	jumpPadObject->Initialize(dxCommon_, spriteCommon_);
	jumpPadObject->SetModel(jumpPadModel);
	jumpPadObject->SetScale({ 3.0f, 0.5f, 3.0f });
	jumpPadObject->SetPosition({ 5.0f, 1.0f, 5.0f });
	jumpPadObject->SetColor({ 1.0f, 0.3f, 0.3f, 1.0f }); // 赤色
	stageObjects_.emplace_back(jumpPadObject);

	// ジャンプ台の衝突判定
	CollisionObject jumpPadCollision;
	jumpPadCollision.position = { 5.0f, 1.0f, 5.0f };
	jumpPadCollision.scale = { 3.0f, 0.5f, 3.0f };
	jumpPadCollision.rotation = { 0.0f, 0.0f, 0.0f };
	jumpPadCollision.shape = CollisionShape::Box;
	jumpPadCollision.isJumpPad = true;
	jumpPadCollision.jumpPadPower = 0.5f;
	stageCollisions_.push_back(jumpPadCollision);

	// 高台
	Model* platformModel = new Model();
	platformModel->Initialize(dxCommon_);
	platformModel->LoadFromObj("Resources/models", "cube.obj");
	stageModels_.emplace_back(platformModel);

	Object3d* platformObject = new Object3d();
	platformObject->Initialize(dxCommon_, spriteCommon_);
	platformObject->SetModel(platformModel);
	platformObject->SetScale({ 5.0f, 1.0f, 5.0f });
	platformObject->SetPosition({ -8.0f, 3.0f, -8.0f });
	platformObject->SetColor({ 0.3f, 0.3f, 1.0f, 1.0f }); // 青色
	stageObjects_.emplace_back(platformObject);

	// 高台の衝突判定
	CollisionObject platformCollision;
	platformCollision.position = { -8.0f, 3.0f, -8.0f };
	platformCollision.scale = { 5.0f, 1.0f, 5.0f };
	platformCollision.rotation = { 0.0f, 0.0f, 0.0f };
	platformCollision.shape = CollisionShape::Box;
	stageCollisions_.push_back(platformCollision);

	// 動く床
	Model* movingPlatformModel = new Model();
	movingPlatformModel->Initialize(dxCommon_);
	movingPlatformModel->LoadFromObj("Resources/models", "cube.obj");
	stageModels_.emplace_back(movingPlatformModel);

	Object3d* movingPlatformObject = new Object3d();
	movingPlatformObject->Initialize(dxCommon_, spriteCommon_);
	movingPlatformObject->SetModel(movingPlatformModel);
	movingPlatformObject->SetScale({ 3.0f, 0.5f, 3.0f });
	movingPlatformObject->SetPosition({ 0.0f, 2.0f, 8.0f });
	movingPlatformObject->SetColor({ 0.3f, 1.0f, 0.3f, 1.0f }); // 緑色
	stageObjects_.emplace_back(movingPlatformObject);

	// 動く床の衝突判定
	CollisionObject movingPlatformCollision;
	movingPlatformCollision.position = { 0.0f, 2.0f, 8.0f };
	movingPlatformCollision.scale = { 3.0f, 0.5f, 3.0f };
	movingPlatformCollision.rotation = { 0.0f, 0.0f, 0.0f };
	movingPlatformCollision.shape = CollisionShape::Box;
	movingPlatformCollision.isMoving = true;
	movingPlatformCollision.moveDirection = { 1.0f, 0.0f, 0.0f }; // X軸方向に移動
	movingPlatformCollision.moveRange = 5.0f; // 移動範囲
	movingPlatformCollision.moveSpeed = 0.05f; // 移動速度
	stageCollisions_.push_back(movingPlatformCollision);

	// 障害物（円柱）
	Model* cylinderModel = new Model();
	cylinderModel->Initialize(dxCommon_);
	cylinderModel->LoadFromObj("Resources/models", "cylinder.obj");
	stageModels_.emplace_back(cylinderModel);

	for (int i = 0; i < 5; i++) {
		Object3d* cylinderObject = new Object3d();
		cylinderObject->Initialize(dxCommon_, spriteCommon_);
		cylinderObject->SetModel(cylinderModel);
		cylinderObject->SetScale({ 1.0f, 3.0f, 1.0f });
		cylinderObject->SetPosition({ -5.0f + i * 2.0f, 1.5f, 0.0f });
		cylinderObject->SetColor({ 1.0f, 0.7f, 0.2f, 1.0f }); // オレンジ色
		stageObjects_.emplace_back(cylinderObject);

		// 円柱の衝突判定
		CollisionObject cylinderCollision;
		cylinderCollision.position = { -5.0f + i * 2.0f, 1.5f, 0.0f };
		cylinderCollision.scale = { 1.0f, 3.0f, 1.0f };
		cylinderCollision.rotation = { 0.0f, 0.0f, 0.0f };
		cylinderCollision.shape = CollisionShape::Cylinder;
		stageCollisions_.push_back(cylinderCollision);
	}

	// 狭い通路
	Model* wallModel = new Model();
	wallModel->Initialize(dxCommon_);
	wallModel->LoadFromObj("Resources/models", "cube.obj");
	stageModels_.emplace_back(wallModel);

	// 左壁
	Object3d* leftWallObject = new Object3d();
	leftWallObject->Initialize(dxCommon_, spriteCommon_);
	leftWallObject->SetModel(wallModel);
	leftWallObject->SetScale({ 1.0f, 2.0f, 10.0f });
	leftWallObject->SetPosition({ -12.0f, 1.0f, 0.0f });
	leftWallObject->SetColor({ 0.7f, 0.7f, 0.7f, 1.0f }); // グレー
	stageObjects_.emplace_back(leftWallObject);

	// 左壁の衝突判定
	CollisionObject leftWallCollision;
	leftWallCollision.position = { -12.0f, 1.0f, 0.0f };
	leftWallCollision.scale = { 1.0f, 2.0f, 10.0f };
	leftWallCollision.rotation = { 0.0f, 0.0f, 0.0f };
	leftWallCollision.shape = CollisionShape::Box;
	stageCollisions_.push_back(leftWallCollision);

	// 右壁
	Object3d* rightWallObject = new Object3d();
	rightWallObject->Initialize(dxCommon_, spriteCommon_);
	rightWallObject->SetModel(wallModel);
	rightWallObject->SetScale({ 1.0f, 2.0f, 10.0f });
	rightWallObject->SetPosition({ -10.0f, 1.0f, 0.0f });
	rightWallObject->SetColor({ 0.7f, 0.7f, 0.7f, 1.0f }); // グレー
	stageObjects_.emplace_back(rightWallObject);

	// 右壁の衝突判定
	CollisionObject rightWallCollision;
	rightWallCollision.position = { -10.0f, 1.0f, 0.0f };
	rightWallCollision.scale = { 1.0f, 2.0f, 10.0f };
	rightWallCollision.rotation = { 0.0f, 0.0f, 0.0f };
	rightWallCollision.shape = CollisionShape::Box;
	stageCollisions_.push_back(rightWallCollision);

	// ゴール地点
	goalModel_ = std::make_unique<Model>();
	goalModel_->Initialize(dxCommon_);
	goalModel_->LoadFromObj("Resources/models", "sphere.obj");

	goalObject_ = std::make_unique<Object3d>();
	goalObject_->Initialize(dxCommon_, spriteCommon_);
	goalObject_->SetModel(goalModel_.get());
	goalObject_->SetScale({ 2.0f, 2.0f, 2.0f });
	goalObject_->SetPosition({ -8.0f, 5.0f, -8.0f });
	goalObject_->SetColor({ 1.0f, 1.0f, 0.0f, 1.0f }); // 黄色

	// ゴール地点の設定
	goalPosition_ = { -8.0f, 5.0f, -8.0f };
	goalRadius_ = 2.0f;

	// プレイヤーに衝突オブジェクトを設定
	playerController_->ClearCollisionObjects();
	for (const auto& collision : stageCollisions_) {
		playerController_->AddCollisionObject(collision);
	}
}

// UIの初期化
void AthleticGame::InitializeUI() {
	try {
		// タイトル画面
		titleSprite_ = std::make_unique<Sprite>();
		titleSprite_->Initialize(spriteCommon_, "Resources/textures/title_background.png");
		titleSprite_->SetPosition({ WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f });
		titleSprite_->SetSize({ static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight) });
		titleSprite_->SetAnchorPoint({ 0.5f, 0.5f });

		// ゲームオーバー画面
		gameOverSprite_ = std::make_unique<Sprite>();
		gameOverSprite_->Initialize(spriteCommon_, "Resources/textures/gameover_background.png");
		gameOverSprite_->SetPosition({ WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f });
		gameOverSprite_->SetSize({ static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight) });
		gameOverSprite_->SetAnchorPoint({ 0.5f, 0.5f });

		// クリア画面
		clearSprite_ = std::make_unique<Sprite>();
		clearSprite_->Initialize(spriteCommon_, "Resources/textures/clear_background.png");
		clearSprite_->SetPosition({ WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f });
		clearSprite_->SetSize({ static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight) });
		clearSprite_->SetAnchorPoint({ 0.5f, 0.5f });

		// タイマー表示
		timerSprite_ = std::make_unique<Sprite>();
		timerSprite_->Initialize(spriteCommon_, "Resources/textures/timer_background.png");
		timerSprite_->SetPosition({ WinApp::kClientWidth - 100.0f, 50.0f });
		timerSprite_->SetSize({ 200.0f, 100.0f });
		timerSprite_->SetAnchorPoint({ 0.5f, 0.5f });
	}
	catch (const std::exception& e) {
		OutputDebugStringA(("Failed to initialize UI: " + std::string(e.what()) + "\n").c_str());
		// UIがロードできなくてもゲームは続行できるようにする
	}
}

// ImGuiの初期化
void AthleticGame::InitializeImGui() {
	// ImGuiの設定（必要に応じて）
	OutputDebugStringA("AthleticGame: ImGui initialized\n");
}

// パーティクルの設定
void AthleticGame::SetupParticles() {
	// ゴール地点のパーティクル
	goalParticleEmitter_ = std::make_unique<ParticleEmitter>(
		"goal_particle",       // パーティクルグループ名
		goalPosition_,         // 発生位置
		5,                     // 発生数
		2.0f,                  // 発生頻度
		Vector3(-0.5f, 0.5f, -0.5f),   // 最小速度
		Vector3(0.5f, 1.0f, 0.5f),     // 最大速度
		Vector3(0.0f, 0.0f, 0.0f),     // 最小加速度
		Vector3(0.0f, 0.0f, 0.0f),     // 最大加速度
		0.3f,                  // 最小初期サイズ
		0.5f,                  // 最大初期サイズ
		0.1f,                  // 最小終了サイズ
		0.2f,                  // 最大終了サイズ
		Vector4(1.0f, 1.0f, 0.0f, 1.0f), // 最小初期色（黄色）
		Vector4(1.0f, 1.0f, 0.0f, 1.0f), // 最大初期色（黄色）
		Vector4(1.0f, 1.0f, 0.0f, 0.0f), // 最小終了色（黄色、透明）
		Vector4(1.0f, 1.0f, 0.0f, 0.0f), // 最大終了色（黄色、透明）
		0.0f,                  // 最小回転角度
		360.0f,                // 最大回転角度
		-90.0f,                // 最小回転速度
		90.0f,                 // 最大回転速度
		1.0f,                  // 最小寿命
		2.0f                   // 最大寿命
	);

	// ジャンプ台のパーティクル
	jumpPadParticleEmitter_ = std::make_unique<ParticleEmitter>(
		"jump_particle",       // パーティクルグループ名
		Vector3(5.0f, 1.3f, 5.0f), // 発生位置（ジャンプ台の上）
		3,                     // 発生数
		1.5f,                  // 発生頻度
		Vector3(-0.3f, 0.5f, -0.3f),   // 最小速度
		Vector3(0.3f, 0.8f, 0.3f),     // 最大速度
		Vector3(0.0f, 0.0f, 0.0f),     // 最小加速度
		Vector3(0.0f, 0.0f, 0.0f),     // 最大加速度
		0.2f,                  // 最小初期サイズ
		0.4f,                  // 最大初期サイズ
		0.05f,                 // 最小終了サイズ
		0.1f,                  // 最大終了サイズ
		Vector4(1.0f, 0.3f, 0.3f, 1.0f), // 最小初期色（赤色）
		Vector4(1.0f, 0.3f, 0.3f, 1.0f), // 最大初期色（赤色）
		Vector4(1.0f, 0.3f, 0.3f, 0.0f), // 最小終了色（赤色、透明）
		Vector4(1.0f, 0.3f, 0.3f, 0.0f), // 最大終了色（赤色、透明）
		0.0f,                  // 最小回転角度
		360.0f,                // 最大回転角度
		-45.0f,                // 最小回転速度
		45.0f,                 // 最大回転速度
		0.5f,                  // 最小寿命
		1.0f                   // 最大寿命
	);
}

// 更新
void AthleticGame::Update() {
	// 初期化されていない場合は何もしない
	if (!initialized_) {
		OutputDebugStringA("AthleticGame: Update called before initialization\n");
		return;
	}

	try {
		// キーボード入力の処理
		ProcessInput();

		// ゲーム状態に応じた更新
		UpdateByState();

		// 動くオブジェクトや特殊オブジェクトの更新
		UpdateStageLogic();

		// タイマーの更新
		UpdateTimer();

		// ゴール判定
		CheckGoalCondition();

		// イメージUIの更新
		if (titleSprite_) titleSprite_->Update();
		if (gameOverSprite_) gameOverSprite_->Update();
		if (clearSprite_) clearSprite_->Update();
		if (timerSprite_) timerSprite_->Update();

		// パーティクルエミッタの更新
		if (goalParticleEmitter_) goalParticleEmitter_->Update();
		if (jumpPadParticleEmitter_) jumpPadParticleEmitter_->Update();
	}
	catch (const std::exception& e) {
		OutputDebugStringA(("ERROR in AthleticGame::Update: " + std::string(e.what()) + "\n").c_str());
	}
}

// キーボード入力の処理
void AthleticGame::ProcessInput() {
	// ESCキーでタイトルに戻る
	if (input_->TriggerKey(DIK_ESCAPE)) {
		if (gameState_ == GameState::Playing) {
			gameState_ = GameState::Paused;
		}
		else if (gameState_ == GameState::Paused) {
			gameState_ = GameState::Playing;
		}
	}

	// スペースキーでゲーム開始
	if (input_->TriggerKey(DIK_SPACE)) {
		if (gameState_ == GameState::Title) {
			gameState_ = GameState::Playing;
			ResetStage();
		}
		else if (gameState_ == GameState::GameClear || gameState_ == GameState::GameOver) {
			gameState_ = GameState::Title;
		}
	}

	// Rキーでリスポーン
	if (input_->TriggerKey(DIK_R)) {
		if (gameState_ == GameState::Playing) {
			playerController_->Respawn();
		}
	}
}

// ゲーム状態に応じた更新
void AthleticGame::UpdateByState() {
	switch (gameState_) {
	case GameState::Title:
	{
		// タイトル画面の更新
		rotationAngle_ += 0.02f;
		if (goalObject_) {
			goalObject_->SetRotation({ 0.0f, rotationAngle_, 0.0f });
			goalObject_->Update();
		}
		break;
	}

	case GameState::Playing:
	{
		// ゲームプレイ中の更新
		playerController_->Update();

		// ステージオブジェクトの更新
		for (auto& object : stageObjects_) {
			object->Update();
		}

		if (goalObject_) {
			rotationAngle_ += 0.02f;
			goalObject_->SetRotation({ 0.0f, rotationAngle_, 0.0f });
			goalObject_->Update();
		}
		break;
	}

	case GameState::Paused:
	{
		// 一時停止中の更新
		// （ほとんど何もしない）
		break;
	}

	case GameState::GameClear:
	{
		// ゲームクリア時の更新
		rotationAngle_ += 0.04f;
		if (goalObject_) {
			goalObject_->SetRotation({ 0.0f, rotationAngle_, 0.0f });
			goalObject_->Update();
		}
		break;
	}

	case GameState::GameOver:
	{
		// ゲームオーバー時の更新
		// （ほとんど何もしない）
		break;
	}
	}
}

// タイマーの更新
void AthleticGame::UpdateTimer() {
	// プレイ中のみタイマー更新
	if (gameState_ == GameState::Playing) {
		// タイマーが非アクティブならスタート
		if (!timerActive_) {
			startTime_ = std::chrono::steady_clock::now();
			timerActive_ = true;
		}

		// 経過時間の更新
		currentTime_ = std::chrono::steady_clock::now();
		std::chrono::duration<float> elapsed = currentTime_ - startTime_;
		elapsedTime_ = elapsed.count();
	}
	else if (gameState_ != GameState::Paused) {
		// プレイ中でなければタイマーをリセット
		timerActive_ = false;
	}
}

// ステージのリセット
void AthleticGame::ResetStage() {
	// プレイヤーをリスポーン地点に戻す
	playerController_->Respawn();

	// タイマーリセット
	elapsedTime_ = 0.0f;
	timerActive_ = false;

	// 動く障害物の位置リセット
	for (auto& collision : stageCollisions_) {
		if (collision.isMoving) {
			collision.position = collision.initialPosition;
			collision.moveTime = 0.0f;
		}
	}

	// プレイヤーに衝突オブジェクトを再設定
	playerController_->ClearCollisionObjects();
	for (const auto& collision : stageCollisions_) {
		playerController_->AddCollisionObject(collision);
	}
}

// ステージ固有のロジック
void AthleticGame::UpdateStageLogic() {
	// 動く障害物の位置を3Dオブジェクトに反映
	int movingObjectIndex = 0;
	for (size_t i = 0; i < stageCollisions_.size(); i++) {
		if (stageCollisions_[i].isMoving) {
			// 対応するObject3dを探す（インデックスで仮定）
			if (movingObjectIndex < stageObjects_.size()) {
				stageObjects_[3]->SetPosition(stageCollisions_[i].position);
				movingObjectIndex++;
			}
		}
	}
}

// ゴール判定
void AthleticGame::CheckGoalCondition() {
	// プレイ中のみゴール判定
	if (gameState_ == GameState::Playing) {
		if (playerController_->CheckGoal(goalPosition_, goalRadius_)) {
			// ゴール達成
			gameState_ = GameState::GameClear;

			// ベストタイム更新判定
			if (elapsedTime_ < bestTime_) {
				bestTime_ = elapsedTime_;
			}

			// デバッグ出力
			OutputDebugStringA(("Goal reached! Time: " + std::to_string(elapsedTime_) + " seconds\n").c_str());
		}
	}
}

// 描画
void AthleticGame::Draw() {
	// 初期化されていない場合は何もしない
	if (!initialized_) {
		OutputDebugStringA("AthleticGame: Draw called before initialization\n");
		return;
	}

	try {
		// SRVヒープを描画前に設定
		if (srvManager_) {
			srvManager_->PreDraw();
		}

		// ゲーム状態に応じた描画
		switch (gameState_) {
		case GameState::Title:
		{
			// タイトル画面の描画
			if (titleSprite_) {
				spriteCommon_->CommonDraw();
				titleSprite_->Draw();
			}

			// ゴールオブジェクトの描画
			if (goalObject_) {
				goalObject_->Draw();
			}
			break;
		}

		case GameState::Playing:
		case GameState::Paused:
		{
			// ステージの描画
			for (auto& object : stageObjects_) {
				object->Draw();
			}

			// ゴールオブジェクトの描画
			if (goalObject_) {
				goalObject_->Draw();
			}

			// タイマー表示の描画
			if (timerSprite_) {
				spriteCommon_->CommonDraw();
				timerSprite_->Draw();
			}
			break;
		}

		case GameState::GameClear:
		{
			// クリア画面の描画
			if (clearSprite_) {
				spriteCommon_->CommonDraw();
				clearSprite_->Draw();
			}
			break;
		}

		case GameState::GameOver:
		{
			// ゲームオーバー画面の描画
			if (gameOverSprite_) {
				spriteCommon_->CommonDraw();
				gameOverSprite_->Draw();
			}
			break;
		}
		}

		// ImGuiの描画
		DrawImGui();
	}
	catch (const std::exception& e) {
		OutputDebugStringA(("ERROR in AthleticGame::Draw: " + std::string(e.what()) + "\n").c_str());
	}
}

// ImGuiの描画
void AthleticGame::DrawImGui() {
	try {
		// ImGuiの新しいフレーム開始
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// ゲーム状態に応じた表示
		switch (gameState_) {
		case GameState::Title:
		{
			// タイトル画面用のImGui
			ImGui::SetNextWindowPos(ImVec2(WinApp::kClientWidth / 2 - 150, 100), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);
			ImGui::Begin("FPS Athletic", nullptr, ImGuiWindowFlags_NoResize);
			ImGui::SetWindowFontScale(1.5f);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Athletic Game");
			ImGui::SetWindowFontScale(1.0f);
			ImGui::Text("Press SPACE to start game");
			ImGui::Text("Controls:");
			ImGui::Text("WASD - Move, SPACE - Jump");
			ImGui::Text("R - Respawn, ESC - Pause");
			ImGui::Text("Mouse - Look around");
			ImGui::End();
			break;
		}

		case GameState::Playing:
		{
			// プレイ中のUI
			ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(200, 120), ImGuiCond_Once);
			ImGui::Begin("Game Info", nullptr, ImGuiWindowFlags_NoResize);

			// タイマー表示
			ImGui::Text("Time: %.2f sec", elapsedTime_);
			ImGui::Text("Best Time: %.2f sec", bestTime_ < 999.0f ? bestTime_ : 0.0f);

			// プレイヤー位置
			const Vector3& playerPos = playerController_->GetPosition();
			ImGui::Text("Pos: %.1f, %.1f, %.1f", playerPos.x, playerPos.y, playerPos.z);

			ImGui::End();
			break;
		}

		case GameState::Paused:
		{
			// 一時停止中のUI
			ImGui::SetNextWindowPos(ImVec2(WinApp::kClientWidth / 2 - 100, WinApp::kClientHeight / 2 - 50), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Once);
			ImGui::Begin("Paused", nullptr, ImGuiWindowFlags_NoResize);
			ImGui::SetWindowFontScale(1.5f);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Game Paused");
			ImGui::SetWindowFontScale(1.0f);
			ImGui::Text("Press ESC to continue");
			ImGui::End();
			break;
		}

		case GameState::GameClear:
		{
			// ゲームクリア時のUI
			ImGui::SetNextWindowPos(ImVec2(WinApp::kClientWidth / 2 - 150, WinApp::kClientHeight / 2 - 75), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);
			ImGui::Begin("Game Clear", nullptr, ImGuiWindowFlags_NoResize);
			ImGui::SetWindowFontScale(1.5f);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Congratulations!");
			ImGui::SetWindowFontScale(1.0f);
			ImGui::Text("Your Time: %.2f seconds", elapsedTime_);
			ImGui::Text("Best Time: %.2f seconds", bestTime_);
			ImGui::Text("Press SPACE to return to title");
			ImGui::End();
			break;
		}

		case GameState::GameOver:
		{
			// ゲームオーバー時のUI
			ImGui::SetNextWindowPos(ImVec2(WinApp::kClientWidth / 2 - 150, WinApp::kClientHeight / 2 - 50), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);
			ImGui::Begin("Game Over", nullptr, ImGuiWindowFlags_NoResize);
			ImGui::SetWindowFontScale(1.5f);
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Game Over");
			ImGui::SetWindowFontScale(1.0f);
			ImGui::Text("Press SPACE to return to title");
			ImGui::End();
			break;
		}
		}

		// ImGuiの描画
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());
	}
	catch (const std::exception& e) {
		OutputDebugStringA(("ERROR in DrawImGui: " + std::string(e.what()) + "\n").c_str());
	}
}

// 終了処理
void AthleticGame::Finalize() {
	try {
		// リソースの解放
		stageObjects_.clear();
		stageModels_.clear();
		goalObject_.reset();
		goalModel_.reset();
		titleSprite_.reset();
		gameOverSprite_.reset();
		clearSprite_.reset();
		timerSprite_.reset();
		goalParticleEmitter_.reset();
		jumpPadParticleEmitter_.reset();
		playerController_.reset();

		// デバッグ出力
		OutputDebugStringA("AthleticGame: Successfully finalized\n");
	}
	catch (const std::exception& e) {
		OutputDebugStringA(("ERROR in AthleticGame::Finalize: " + std::string(e.what()) + "\n").c_str());
	}
}
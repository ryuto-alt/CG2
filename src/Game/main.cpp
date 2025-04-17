#include <string>
#include <format>
#include <windows.h>
#pragma comment(lib,"dxguid.lib")
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "RenderingPipeline.h"

#include <numbers>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "D3DResourceCheck.h"
#include "Logger.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "math.h"
#include "Camera.h"
#include "SrvManager.h"  

// パーティクルシステム関連のヘッダー
#include "ParticleManager.h"
#include "ParticleEmitter.h"

// ImGuiの初期化関数
void InitializeImGui(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager) {
    // ImGui初期化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(winApp->GetHwnd());

    // SrvManagerのディスクリプタヒープを使用
    ImGui_ImplDX12_Init(
        dxCommon->GetDevice(),
        2, // SwapChainのバッファ数
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        srvManager->GetDescriptorHeap().Get(),
        srvManager->GetCPUDescriptorHandle(0), // ImGui用に0番を使用
        srvManager->GetGPUDescriptorHandle(0)
    );
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    D3DResourceLeakChecker leakCheck;

    CoInitializeEx(0, COINIT_MULTITHREADED);

    OutputDebugStringA("Hello, DirectX!\n");

    // ポインタ
    WinApp* winApp = nullptr;
    DirectXCommon* dxCommon = nullptr;
    Input* input = nullptr;
    SpriteCommon* spriteCommon = nullptr;
    SrvManager* srvManager = nullptr;

    // WindowsAPI初期化
    winApp = new WinApp;
    winApp->Initialize();

    // DX初期化
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);

    // SRVマネージャの初期化
    srvManager = new SrvManager();
    srvManager->Initialize(dxCommon);

    // テクスチャマネージャの初期化
    TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

    // パーティクルマネージャの初期化
    ParticleManager::GetInstance()->Initialize(dxCommon, srvManager);

    // パーティクルグループの作成
    ParticleManager::GetInstance()->CreateParticleGroup("smoke", "resources/particle/smoke.png");
    ParticleManager::GetInstance()->CreateParticleGroup("fire", "resources/particle/fire.png");
    ParticleManager::GetInstance()->CreateParticleGroup("star", "resources/particle/star.png");

    // ImGuiの初期化
    InitializeImGui(winApp, dxCommon, srvManager);

    // 入力初期化
    input = new Input();
    input->Initialize(winApp);

    // スプライト共通部分の初期化
    spriteCommon = new SpriteCommon();
    spriteCommon->Initialize(dxCommon);

    // カメラの作成と初期化
    Camera* camera = new Camera();
    camera->SetTranslate({ 0.0f, 0.0f, -5.0f });
    Object3dCommon::SetDefaultCamera(camera);

    // カメラの視点から少し前方にパーティクルエミッタを配置
    Vector3 cameraPos = camera->GetTranslate();
    Vector3 cameraFront = { 0.0f, 0.0f, 1.0f }; // カメラは-Z方向を向いている

    // パーティクルエミッタの作成 - カメラのすぐ前に配置（非常に大きなサイズで）
    ParticleEmitter* smokeEmitter = new ParticleEmitter(
        "smoke",
        { cameraPos.x, cameraPos.y, cameraPos.z + 1.0f }, // カメラの1.0m前
        100,                  // 一度に発生する数を増やす
        10.0f,               // 発生頻度（秒間）
        { -0.5f, 0.5f, -0.5f },  // 最小速度
        { 0.5f, 1.0f, 0.5f },    // 最大速度
        { 0.0f, 0.0f, 0.0f },    // 最小加速度
        { 0.0f, 0.5f, 0.0f },    // 最大加速度
        5.0f,                    // 最小開始サイズを非常に大きく
        10.0f,                   // 最大開始サイズを非常に大きく
        7.0f,                    // 最小終了サイズを非常に大きく
        15.0f,                   // 最大終了サイズを非常に大きく
        { 1.0f, 1.0f, 1.0f, 1.0f },  // 最小開始色（白）
        { 1.0f, 1.0f, 1.0f, 1.0f },  // 最大開始色（白）
        { 1.0f, 1.0f, 1.0f, 0.0f },  // 最小終了色（透明）
        { 1.0f, 1.0f, 1.0f, 0.0f }   // 最大終了色（透明）
    );

    ParticleEmitter* fireEmitter = new ParticleEmitter(
        "fire",
        { cameraPos.x + 1.0f, cameraPos.y, cameraPos.z + 1.0f }, // カメラの1.0m前、右に1m
        100,                  // 一度に発生する数を増やす
        20.0f,               // 発生頻度（秒間）
        { -0.2f, 0.5f, -0.2f },  // 最小速度
        { 0.2f, 1.0f, 0.2f },    // 最大速度
        { 0.0f, 0.0f, 0.0f },    // 最小加速度
        { 0.0f, 0.0f, 0.0f },    // 最大加速度
        5.0f,                    // 最小開始サイズを非常に大きく
        8.0f,                    // 最大開始サイズを非常に大きく
        2.0f,                    // 最小終了サイズ
        4.0f,                    // 最大終了サイズ
        { 1.0f, 0.2f, 0.0f, 1.0f },  // 最小開始色（赤橙色）を鮮やかに
        { 1.0f, 0.5f, 0.0f, 1.0f },  // 最大開始色（橙色）を鮮やかに
        { 1.0f, 0.0f, 0.0f, 0.0f },  // 最小終了色（赤、透明）
        { 1.0f, 0.2f, 0.0f, 0.0f }   // 最大終了色（赤橙、透明）
    );

    ParticleEmitter* starEmitter = new ParticleEmitter(
        "star",
        { cameraPos.x - 1.0f, cameraPos.y, cameraPos.z + 1.0f }, // カメラの1.0m前、左に1m
        100,                  // 一度に発生する数を増やす
        5.0f,                // 発生頻度（秒間）
        { -1.0f, -1.0f, -1.0f }, // 最小速度
        { 1.0f, 1.0f, 1.0f },    // 最大速度
        { 0.0f, -0.5f, 0.0f },   // 最小加速度
        { 0.0f, -0.3f, 0.0f },   // 最大加速度
        4.0f,                    // 最小開始サイズを非常に大きく
        6.0f,                    // 最大開始サイズを非常に大きく
        1.0f,                    // 最小終了サイズ
        2.0f,                    // 最大終了サイズ
        { 0.0f, 0.5f, 1.0f, 1.0f },  // 最小開始色（青色）を鮮やかに
        { 0.5f, 0.7f, 1.0f, 1.0f },  // 最大開始色（水色）を鮮やかに
        { 0.0f, 1.0f, 1.0f, 0.0f },  // 最小終了色（シアン、透明）
        { 0.5f, 1.0f, 1.0f, 0.0f },  // 最大終了色（水色、透明）
        0.0f,                        // 最小開始回転
        3.14f * 2.0f,                // 最大開始回転
        -1.0f,                       // 最小回転速度
        1.0f,                        // 最大回転速度
        1.0f,                        // 最小生存時間
        3.0f                         // 最大生存時間
    );

    // カメラの移動速度
    float cameraSpeed = 0.1f;

    // マウスの感度
    float mouseSensitivity = 0.0008f;

    // マウスカーソルの表示状態
    bool showMouseCursor = false;

    // メインループ
    while (true) {
        // Windowsのメッセージ処理
        if (winApp->ProcessMessage()) {
            // ゲームループを抜ける
            break;
        }

        // 入力更新
        input->Update();

        // ImGui開始
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // ImGuiウィンドウ - カメラ設定
        ImGui::Begin("Camera Settings");

        // カメラ操作UI
        ImGui::Text("Camera Controls");
        ImGui::Text("WASD: Move in the direction you're facing");
        ImGui::Text("Mouse: Look around");
        ImGui::Text("SPACE: Move Up | SHIFT: Move Down");
        ImGui::Text("ESC: Toggle mouse cursor visibility");

        // マウスカーソル状態の表示とチェックボックスで切り替え
        if (ImGui::Checkbox("Show Mouse Cursor", &showMouseCursor)) {
            input->SetMouseCursor(showMouseCursor);

            if (!showMouseCursor) {
                // マウスカーソルを非表示にした場合、マウスを中央に戻す
                POINT center;
                center.x = WinApp::kClientWidth / 2;
                center.y = WinApp::kClientHeight / 2;
                ClientToScreen(winApp->GetHwnd(), &center);
                SetCursorPos(center.x, center.y);
            }
        }

        // マウス感度設定
        ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity, 0.001f, 0.01f);

        // カメラ設定
        Vector3 cameraPos = camera->GetTranslate();
        Vector3 cameraRot = camera->GetRotate();

        if (ImGui::DragFloat3("Camera Position", &cameraPos.x, 0.1f)) {
            camera->SetTranslate(cameraPos);
        }

        if (ImGui::DragFloat3("Camera Rotation", &cameraRot.x, 0.01f)) {
            camera->SetRotate(cameraRot);
        }

        float fovY = camera->GetFovY();
        if (ImGui::SliderFloat("Field of View", &fovY, 0.1f, 1.5f)) {
            camera->SetFovY(fovY);
        }

        ImGui::End();

        // パーティクル設定用GUI
        ImGui::Begin("Particle Settings");

        // 煙エミッタの設定
        ImGui::Text("Smoke Emitter");
        Vector3 smokePos = smokeEmitter->GetPosition();
        if (ImGui::DragFloat3("Smoke Position", &smokePos.x, 0.1f)) {
            smokeEmitter->SetPosition(smokePos);
        }
        bool smokeEmitting = smokeEmitter->IsEmitting();
        if (ImGui::Checkbox("Smoke Emitting", &smokeEmitting)) {
            smokeEmitter->SetEmitting(smokeEmitting);
        }
        uint32_t smokeCount = smokeEmitter->GetEmitCount();
        if (ImGui::DragInt("Smoke Count", (int*)&smokeCount, 1, 1, 100)) {
            smokeEmitter->SetEmitCount(smokeCount);
        }
        float smokeRate = smokeEmitter->GetEmitRate();
        if (ImGui::DragFloat("Smoke Rate", &smokeRate, 0.1f, 0.1f, 50.0f)) {
            smokeEmitter->SetEmitRate(smokeRate);
        }

        // 炎エミッタの設定
        ImGui::Text("Fire Emitter");
        Vector3 firePos = fireEmitter->GetPosition();
        if (ImGui::DragFloat3("Fire Position", &firePos.x, 0.1f)) {
            fireEmitter->SetPosition(firePos);
        }
        bool fireEmitting = fireEmitter->IsEmitting();
        if (ImGui::Checkbox("Fire Emitting", &fireEmitting)) {
            fireEmitter->SetEmitting(fireEmitting);
        }
        uint32_t fireCount = fireEmitter->GetEmitCount();
        if (ImGui::DragInt("Fire Count", (int*)&fireCount, 1, 1, 100)) {
            fireEmitter->SetEmitCount(fireCount);
        }
        float fireRate = fireEmitter->GetEmitRate();
        if (ImGui::DragFloat("Fire Rate", &fireRate, 0.1f, 0.1f, 50.0f)) {
            fireEmitter->SetEmitRate(fireRate);
        }

        // 星エミッタの設定
        ImGui::Text("Star Emitter");
        Vector3 starPos = starEmitter->GetPosition();
        if (ImGui::DragFloat3("Star Position", &starPos.x, 0.1f)) {
            starEmitter->SetPosition(starPos);
        }
        bool starEmitting = starEmitter->IsEmitting();
        if (ImGui::Checkbox("Star Emitting", &starEmitting)) {
            starEmitter->SetEmitting(starEmitting);
        }
        uint32_t starCount = starEmitter->GetEmitCount();
        if (ImGui::DragInt("Star Count", (int*)&starCount, 1, 1, 100)) {
            starEmitter->SetEmitCount(starCount);
        }
        float starRate = starEmitter->GetEmitRate();
        if (ImGui::DragFloat("Star Rate", &starRate, 0.1f, 0.1f, 50.0f)) {
            starEmitter->SetEmitRate(starRate);
        }

        // デバッグ情報の表示
        ImGui::Separator();
        ImGui::Text("Debug Info");
        ImGui::Text("Camera Position: %.2f, %.2f, %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
        ImGui::Text("Smoke Position: %.2f, %.2f, %.2f", smokePos.x, smokePos.y, smokePos.z);
        ImGui::Text("Fire Position: %.2f, %.2f, %.2f", firePos.x, firePos.y, firePos.z);
        ImGui::Text("Star Position: %.2f, %.2f, %.2f", starPos.x, starPos.y, starPos.z);
        ImGui::Text("Smoke Particles: %d", ParticleManager::GetInstance()->GetParticleCount("smoke"));
        ImGui::Text("Fire Particles: %d", ParticleManager::GetInstance()->GetParticleCount("fire"));
        ImGui::Text("Star Particles: %d", ParticleManager::GetInstance()->GetParticleCount("star"));

        // デバッグ用ボタン - シンプルな四角形描画テスト
        if (ImGui::Button("Draw Simple Quad Test")) {
            // 次のフレームで単純な四角形を描画するフラグを立てる
            ParticleManager::GetInstance()->DrawSimpleQuad();
        }

        ImGui::End();

        // マウス入力の取得とカメラ回転
        DIMOUSESTATE mouseState;
        if (SUCCEEDED(input->GetMouseState(&mouseState)) && !showMouseCursor) {
            // マウスの移動量を回転に変換
            Vector3 rot = camera->GetRotate();
            rot.x += mouseState.lY * mouseSensitivity; // マウスY移動→X軸回転(上下)
            rot.y += mouseState.lX * mouseSensitivity; // マウスX移動→Y軸回転(左右)

            // 上下の視点移動を制限（-89°～89°）
            if (rot.x > 1.55f) rot.x = 1.55f;
            if (rot.x < -1.55f) rot.x = -1.55f;

            camera->SetRotate(rot);

            // マウスを中央に戻す
            POINT center;
            center.x = WinApp::kClientWidth / 2;
            center.y = WinApp::kClientHeight / 2;
            ClientToScreen(winApp->GetHwnd(), &center);
            SetCursorPos(center.x, center.y);
        }

        // カメラの向きベクトルを計算
        Vector3 rot = camera->GetRotate();
        Vector3 forward = {
            sinf(rot.y),
            0.0f,
            cosf(rot.y)
        };
        Vector3 right = {
            cosf(rot.y),
            0.0f,
            -sinf(rot.y)
        };

        // 正規化（単位ベクトル化）
        float length = sqrtf(forward.x * forward.x + forward.z * forward.z);
        forward.x /= length;
        forward.z /= length;

        length = sqrtf(right.x * right.x + right.z * right.z);
        right.x /= length;
        right.z /= length;

        // キーボードによるカメラ操作（方向ベクトルに沿った移動）
        Vector3 pos = camera->GetTranslate();
        if (input->PushKey(DIK_W)) {
            pos.x += forward.x * cameraSpeed;
            pos.z += forward.z * cameraSpeed;
        }
        if (input->PushKey(DIK_S)) {
            pos.x -= forward.x * cameraSpeed;
            pos.z -= forward.z * cameraSpeed;
        }
        if (input->PushKey(DIK_A)) {
            pos.x -= right.x * cameraSpeed;
            pos.z -= right.z * cameraSpeed;
        }
        if (input->PushKey(DIK_D)) {
            pos.x += right.x * cameraSpeed;
            pos.z += right.z * cameraSpeed;
        }
        // 上下移動の追加
        if (input->PushKey(DIK_SPACE)) {
            pos.y += cameraSpeed; // 上に移動
        }
        if (input->PushKey(DIK_LSHIFT)) {
            pos.y -= cameraSpeed; // 下に移動
        }

        camera->SetTranslate(pos);

        // カメラ移動に合わせて、パーティクルエミッタの位置も更新
        smokeEmitter->SetPosition({ pos.x, pos.y, pos.z + 1.0f });
        fireEmitter->SetPosition({ pos.x + 1.0f, pos.y, pos.z + 1.0f });
        starEmitter->SetPosition({ pos.x - 1.0f, pos.y, pos.z + 1.0f });

        // マウスカーソル表示切替
        if (input->TriggerKey(DIK_ESCAPE)) { // ESCキーでマウスカーソル表示切替
            showMouseCursor = !showMouseCursor;
            input->SetMouseCursor(showMouseCursor);

            if (!showMouseCursor) {
                // マウスを中央に戻す
                POINT center;
                center.x = WinApp::kClientWidth / 2;
                center.y = WinApp::kClientHeight / 2;
                ClientToScreen(winApp->GetHwnd(), &center);
                SetCursorPos(center.x, center.y);
            }
        }

        // エミッタの更新
        smokeEmitter->Update();
        fireEmitter->Update();
        starEmitter->Update();

        // パーティクルマネージャの更新
        ParticleManager::GetInstance()->Update(camera);

        // DirectXの描画準備 - 背景を黒色に変更
        // dxCommon->Begin() の内部で float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 黒色
        dxCommon->Begin();

        // SRVヒープのセット
        srvManager->PreDraw();

        // カメラの更新
        camera->Update();

        // パーティクルの描画
        ParticleManager::GetInstance()->Draw();

        // ImGuiの描画
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

        // 描画終了
        dxCommon->End();
    }

    // ImGuiの解放
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // パーティクルエミッタの解放
    delete smokeEmitter;
    delete fireEmitter;
    delete starEmitter;
    ParticleManager::GetInstance()->Finalize();

    // カメラの解放
    delete camera;

    // 終了処理
    TextureManager::GetInstance()->Finalize();
    delete winApp;
    delete dxCommon;
    delete input;
    delete spriteCommon;
    delete srvManager;

    return 0;
}
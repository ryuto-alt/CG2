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

    // パーティクルグループの作成 - star.pngのみを使用
    ParticleManager::GetInstance()->CreateParticleGroup("star", "resources/particle/star.png");
    ParticleManager::GetInstance()->CreateParticleGroup("star_green", "resources/particle/star.png");
    ParticleManager::GetInstance()->CreateParticleGroup("star_purple", "resources/particle/star.png");

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
    camera->SetTranslate({ 0.670f, 0.0f, -20.0f });
    Object3dCommon::SetDefaultCamera(camera);

    // 青色系の星パーティクル
    ParticleEmitter* blueStarEmitter = new ParticleEmitter(
        "star",  // 元のグループ名
        { -1.0f, 0.5f, -2.0f },   // 固定位置
        10,                        // 一度に発生する数
        10.0f,                     // 発生頻度
        { -1.0f, -1.0f, -1.0f },  // 最小速度
        { 1.0f, 1.0f, 1.0f },     // 最大速度
        { 0.0f, -0.5f, 0.0f },    // 最小加速度
        { 0.0f, -0.3f, 0.0f },    // 最大加速度
        4.0f,                     // 最小開始サイズ
        6.0f,                     // 最大開始サイズ
        1.0f,                     // 最小終了サイズ
        2.0f,                     // 最大終了サイズ
        { 0.0f, 0.5f, 1.0f, 1.0f },  // 最小開始色（青色）
        { 0.5f, 0.7f, 1.0f, 1.0f },  // 最大開始色（水色）
        { 0.0f, 1.0f, 1.0f, 0.0f },  // 最小終了色（シアン、透明）
        { 0.5f, 1.0f, 1.0f, 0.0f },  // 最大終了色（水色、透明）
        0.0f,                        // 最小開始回転
        3.14f * 2.0f,                // 最大開始回転
        -1.0f,                       // 最小回転速度
        1.0f,                        // 最大回転速度
        0.5f,                        // 最小生存時間
        1.0f                         // 最大生存時間
    );

    // 緑色系の星パーティクル - 横方向の動きを強調
    ParticleEmitter* greenStarEmitter = new ParticleEmitter(
        "star_green",  // 新しいグループ名
        { 1.0f, 0.5f, -2.0f },    // 固定位置（中央）
        10,                        // 一度に発生する数
        10.0f,                     // 発生頻度
        { -1.5f, 0.1f, -0.2f },   // 最小速度 - 横方向に強い
        { 1.5f, 0.8f, 0.2f },     // 最大速度 - 横方向に強い
        { 0.0f, 0.1f, 0.0f },     // 最小加速度 - わずかに上向き
        { 0.0f, 0.3f, 0.0f },     // 最大加速度 - わずかに上向き
        3.0f,                     // 最小開始サイズ
        5.0f,                     // 最大開始サイズ
        0.5f,                     // 最小終了サイズ
        1.5f,                     // 最大終了サイズ
        { 0.0f, 0.8f, 0.2f, 1.0f },  // 最小開始色（緑色）
        { 0.2f, 1.0f, 0.5f, 1.0f },  // 最大開始色（明るい緑）
        { 0.1f, 0.7f, 0.3f, 0.0f },  // 最小終了色（緑、透明）
        { 0.3f, 1.0f, 0.6f, 0.0f },  // 最大終了色（明るい緑、透明）
        0.0f,                        // 最小開始回転
        3.14f * 2.0f,                // 最大開始回転
        -2.0f,                       // 最小回転速度 - 速い回転
        2.0f,                        // 最大回転速度 - 速い回転
        0.6f,                        // 最小生存時間
        1.2f                         // 最大生存時間
    );

    // 紫色系の星パーティクル - 爆発的な動き
    ParticleEmitter* purpleStarEmitter = new ParticleEmitter(
        "star_purple",  // 新しいグループ名
        { 3.0f, 0.5f, -2.0f },    // 固定位置（右側）
        10,                        // 一度に発生する数
        10.0f,                     // 発生頻度
        { -2.0f, -2.0f, -2.0f },  // 最小速度 - 全方向に強い
        { 2.0f, 2.0f, 2.0f },     // 最大速度 - 全方向に強い
        { 0.0f, -0.2f, 0.0f },    // 最小加速度 - わずかに下向き - 修正：min <= max になるよう値を入れ替え
        { 0.0f, -0.1f, 0.0f },    // 最大加速度 - わずかに下向き - 修正：min <= max になるよう値を入れ替え
        2.0f,                     // 最小開始サイズ
        4.0f,                     // 最大開始サイズ
        0.2f,                     // 最小終了サイズ - 小さく消えていく
        0.5f,                     // 最大終了サイズ - 小さく消えていく
        { 0.8f, 0.2f, 1.0f, 1.0f },  // 最小開始色（紫色）
        { 1.0f, 0.4f, 1.0f, 1.0f },  // 最大開始色（明るい紫）
        { 0.6f, 0.0f, 0.8f, 0.0f },  // 最小終了色（暗い紫、透明）
        { 1.0f, 0.2f, 1.0f, 0.0f },  // 最大終了色（紫、透明）
        0.0f,                        // 最小開始回転
        3.14f * 2.0f,                // 最大開始回転
        -3.0f,                       // 最小回転速度 - 非常に速い回転
        3.0f,                        // 最大回転速度 - 非常に速い回転
        0.4f,                        // 最小生存時間 - 短め
        0.8f                         // 最大生存時間 - 短め
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

        // 青色の星エミッタの設定
        ImGui::Text("Blue Star Emitter");
        Vector3 bluePos = blueStarEmitter->GetPosition();
        if (ImGui::DragFloat3("Blue Position", &bluePos.x, 0.1f)) {
            blueStarEmitter->SetPosition(bluePos);
        }
        bool blueEmitting = blueStarEmitter->IsEmitting();
        if (ImGui::Checkbox("Blue Emitting", &blueEmitting)) {
            blueStarEmitter->SetEmitting(blueEmitting);
        }
        uint32_t blueCount = blueStarEmitter->GetEmitCount();
        if (ImGui::DragInt("Blue Count", (int*)&blueCount, 1, 1, 10)) {
            blueStarEmitter->SetEmitCount(blueCount);
        }
        float blueRate = blueStarEmitter->GetEmitRate();
        if (ImGui::DragFloat("Blue Rate", &blueRate, 0.1f, 0.1f, 5.0f)) {
            blueStarEmitter->SetEmitRate(blueRate);
        }

        // 緑色の星エミッタの設定
        ImGui::Text("Green Star Emitter");
        Vector3 greenPos = greenStarEmitter->GetPosition();
        if (ImGui::DragFloat3("Green Position", &greenPos.x, 0.1f)) {
            greenStarEmitter->SetPosition(greenPos);
        }
        bool greenEmitting = greenStarEmitter->IsEmitting();
        if (ImGui::Checkbox("Green Emitting", &greenEmitting)) {
            greenStarEmitter->SetEmitting(greenEmitting);
        }
        uint32_t greenCount = greenStarEmitter->GetEmitCount();
        if (ImGui::DragInt("Green Count", (int*)&greenCount, 1, 1, 10)) {
            greenStarEmitter->SetEmitCount(greenCount);
        }
        float greenRate = greenStarEmitter->GetEmitRate();
        if (ImGui::DragFloat("Green Rate", &greenRate, 0.1f, 0.1f, 5.0f)) {
            greenStarEmitter->SetEmitRate(greenRate);
        }

        // 紫色の星エミッタの設定
        ImGui::Text("Purple Star Emitter");
        Vector3 purplePos = purpleStarEmitter->GetPosition();
        if (ImGui::DragFloat3("Purple Position", &purplePos.x, 0.1f)) {
            purpleStarEmitter->SetPosition(purplePos);
        }
        bool purpleEmitting = purpleStarEmitter->IsEmitting();
        if (ImGui::Checkbox("Purple Emitting", &purpleEmitting)) {
            purpleStarEmitter->SetEmitting(purpleEmitting);
        }
        uint32_t purpleCount = purpleStarEmitter->GetEmitCount();
        if (ImGui::DragInt("Purple Count", (int*)&purpleCount, 1, 1, 10)) {
            purpleStarEmitter->SetEmitCount(purpleCount);
        }
        float purpleRate = purpleStarEmitter->GetEmitRate();
        if (ImGui::DragFloat("Purple Rate", &purpleRate, 0.1f, 0.1f, 5.0f)) {
            purpleStarEmitter->SetEmitRate(purpleRate);
        }

        // デバッグ情報の表示
        ImGui::Separator();
        ImGui::Text("Debug Info");
        ImGui::Text("Camera Position: %.2f, %.2f, %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
        ImGui::Text("Blue Star Position: %.2f, %.2f, %.2f", bluePos.x, bluePos.y, bluePos.z);
        ImGui::Text("Green Star Position: %.2f, %.2f, %.2f", greenPos.x, greenPos.y, greenPos.z);
        ImGui::Text("Purple Star Position: %.2f, %.2f, %.2f", purplePos.x, purplePos.y, purplePos.z);
        ImGui::Text("Blue Star Particles: %d", ParticleManager::GetInstance()->GetParticleCount("star"));
        ImGui::Text("Green Star Particles: %d", ParticleManager::GetInstance()->GetParticleCount("star_green"));
        ImGui::Text("Purple Star Particles: %d", ParticleManager::GetInstance()->GetParticleCount("star_purple"));

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
        blueStarEmitter->Update();
        greenStarEmitter->Update();
        purpleStarEmitter->Update();

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
    delete blueStarEmitter;
    delete greenStarEmitter;
    delete purpleStarEmitter;
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
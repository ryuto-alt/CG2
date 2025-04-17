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
#include <random>
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "D3DResourceCheck.h"
#include "Logger.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Mymath.h"

#include "Model.h"
#include "Object3d.h"
#include "Camera.h"
#include "SrvManager.h"
#include "Particle.h"

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
    camera->SetTranslate({ 0.0f, 0.0f, -20.0f });

    // デフォルトカメラとして設定
    Object3dCommon::SetDefaultCamera(camera);

    // モデル（四角形）の読み込み
    Model* quadModel = new Model();
    quadModel->Initialize(dxCommon);
    quadModel->LoadFromObj("resources", "plane.obj");

    // パーティクルシステムの初期化
    Particle* particleSystem = new Particle();
    particleSystem->Initialize(dxCommon, spriteCommon, srvManager, quadModel);

    // テクスチャの設定
    particleSystem->SetTexture("resources/particle.png");

    // パーティクルの作成 (初期値として1000個)
    const uint32_t kParticleCount = 1000;
    particleSystem->CreateParticles(kParticleCount);

    // パーティクル表示設定
    bool showParticles = true;

    // パーティクル数の変更
    uint32_t newParticleCount = kParticleCount;

    // フレームカウンターと時間計測用変数
    uint64_t frameCount = 0;
    std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();

    // メインループ
    while (true) {
        // Windowsのメッセージ処理
        if (winApp->ProcessMessage()) {
            // ゲームループを抜ける
            break;
        }

        // 入力更新
        input->Update();

        // 経過時間計算
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // ImGui開始
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // ImGuiウィンドウ
        ImGui::Begin("Particle System Settings");

        // パーティクル表示設定
        ImGui::Checkbox("Show Particles", &showParticles);

        // パーティクル数変更
        ImGui::InputInt("Particle Count", (int*)&newParticleCount);
        if (ImGui::Button("Update Particle Count") && newParticleCount > 0) {
            particleSystem->CreateParticles(newParticleCount);
        }

        // FPSを表示
        ImGui::Text("Frame Rate: %.1f FPS", 1.0f / deltaTime);

        // カメラ設定
        Vector3 cameraPos = camera->GetTranslate();
        Vector3 cameraRot = camera->GetRotate();

        if (ImGui::DragFloat3("Camera Position", &cameraPos.x, 0.1f)) {
            camera->SetTranslate(cameraPos);
        }

        if (ImGui::DragFloat3("Camera Rotation", &cameraRot.x, 0.01f)) {
            camera->SetRotate(cameraRot);
        }

        ImGui::End();

        // キーボードによるカメラ操作
        if (input->PushKey(DIK_W)) {
            Vector3 pos = camera->GetTranslate();
            pos.z += 0.1f;
            camera->SetTranslate(pos);
        }
        if (input->PushKey(DIK_S)) {
            Vector3 pos = camera->GetTranslate();
            pos.z -= 0.1f;
            camera->SetTranslate(pos);
        }
        if (input->PushKey(DIK_A)) {
            Vector3 pos = camera->GetTranslate();
            pos.x -= 0.1f;
            camera->SetTranslate(pos);
        }
        if (input->PushKey(DIK_D)) {
            Vector3 pos = camera->GetTranslate();
            pos.x += 0.1f;
            camera->SetTranslate(pos);
        }
        if (input->PushKey(DIK_SPACE)) {
            Vector3 pos = camera->GetTranslate();
            pos.y += 0.1f;
            camera->SetTranslate(pos);
        }
        if (input->PushKey(DIK_LSHIFT)) {
            Vector3 pos = camera->GetTranslate();
            pos.y -= 0.1f;
            camera->SetTranslate(pos);
        }

        // カメラの更新
        camera->Update();

        // パーティクルシステムの更新
        if (showParticles) {
            particleSystem->Update(camera->GetViewProjectionMatrix(), deltaTime);
        }

        // DirectXの描画準備
        dxCommon->Begin();

        // SRVヒープのセット
        srvManager->PreDraw();

        // パーティクルの描画
        if (showParticles) {
            particleSystem->Draw();
        }

        // ImGuiの描画
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

        // 描画終了
        dxCommon->End();

        // フレームカウンター更新
        frameCount++;
    }

    // ImGuiの解放
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // パーティクルシステムの解放
    delete particleSystem;

    // モデルの解放
    delete quadModel;

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
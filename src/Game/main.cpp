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
#include "Sprite.h"
#include "TextureManager.h"
#include "math.h"

// 追加のインクルード
#include "Model.h"
#include "Object3d.h"
#include "Camera.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    D3DResourceLeakChecker leakCheck;

    CoInitializeEx(0, COINIT_MULTITHREADED);

    OutputDebugStringA("Hello, DirectX!\n");

    // ポインタ
    WinApp* winApp = nullptr;
    DirectXCommon* dxCommon = nullptr;
    Input* input = nullptr;
    SpriteCommon* spriteCommon = nullptr;

    // WindowsAPI初期化
    winApp = new WinApp;
    winApp->Initialize();

    // DX初期化
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);

    // テクスチャマネージャの初期化
    TextureManager::GetInstance()->Initialize(dxCommon);

    // 入力初期化
    input = new Input();
    input->Initialize(winApp);

    // スプライト共通部分の初期化
    spriteCommon = new SpriteCommon();
    spriteCommon->Initialize(dxCommon);

    // ===== カメラの作成と初期化 =====
    Camera* mainCamera = new Camera();
    mainCamera->SetTranslate({ 0.0f, 1.0f, -5.0f });

    // 2つ目のカメラ（切り替え用）
    Camera* secondCamera = new Camera();
    secondCamera->SetTranslate({ 5.0f, 2.0f, -3.0f });
    secondCamera->SetRotate({ 0.0f, -0.5f, 0.0f });

    // 現在使用中のカメラ
    Camera* currentCamera = mainCamera;

    // デフォルトカメラとして設定
    Object3dCommon::SetDefaultCamera(currentCamera);

    // ===== モデルの読み込み =====
    Model* axisModel = new Model();
    axisModel->Initialize(dxCommon);
    axisModel->LoadFromObj("resources", "axis.obj");

    Model* dragonModel = new Model();
    dragonModel->Initialize(dxCommon);
    dragonModel->LoadFromObj("resources", "dragon.obj");

    Model* planeModel = new Model();
    planeModel->Initialize(dxCommon);
    planeModel->LoadFromObj("resources", "plane.obj");

    // ===== 3Dオブジェクトの作成 =====
    // 座標軸表示用オブジェクト
    Object3d* axisObject = new Object3d();
    axisObject->Initialize(dxCommon, spriteCommon);
    axisObject->SetModel(axisModel);
    axisObject->SetPosition({ 0.0f, 0.0f, 0.0f });
    axisObject->SetScale({ 1.0f, 1.0f, 1.0f });
    // カメラを個別に設定する例
    axisObject->SetCamera(mainCamera);

    // ドラゴンオブジェクト
    Object3d* dragonObject = new Object3d();
    dragonObject->Initialize(dxCommon, spriteCommon);
    dragonObject->SetModel(dragonModel);
    dragonObject->SetPosition({ 0.0f, 0.0f, 0.0f });
    dragonObject->SetScale({ 0.1f, 0.1f, 0.1f }); // ドラゴンは大きいので縮小
    dragonObject->SetRotation({ 0.0f, 0.0f, 0.0f });
    dragonObject->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    // デフォルトカメラを使用する例（明示的に設定しない）

    // 平面オブジェクト（床として使用）
    Object3d* planeObject = new Object3d();
    planeObject->Initialize(dxCommon, spriteCommon);
    planeObject->SetModel(planeModel);
    planeObject->SetPosition({ 0.0f, -1.0f, 0.0f });
    planeObject->SetScale({ 5.0f, 1.0f, 5.0f }); // 床を広げる
    planeObject->SetRotation({ 0.0f, 0.0f, 0.0f });
    planeObject->SetColor({ 0.8f, 0.8f, 0.8f, 1.0f }); // 灰色
    // デフォルトカメラを使用する例（明示的に設定しない）

    // ライト設定
    DirectionalLight light;
    light.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    light.direction = { 0.0f, -1.0f, 1.0f };
    light.intensity = 1.0f;

    // 全オブジェクトにライト設定を適用
    axisObject->SetDirectionalLight(light);
    dragonObject->SetDirectionalLight(light);
    planeObject->SetDirectionalLight(light);

    // 表示するモデルの選択
    bool showAxis = true;
    bool showDragon = true;
    bool showPlane = true;

    // ドラゴンの自動回転
    bool autoRotateDragon = true;
    float dragonRotationSpeed = 0.01f;

    // カメラの移動速度
    float cameraSpeed = 0.1f;

    // オブジェクトリスト（一括操作用）
    std::vector<Object3d*> allObjects = {
        axisObject, dragonObject, planeObject
    };

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

        // ImGuiウィンドウ
        ImGui::Begin("Camera & Object Settings");

        // カメラ切り替え
        if (ImGui::Button("Switch Camera")) {
            if (currentCamera == mainCamera) {
                currentCamera = secondCamera;
            }
            else {
                currentCamera = mainCamera;
            }

            // デフォルトカメラを更新
            Object3dCommon::SetDefaultCamera(currentCamera);

            // ドラゴンと床のカメラを更新（デフォルトカメラを使うオブジェクト）
            // 特に何もしなくても、次のUpdateで新しいデフォルトカメラが使われる
        }

        // 現在のカメラ情報表示
        ImGui::Text("Current Camera: %s", (currentCamera == mainCamera) ? "Main Camera" : "Second Camera");

        // カメラ操作UI
        ImGui::Text("Camera Controls");
        ImGui::Text("WASD: Move Camera | Arrow Keys: Rotate Camera");

        // カメラ設定
        Vector3 cameraPos = currentCamera->GetTranslate();
        Vector3 cameraRot = currentCamera->GetRotate();

        if (ImGui::DragFloat3("Camera Position", &cameraPos.x, 0.1f)) {
            currentCamera->SetTranslate(cameraPos);
        }

        if (ImGui::DragFloat3("Camera Rotation", &cameraRot.x, 0.01f)) {
            currentCamera->SetRotate(cameraRot);
        }

        float fovY = currentCamera->GetFovY();
        if (ImGui::SliderFloat("Field of View", &fovY, 0.1f, 1.5f)) {
            currentCamera->SetFovY(fovY);
        }

        // モデル表示設定
        ImGui::Separator();
        ImGui::Text("Display Settings");
        ImGui::Checkbox("Show Axis", &showAxis);
        ImGui::Checkbox("Show Dragon", &showDragon);
        ImGui::Checkbox("Show Plane", &showPlane);

        // ドラゴン設定
        ImGui::Separator();
        ImGui::Text("Dragon Settings");

        Vector3 dragonPos = dragonObject->GetPosition();
        if (ImGui::DragFloat3("Dragon Position", &dragonPos.x, 0.1f)) {
            dragonObject->SetPosition(dragonPos);
        }

        Vector3 dragonRot = dragonObject->GetRotation();
        if (ImGui::DragFloat3("Dragon Rotation", &dragonRot.x, 0.01f)) {
            dragonObject->SetRotation(dragonRot);
        }

        ImGui::Checkbox("Auto Rotate Dragon", &autoRotateDragon);
        if (autoRotateDragon) {
            ImGui::SliderFloat("Rotation Speed", &dragonRotationSpeed, 0.001f, 0.05f);
        }

        ImGui::End();

        // キーボードによるカメラ操作
        if (input->PushKey(DIK_W)) {
            Vector3 pos = currentCamera->GetTranslate();
            pos.z += cameraSpeed;
            currentCamera->SetTranslate(pos);
        }
        if (input->PushKey(DIK_S)) {
            Vector3 pos = currentCamera->GetTranslate();
            pos.z -= cameraSpeed;
            currentCamera->SetTranslate(pos);
        }
        if (input->PushKey(DIK_A)) {
            Vector3 pos = currentCamera->GetTranslate();
            pos.x -= cameraSpeed;
            currentCamera->SetTranslate(pos);
        }
        if (input->PushKey(DIK_D)) {
            Vector3 pos = currentCamera->GetTranslate();
            pos.x += cameraSpeed;
            currentCamera->SetTranslate(pos);
        }

        if (input->PushKey(DIK_UP)) {
            Vector3 rot = currentCamera->GetRotate();
            rot.x += 0.01f;
            currentCamera->SetRotate(rot);
        }
        if (input->PushKey(DIK_DOWN)) {
            Vector3 rot = currentCamera->GetRotate();
            rot.x -= 0.01f;
            currentCamera->SetRotate(rot);
        }
        if (input->PushKey(DIK_LEFT)) {
            Vector3 rot = currentCamera->GetRotate();
            rot.y -= 0.01f;
            currentCamera->SetRotate(rot);
        }
        if (input->PushKey(DIK_RIGHT)) {
            Vector3 rot = currentCamera->GetRotate();
            rot.y += 0.01f;
            currentCamera->SetRotate(rot);
        }

        // ドラゴンの自動回転
        if (autoRotateDragon && showDragon) {
            Vector3 rotation = dragonObject->GetRotation();
            rotation.y += dragonRotationSpeed;
            dragonObject->SetRotation(rotation);
        }

        // DirectXの描画準備
        dxCommon->Begin();

        // カメラの更新
        mainCamera->Update();
        secondCamera->Update();

        // 3Dオブジェクトの更新と描画
        if (showAxis) {
            // axisObjectには明示的にカメラが設定されている
            axisObject->Update();
            axisObject->Draw();
        }

        if (showDragon) {
            // dragonObjectはデフォルトカメラを使用
            dragonObject->Update();
            dragonObject->Draw();
        }

        if (showPlane) {
            // planeObjectはデフォルトカメラを使用
            planeObject->Update();
            planeObject->Draw();
        }

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

    // オブジェクトの解放
    delete axisObject;
    delete dragonObject;
    delete planeObject;

    // モデルの解放
    delete axisModel;
    delete dragonModel;
    delete planeModel;

    // カメラの解放
    delete mainCamera;
    delete secondCamera;

    // 終了処理
    TextureManager::GetInstance()->Finalize();
    delete winApp;
    delete dxCommon;
    delete input;
    delete spriteCommon;

    return 0;
}
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

    // マウスの感度
    float mouseSensitivity = 0.0008f;

    // マウスカーソルの表示状態
    bool showMouseCursor = false;

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
        ImGui::Text("WASD: Move in the direction you're facing");
        ImGui::Text("Mouse: Look around");
        ImGui::Text("SPACE: Move Up | SHIFT: Move Down");
        ImGui::Text("TAB: Toggle mouse cursor visibility");

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
        if (ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity, 0.001f, 0.01f)) {
            // スライダーで値が変更された場合、自動的にmouseSensitivity変数が更新される
        }

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

        // マウス入力の取得とカメラ回転
        DIMOUSESTATE mouseState;
        if (SUCCEEDED(input->GetMouseState(&mouseState)) && !showMouseCursor) {
            // マウスの移動量を回転に変換
            Vector3 rot = currentCamera->GetRotate();
            rot.x += mouseState.lY * mouseSensitivity; // マウスY移動→X軸回転(上下)
            rot.y += mouseState.lX * mouseSensitivity; // マウスX移動→Y軸回転(左右)

            // 上下の視点移動を制限（-89°～89°）
            if (rot.x > 1.55f) rot.x = 1.55f;
            if (rot.x < -1.55f) rot.x = -1.55f;

            currentCamera->SetRotate(rot);

            // マウスを中央に戻す
            POINT center;
            center.x = WinApp::kClientWidth / 2;
            center.y = WinApp::kClientHeight / 2;
            ClientToScreen(winApp->GetHwnd(), &center);
            SetCursorPos(center.x, center.y);
        }

        // カメラの向きベクトルを計算
        Vector3 rot = currentCamera->GetRotate();
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
        Vector3 pos = currentCamera->GetTranslate();
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

        currentCamera->SetTranslate(pos);

        // マウスカーソル表示切替
        if (input->TriggerKey(DIK_ESCAPE)) { // TABキーでマウスカーソル表示切替
            showMouseCursor = !showMouseCursor;
            input->SetMouseCursor(showMouseCursor);

            // カーソル表示中はマウス操作を無効化
            if (showMouseCursor) {
                // カーソル表示時は通常のウィンドウモードに
                // 必要に応じてここでカメラ操作の一時停止処理を追加
            }
            else {
                // カーソル非表示時はゲームモードに戻す
                // 必要に応じてここでカメラ操作の再開処理を追加

                // マウスを中央に戻す
                POINT center;
                center.x = WinApp::kClientWidth / 2;
                center.y = WinApp::kClientHeight / 2;
                ClientToScreen(winApp->GetHwnd(), &center);
                SetCursorPos(center.x, center.y);
            }
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
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

// 新しく追加したクラス
#include "Model.h"
#include "Object3d.h"

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

    Model* multiMaterialModel = new Model();
    multiMaterialModel->Initialize(dxCommon);
    multiMaterialModel->LoadFromObj("resources", "multiMaterial.obj");

    // ===== 3Dオブジェクトの作成 =====
    // 座標軸表示用オブジェクト
    Object3d* axisObject = new Object3d();
    axisObject->Initialize(dxCommon, spriteCommon);
    axisObject->SetModel(axisModel);
    axisObject->SetPosition({ 0.0f, 0.0f, 0.0f });
    axisObject->SetScale({ 1.0f, 1.0f, 1.0f });

    // ドラゴンオブジェクト
    Object3d* dragonObject = new Object3d();
    dragonObject->Initialize(dxCommon, spriteCommon);
    dragonObject->SetModel(dragonModel);
    dragonObject->SetPosition({ 0.0f, 0.0f, 0.0f });
    dragonObject->SetScale({ 0.1f, 0.1f, 0.1f }); // ドラゴンは大きいので縮小
    dragonObject->SetRotation({ 0.0f, 0.0f, 0.0f });
    dragonObject->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    // 平面オブジェクト（床として使用）
    Object3d* planeObject = new Object3d();
    planeObject->Initialize(dxCommon, spriteCommon);
    planeObject->SetModel(planeModel);
    planeObject->SetPosition({ 0.0f, -1.0f, 0.0f });
    planeObject->SetScale({ 5.0f, 1.0f, 5.0f }); // 床を広げる
    planeObject->SetRotation({ 0.0f, 0.0f, 0.0f });
    planeObject->SetColor({ 0.8f, 0.8f, 0.8f, 1.0f }); // 灰色

    // マルチマテリアルオブジェクト
    const int kMultiMaterialCount = 3;
    std::vector<Object3d*> multiMaterialObjects;

    for (int i = 0; i < kMultiMaterialCount; i++) {
        Object3d* object = new Object3d();
        object->Initialize(dxCommon, spriteCommon);
        object->SetModel(multiMaterialModel);

        // 位置をずらして並べる
        float posX = static_cast<float>(i) * 2.0f - 2.0f;
        object->SetPosition({ posX, 1.0f, 0.0f });
        object->SetScale({ 1.0f, 1.0f, 1.0f });

        // 色を変える
        Vector4 color;
        if (i == 0) color = { 1.0f, 0.3f, 0.3f, 1.0f }; // 赤っぽい
        else if (i == 1) color = { 0.3f, 1.0f, 0.3f, 1.0f }; // 緑っぽい
        else color = { 0.3f, 0.3f, 1.0f, 1.0f }; // 青っぽい

        object->SetColor(color);
        multiMaterialObjects.push_back(object);
    }

    //// スプライトの作成
    //std::vector<std::string> textureFilePaths = {
    //    "Resources/monsterBall.png",
    //    "Resources/uvChecker.png",
    //    "Resources/kao.png"
    //};

    std::vector<Sprite*> sprites;

   /* for (uint32_t i = 0; i < textureFilePaths.size(); ++i) {
        Sprite* sprite = new Sprite();
        sprite->Initialize(spriteCommon, textureFilePaths[i]);
        sprites.push_back(sprite);
    }*/

    // スプライトの初期設定
    for (int i = 0; i < sprites.size(); i++) {
        Vector2 position = { 100.0f + i * 200.0f, 100.0f };
        Vector2 size = { 100.0f, 100.0f };

        sprites[i]->SetPosition(position);
        sprites[i]->SetSize(size);
        sprites[i]->SetAnchorPoint({ 0.5f, 0.5f });
        sprites[i]->SetTextureLeftTop({ 0.0f, 0.0f });
        sprites[i]->SetTextureSize({ 256.0f, 256.0f });
    }

    // カメラ設定
    Transform cameraTransform = {
        {1.0f, 1.0f, 1.0f},  // スケール
        {0.0f, 0.0f, 0.0f},  // 回転
        {0.0f, 1.0f, -5.0f}  // 位置
    };

    // ライト設定
    DirectionalLight light;
    light.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    light.direction = { 0.0f, -1.0f, 1.0f };
    light.intensity = 1.0f;

    // 全オブジェクトにライト設定を適用
    axisObject->SetDirectionalLight(light);
    dragonObject->SetDirectionalLight(light);
    planeObject->SetDirectionalLight(light);
    for (auto& obj : multiMaterialObjects) {
        obj->SetDirectionalLight(light);
    }

    // 表示するモデルの選択
    bool showAxis = true;
    bool showDragon = true;
    bool showPlane = true;
    bool showMultiMaterial = true;

    // ドラゴンの自動回転
    bool autoRotateDragon = true;
    float dragonRotationSpeed = 0.01f;

    // メインループ
    while (true) {
        // Windowsのメッセージ処理
        if (winApp->ProcessMessage()) {
            // ゲームループを抜ける
            break;
        }

        // 入力更新
        input->Update();

        // スプライト更新
        for (Sprite* sprite : sprites) {
            sprite->Update();
        }

        // ImGui開始
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // ImGuiウィンドウ
        ImGui::Begin("3D Settings");

        // モデル表示設定
        if (ImGui::CollapsingHeader("Display Settings")) {
            ImGui::Checkbox("Show Axis", &showAxis);
            ImGui::Checkbox("Show Dragon", &showDragon);
            ImGui::Checkbox("Show Plane", &showPlane);
            ImGui::Checkbox("Show Multi-Material Objects", &showMultiMaterial);
        }

        // カメラ設定ウィンドウ
        if (ImGui::CollapsingHeader("Camera")) {
            ImGui::DragFloat3("Camera Position", &cameraTransform.translate.x, 0.1f);
            ImGui::DragFloat3("Camera Rotation", &cameraTransform.rotate.x, 0.01f);
        }

        // ライト設定ウィンドウ
        if (ImGui::CollapsingHeader("Light")) {
            ImGui::ColorEdit4("Light Color", &light.color.x);
            ImGui::DragFloat3("Light Direction", &light.direction.x, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("Light Intensity", &light.intensity, 0.01f, 0.0f, 2.0f);

            if (ImGui::Button("Apply Light Settings")) {
                // 全オブジェクトにライト設定を適用
                axisObject->SetDirectionalLight(light);
                dragonObject->SetDirectionalLight(light);
                planeObject->SetDirectionalLight(light);
                for (auto& obj : multiMaterialObjects) {
                    obj->SetDirectionalLight(light);
                }
            }
        }

        // ドラゴン設定ウィンドウ
        if (ImGui::CollapsingHeader("Dragon Settings")) {
            Vector3 position = dragonObject->GetPosition();
            if (ImGui::DragFloat3("Position", &position.x, 0.1f)) {
                dragonObject->SetPosition(position);
            }

            Vector3 rotation = dragonObject->GetRotation();
            if (ImGui::DragFloat3("Rotation", &rotation.x, 0.01f)) {
                dragonObject->SetRotation(rotation);
            }

            Vector3 scale = dragonObject->GetScale();
            if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.01f, 1.0f)) {
                dragonObject->SetScale(scale);
            }

            Vector4 color = dragonObject->GetColor();
            if (ImGui::ColorEdit4("Color", &color.x)) {
                dragonObject->SetColor(color);
            }

            ImGui::Checkbox("Auto Rotate", &autoRotateDragon);
            if (autoRotateDragon) {
                ImGui::DragFloat("Rotation Speed", &dragonRotationSpeed, 0.001f, 0.001f, 0.1f);
            }
        }

        // マルチマテリアルオブジェクト設定ウィンドウ
        if (ImGui::CollapsingHeader("Multi-Material Objects")) {
            for (int i = 0; i < multiMaterialObjects.size(); i++) {
                std::string label = "Object " + std::to_string(i + 1);
                if (ImGui::TreeNode(label.c_str())) {
                    // 位置設定
                    Vector3 position = multiMaterialObjects[i]->GetPosition();
                    if (ImGui::DragFloat3("Position", &position.x, 0.1f)) {
                        multiMaterialObjects[i]->SetPosition(position);
                    }

                    // 回転設定
                    Vector3 rotation = multiMaterialObjects[i]->GetRotation();
                    if (ImGui::DragFloat3("Rotation", &rotation.x, 0.01f)) {
                        multiMaterialObjects[i]->SetRotation(rotation);
                    }

                    // スケール設定
                    Vector3 scale = multiMaterialObjects[i]->GetScale();
                    if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.1f, 10.0f)) {
                        multiMaterialObjects[i]->SetScale(scale);
                    }

                    // 色設定
                    Vector4 color = multiMaterialObjects[i]->GetColor();
                    if (ImGui::ColorEdit4("Color", &color.x)) {
                        multiMaterialObjects[i]->SetColor(color);
                    }

                    // ライティング有効/無効
                    bool enableLighting = multiMaterialObjects[i]->GetEnableLighting();
                    if (ImGui::Checkbox("Enable Lighting", &enableLighting)) {
                        multiMaterialObjects[i]->SetEnableLighting(enableLighting);
                    }

                    ImGui::TreePop();
                }
            }
        }

        ImGui::End();
        ImGui::Render();

        // DirectXの描画準備
        dxCommon->Begin();

        // スプライトの描画
        spriteCommon->CommonDraw();
        for (Sprite* sprite : sprites) {
            sprite->Draw();
        }

        // ビュー行列の作成
        Matrix4x4 viewMatrix = MakeAffineMatrix(
            cameraTransform.scale,
            cameraTransform.rotate,
            cameraTransform.translate);
        viewMatrix = Inverse(viewMatrix);

        // プロジェクション行列の作成
        Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
            0.45f,
            static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight),
            0.1f,
            100.0f);

        // 3Dオブジェクトの更新と描画

        // 座標軸の更新と描画
        if (showAxis) {
            axisObject->Update(viewMatrix, projectionMatrix);
            axisObject->Draw();
        }

        // ドラゴンの更新と描画
        if (showDragon) {
            // 自動回転
            if (autoRotateDragon) {
                Vector3 rotation = dragonObject->GetRotation();
                rotation.y += dragonRotationSpeed;
                dragonObject->SetRotation(rotation);
            }

            dragonObject->Update(viewMatrix, projectionMatrix);
            dragonObject->Draw();
        }

        // 平面（床）の更新と描画
        if (showPlane) {
            planeObject->Update(viewMatrix, projectionMatrix);
            planeObject->Draw();
        }

        // マルチマテリアルオブジェクトの更新と描画
        if (showMultiMaterial) {
            for (Object3d* object : multiMaterialObjects) {
                // オブジェクトを少しずつ回転させる
                Vector3 rotation = object->GetRotation();
                rotation.y += 0.01f;
                object->SetRotation(rotation);

                // 行列の更新と描画
                object->Update(viewMatrix, projectionMatrix);
                object->Draw();
            }
        }

        // ImGuiの描画
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

        // 描画終了
        dxCommon->End();
    }

    // ImGuiの解放
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // 3Dオブジェクトの解放
    delete axisObject;
    delete dragonObject;
    delete planeObject;
    for (Object3d* object : multiMaterialObjects) {
        delete object;
    }

    // モデルの解放
    delete axisModel;
    delete dragonModel;
    delete planeModel;
    delete multiMaterialModel;

    // スプライトの解放
    for (Sprite* sprite : sprites) {
        delete sprite;
    }

    // 終了処理
    TextureManager::GetInstance()->Finalize();
    delete winApp;
    delete dxCommon;
    delete input;
    delete spriteCommon;

    return 0;
}
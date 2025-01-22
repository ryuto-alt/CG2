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


#pragma region MaterialData
MaterialData LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename) {

	MaterialData materialData;//構築するMaterialData
	std::string line;//ファイルから読んだ1行を格納するもの
	std::ifstream file(directorypath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなっかたら止める
	while (std::getline(file, line)) {
		std::string identifile;
		std::stringstream s(line);
		s >> identifile;

		//identifierの応じた処理
		if (identifile == "map_Kd") {

			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directorypath + "/" + textureFilename;
		}
	}
	return materialData;
}
#pragma endregion


#pragma region LoadObjeFil関数
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {

	ModelData modelData;//構築するModekData
	std::vector<Vector4>positions;//位置
	std::vector<Vector3>normals;//法線
	std::vector<Vector2>texcoords;//テクスチャ座標
	std::string line;//ファイルから読んだ1行を格納するもの

	//ファイル読み込み
	std::ifstream file(directoryPath + "/" + filename);//faileを開く
	assert(file.is_open());//開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//先頭の識別子を読む

		if (identifier == "v") {

			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			position.x *= -1;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1 - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {

			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1;
			normals.push_back(normal);
		}
		else if (identifier == "f") {

			VertexData triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置・UV・法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');//区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				//VertexData veretex = { position,texcoord,normal };
				//modelData.vertices.push_back(veretex);
				triangle[faceVertex] = { position,texcoord,normal };

			}
			//頂点を逆順で登録刷ることで、周り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);

		}
		else if (identifier == "mtllib") {

			//materialTemlateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}
#pragma endregion 


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;

	CoInitializeEx(0, COINIT_MULTITHREADED);

	OutputDebugStringA("HEllo,DirectX!\n");

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

	//テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon);

	// 入力初期化
	input = new Input();
	input->Initialize(winApp);

	// スプライト共通部分の初期化
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);


	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;


#pragma region Resource
	const uint32_t kSubdivision = 512;
	ModelData modelData = LoadObjFile("resources", "axis.obj");

#pragma region VertexResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * kSubdivision * kSubdivision * 6);

#pragma region ModelResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceModel = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
#pragma endregion


#pragma region vertexResourceModel頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW VertexBufferViewModel{};
	VertexBufferViewModel.BufferLocation = vertexResourceModel->GetGPUVirtualAddress();
	VertexBufferViewModel.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	VertexBufferViewModel.StrideInBytes = sizeof(VertexData);
	VertexData* vertexDataModel = nullptr;
	vertexResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataModel));
	std::memcpy(vertexDataModel, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
#pragma endregion


#pragma region vertexResource頂点バッファーを作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{ };
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 6;
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
#pragma endregion


#pragma region 基準点
	//経度分割1つ分の経度φd
	const float kLonEvery = 2 * std::numbers::pi_v<float> / (float)kSubdivision;
	//緯度分割１つ分の緯度Θd
	const float kLatEvery = std::numbers::pi_v<float> / (float)kSubdivision;
	//緯度方向に分割しながら線を描く
	const float w = 2.0f;
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;//θ
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			//テクスチャ用のTexcoord

			//書き込む最初の場所
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;
			//基準点a
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) };
			//基準点b
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex + 1.0f) / float(kSubdivision) };
			//基準点c
			start++;
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1.0f) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) };

			//基準点c
			start++;
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1.0f) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) };

			//基準点b
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex + 1.0f) / float(kSubdivision) };

			//基準点d
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1) / float(kSubdivision), 1.0f - float(latIndex + 1) / float(kSubdivision) };
		}
	}
#pragma endregion


#pragma region Material用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む	
	Material* materialDataSphere = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	//色
	materialDataSphere->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialDataSphere->enableLighting = true;
	materialDataSphere->uvTransform = MakeIdentity4x4();
#pragma endregion


#pragma region WVP用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込む
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();
#pragma endregion


#pragma region Model用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceModel = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む	
	Material* materialDataModel = nullptr;
	materialResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&materialDataModel));
	//色
	materialDataModel->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialDataModel->enableLighting = true;
	materialDataModel->uvTransform = MakeIdentity4x4();
#pragma endregion


#pragma region ModelTransform用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceModel = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataModel = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataModel));
	//単位行列を書き込む
	transformationMatrixDataModel->WVP = MakeIdentity4x4();
	transformationMatrixDataModel->World = MakeIdentity4x4();
#pragma endregion


#pragma region 平行光源用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,1.0f };
	directionalLightData->intensity = 1.0f;
#pragma endregion


#pragma region Texturを読む
	std::string textureFilePath[2]{ "Resources/monsterBall.png" ,"Resources/uvChecker.png" };
#pragma endregion 


	std::vector<Sprite*>sprites;
	for (uint32_t i = 0; i < 10; ++i) {
		Sprite* sprite = new Sprite();
		sprite->Initialize(spriteCommon, textureFilePath[1]);
		sprites.push_back(sprite);
	}

	int i = 0;
	for (Sprite* sprite : sprites) {
		Vector2 position = sprite->GetPosition();
		Vector2 size = sprite->GetSize();

		position.x = 200.0f * i;
		position.y = 200.0f;
		size = Vector2(100, 100);

		sprite->SetPosition(position);
		sprite->SetSize(size);
		sprite->SetAnchorPoint(Vector2{ 0.0f,0.0f });
		sprite->SetIsFlipY(0);
		sprite->SetTextureLeftTop(Vector2{ i * 64.0f,0.0f });
		sprite->SetTextureSize(Vector2{ 64.0f,64.0f });
		i++;
	}

	Vector2 rotation{ 0 };


#pragma region Transform変数
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,-1.5f,0.0f},{0.0f,0.0f,0.0f } };

#pragma region cameraTransform変数
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-5.0f} };

#pragma region uvspriteTransform変数
	Transform uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

#pragma region model変数
	Transform transformModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
#pragma endregion

	bool useMonsterBall = false;

	while (true) {
		//Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}

		input->Update();

		////ゲームの処理
		//if (input->PushKey(DIK_A)) {
		//	transformModel.translate.x -= 0.01f;
		//}
		//if (input->PushKey(DIK_D)) {
		//	transformModel.translate.x += 0.01f;
		//}
		//if (input->PushKey(DIK_W)) {
		//	transformModel.translate.y += 0.01f;
		//}
		//if (input->PushKey(DIK_S)) {
		//	transformModel.translate.y -= 0.01f;
		//}

/*#pragma region Transform用Matrix
		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);

		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		wvpData->WVP = worldViewProjectionMatrix;
		wvpData->World = worldMatrix;
#pragma endregion


#pragma region model用Matrix
		Matrix4x4 worldMatrixmodel = MakeAffineMatrix(transformModel.scale, transformModel.rotate, transformModel.translate);
		Matrix4x4 worldViewProjectionMatrixModel = Multiply(worldMatrixmodel, Multiply(viewMatrix, projectionMatrix));
		transformationMatrixDataModel->WVP = worldViewProjectionMatrixModel;
		transformationMatrixDataModel->World = worldMatrixmodel;
#pragma endregion*/


		for (Sprite* sprite : sprites) {
			sprite->Update();
		}


		/*#pragma region material用Matrix
				Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
				uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
				uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
				materialDataSprite->uvTransform = uvTransformMatrix;
		#pragma endregion*/


		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Settings");

		// Color Edit ウィンドウ
		if (ImGui::CollapsingHeader("SetColor")) {
			ImGui::ColorEdit4("materialData", &materialDataSphere->color.x);
		}
		ImGui::Separator();

		// Texture変更
		if (ImGui::CollapsingHeader("Texture change")) {
			ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		}
		ImGui::Separator();

		// Lighting
		if (ImGui::CollapsingHeader("Lighting")) {
			ImGui::ColorEdit4("LightSetColor", &directionalLightData->color.x);
			ImGui::DragFloat3("directionalLight", &directionalLightData->direction.x, 0.01f, -1.0f, 1.0f);
		}
		ImGui::Separator();

		// スフィアウィンドウ
		if (ImGui::CollapsingHeader("3DObject")) {
			ImGui::DragFloat3("Translation", &transform.translate.x, 0.01f);
			ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.01f);
			ImGui::DragFloat2("Scale", &transform.scale.x, 0.01f);
			if (ImGui::Button("Reset Transform")) {
				transform = { {1.0f, 1.0f, 1.0f}, {0.0f, -1.5f, 0.0f}, {0.0f, 0.0f, 0.0f} };
			}
		}
		ImGui::Separator();

		// モデルウィンドウ
		if (ImGui::CollapsingHeader("Model"))
		{
			ImGui::DragFloat3("ModelTranslate", &transformModel.translate.x, 0.01f);
			ImGui::DragFloat3("ModelRotate", &transformModel.rotate.x, 0.01f);
			ImGui::DragFloat3("ModelScale", &transformModel.scale.x, 0.01f);
			if (ImGui::Button("Reset Transform")) {
				transformModel = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
			}
		}
		ImGui::Separator();

		//// スプライトウィンドウ
		//if (ImGui::CollapsingHeader("2DSprite")) {
		//	Vector2 size = sprite->GetSize();
		//	Vector2 position = sprite->GetPosition();
		//	float rotation = sprite->GetRotation();
		//	Vector4 spritecolor = sprite->GetColor();
		// 
		//	ImGui::ColorEdit4("*spriteColor", &spritecolor.x);
		//	ImGui::DragFloat2("*ScaleSprite", &size.x, 0.1f);
		//	ImGui::DragFloat("*RotateSprite", &rotation, 0.1f);
		//	ImGui::DragFloat2("*TranslateSprite", &position.x);
		// 
		//	sprite->setColor(spritecolor);
		//	sprite->SetPosition(position);
		//	sprite->SetRotation(rotation);
		//	sprite->SetSize(size);
		// 
		//	if (ImGui::Button("Reset Transform")) {
		//		transformSprite = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
		//	}
		//}
		//ImGui::Separator();

		// UVTransform
		if (ImGui::CollapsingHeader("UVTransform")) {
			ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
		}
		ImGui::Separator();


		ImGui::End();
		ImGui::Render();

		//DirectXの描画準備
		dxCommon->Begin();
		//Spriteの描画準備
		spriteCommon->CommonDraw();

		for (Sprite* sprite : sprites) {
			sprite->Draw();
		}

		//#pragma region スフィアの描画
		//		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
		//		//現状を設定。POSに設定しているものとはまた別。おなじ物を設定すると考えておけばいい
		//		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		//		//wvp用のCBufferの場所を設定
		//		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
		//		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		//		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		//		//描画！
		//		//dxCommon->GetCommandList()->DrawInstanced(kSubdivision * kSubdivision * 6, 1, 0, 0);
		//#pragma endregion


		//#pragma region Modelの描画
		//		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &VertexBufferViewModel);
		//		//wvp用のCBufferの場所を設定
		//		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceModel->GetGPUVirtualAddress());
		//		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU3);
		//		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		//		//描画！
		//		//dxCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
		//#pragma endregion


		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

		dxCommon->End();
	}

#pragma region 解放処理

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	/*CloseHandle(fenceEvent);*/

#ifdef _DEBUG
#endif //_DEBUG
#pragma endregion

	// 終了処理
	winApp->Finalize();
	// 解放処理
	TextureManager::GetInstance()->Finalize();
	delete winApp;
	delete dxCommon;
	delete input;
	delete spriteCommon;
	for (Sprite* sprite : sprites) {
		delete sprite; // 各Spriteオブジェクトの削除
	}

	return 0;
}
#include "GameScene.h"
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

void GameScene::Initialize() {

	// モデルのロード
	player_model_ = Model::CreateFromOBJ("player");

	// カメラ初期化
	camera_.Initialize();

	  // カメラ位置をプレイヤーに近づける
	camera_.translation_ = {0.0f, 0.0f, -10.0f}; // Y:高さ、Z:奥行き
	camera_.UpdateMatrix();                     // 行列更新


	// デバッグカメラ作成
	debugCamera_ = new DebugCamera(1280, 720);


	// プレイヤー初期化（座標など）
	player_ = new Player();
	player_->Initialize(player_model_, &camera_, {0.0f, 0.0f, 0.0f});
	
}

void GameScene::Update() {

#ifdef _DEBUG
	//// スペースキーでデバッグカメラ切り替え
	//if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
	//	isDebugCameraActive_ = !isDebugCameraActive_;
	//}
#endif

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
	}

	// プレイヤー更新
	player_->Update();
}

void GameScene::Draw() {

	// DirectX共通処理取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	// プレイヤー描画
	player_->Draw();

	// 3Dモデル描画後処理
	Model::PostDraw();
}

void GameScene::Delete() {

	delete player_;
	delete player_model_;
	delete debugCamera_;
}

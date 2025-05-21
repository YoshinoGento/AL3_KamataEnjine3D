#include "GameScene.h"
#include "ImageHlp.h"
#include "MapChipField.h"
#include "KamataEngine.h"
#include "Math.h"
using namespace KamataEngine;

void GameScene::Initialize() {

	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("./Resources/miyazaki.png");

	// スプライトインスタントの生成
	sprite_ = Sprite::Create(textureHandle_, {100, 50});

	// 3Dモデルデータの生成
	block_model_ = Model::Create();
	model_ = Model::Create();

	//// 要素数
	//const uint32_t kNumBlockVirtical = 10;
	//const uint32_t kNumBlockHorizontal = 20;
	//// ブロック一個分の横幅
	//const float kBlockWidth = 2.0f;
	//const float kBlockHeight = 2.0f;
	//// 要素数を変更
	//worldTransformBlocks_.resize(kNumBlockVirtical);

	//for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {

	//	// 1列の要素数を設定
	//	worldTransformBlocks_[i].resize(kNumBlockHorizontal);
	//}

	//// キューブの生成
	//for (uint32_t i = 0; i < kNumBlockVirtical; i++) {
	//	for (uint32_t j = 0; j < kNumBlockHorizontal; j++) {

	//		if ((i + j) % 2 == 0)
	//			continue;
	//		worldTransformBlocks_[i][j] = new WorldTransform();
	//		worldTransformBlocks_[i][j]->Initialize();
	//		worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * j;
	//		worldTransformBlocks_[i][j]->translation_.y = kBlockHeight * i;
	//	}
	//}

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// カメラ
	camera_.Initialize();

	// ライン描画が参照するカメラを指定する(アドレス渡し)
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);

	// デバックカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// 縦方向表示の表示を有効にする
	AxisIndicator::GetInstance()->SetVisible(true);

	// 縦方向表示が参照するビュープロジェクションを指定する（アドレス渡し）
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());

	//マップチップ
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("./Resources/blocks.csv");
	GenerateBlocks();

	//座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 1);

	player_.Initialize(modelPlayer_, &camera_, playerPosition);
	
}

void GameScene::Update() {

	//// スプライトの今の座標を取得
	//Vector2 position = sprite_->GetPosition();
	//// 座標を{2,1}移動
	//position.x += 2.0f;
	//position.y += 1.0f;
	//// 移動した座標をスプライトに反映
	//sprite_->SetPosition(position);

	//// デバックテキストの表示
	//ImGui::Begin("Debug1");

	//// float3入力ボックス
	//ImGui::InputFloat3("InputFloat3", inputFloat3);

	//// float3スライダー
	//ImGui::SliderFloat3("SliderFloat3", inputFloat3, 0.0f, 1.0f);

//#ifdef _DEBUG
//	ImGui::Text("Kamata Tarou %d.%d.%d", 2050, 12, 31);
//#endif
//
//	ImGui::End();
//
//	ImGui::ShowDemoWindow();

	// デバックカメラ更新
	debugCamera_->Update();

#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif
	// カメラの処理
	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		//ビュープロジェクション行列の転送
		camera_.TransferMatrix();
	} else {
		//ビュープロジェクション行列の更新と転送
		camera_.UpdateMatrix();

	}

	// ブロックの更新

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			worldTransformBlock->matWorld_ = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);
			// 定数バッファに転送する
			worldTransformBlock->TransferMatrix();
		}
	}
}

void GameScene::Drow() {

	// DirectXCommonインスタントの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// スプライト描画前処理
	Sprite::PreDraw(dxCommon->GetCommandList());

	// スプライト描画
	sprite_->Draw();

	// スプライト描画後処理
	Sprite::PostDraw();

	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	//// 3Dモデル描画
	// block_model_->Draw(worldTransform_, debugCamera_->GetCamera(), textureHandle_);

	// 3Dモデル描画前処理
	// Model::PostDraw();

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			block_model_->Draw(*worldTransformBlock, camera_, nullptr);
		}
	}
}

GameScene::~GameScene() {
	// 3Dモデルデータの解散
	delete block_model_;
	delete model_;
	delete sprite_;
	delete debugCamera_;
	delete mapChipField_;
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

}

void GameScene::GenerateBlocks() {
	// 要素数
	const uint32_t kNumBlockVirtical = mapChipField_->GetNumBlockVirtical();
	const uint32_t kNumBlockHorizontal = mapChipField_->GetNumBlockHorizontal();
	//// ブロック一個分の横幅
	//const float kBlockWidth = 2.0f;
	//const float kBlockHeight = 2.0f;
	// 要素数を変更
	worldTransformBlocks_.resize(kNumBlockVirtical);

	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {

		// 1列の要素数を設定
		worldTransformBlocks_[i].resize(kNumBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < kNumBlockVirtical; i++) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; j++) {

			if (mapChipField_->GetMapChipTypeByIndex(j,i) == MapChipType::kBlock) {

				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j,i);
			}
		}
	}
	
}

#include "Player.h"
#include "MatrixMath.h"
#include <cmath> 
#include <cassert>
#include <numbers>

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {

	assert(model);
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();

	// 初期位置
	worldTransform_.translation_ = position;

	// サイズ調整
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 初期向き
	// 初期向き → 前向き（Z軸方向）に修正

	//シングルトンインスタンスを取得する
	input_ = Input::GetInstance();
	
}

void Player ::Update() {

	// デスフラグの立った弾を削除
	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});
    //キャラクターの移動ベクトル
	Vector3 move = {0.0f, 0.0f, 0.0f};

	// 移動速度
	const float kCharacterSpeed = 0.01f;


	//const float kAcceleration = 0.01f;
	const float kFriction = 0.9f;
	const float kMaxSpeed = 3.0f;

	//Vector3 acceleration = {0.0f, 0.0f, 0.0f};
	
	if (input_->PushKey(DIK_W)) {// 前へ
		move.y += kCharacterSpeed;
	} 
	if (input_->PushKey(DIK_S)) {// 後ろへ
		move.y -= kCharacterSpeed;
	} 
	if (input_->PushKey(DIK_D)) {// 右へ
		move.x += kCharacterSpeed;
	} 
	if (input_->PushKey(DIK_A)) {// 左へ
		move.x -= kCharacterSpeed;
	} 
	

	velocity_ += move;

	// 座標移動（ベクトルの加算）
	worldTransform_.translation_ += move;


	// 速度の制限
	if (Length(velocity_) > kMaxSpeed) {
		velocity_ = Normalized(velocity_) * kMaxSpeed;
	}

	// 摩擦で減速
	velocity_ *= kFriction;

	// 実際に移動
	worldTransform_.translation_ += velocity_;

	// 移動制限座標
	// X軸（左右）
	const float kMoveLimitX = 6.0f;

	// Y軸（上下）
	const float kMoveLimitY = 3.0f;

	// 移動制限
	worldTransform_.translation_.x = max(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = min(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.y = max(worldTransform_.translation_.y, -kMoveLimitY);
	worldTransform_.translation_.y = min(worldTransform_.translation_.y, +kMoveLimitY);

	// 攻撃
	Attack();

	// 弾の更新
	for (PlayerBullet *bullet : bullets_) {

		bullet->Update();

	}

	// 行列更新
	//worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}



void Player ::Draw() {

	// プレイヤーモデル描画
	model_->Draw(worldTransform_, *camera_);

	// 弾の描画
	for (PlayerBullet *bullet : bullets_) {
		bullet->Draw(*camera_);
	}
}

void Player::Attack() {
	

	if (input_->TriggerKey(DIK_SPACE)) {

		//弾の速度
		const float kBulletSpped = 1.0f;
		Vector3 velocity(0, 0, kBulletSpped);

		//速度ベクトルを自機の向きに合わせて回転させる
		velocity = TransformNormal(velocity, worldTransform_.matWorld_);

	
		
		// 弾を生成し、初期化
		PlayerBullet* newBullet = new PlayerBullet();
		newBullet->Initialize(model_, worldTransform_.translation_,velocity);

		


		// 弾を登録する
		bullets_.push_back(newBullet);

	}

}


void Player::FireToward(const Vector3& targetWorld) {
	
	// 弾の速度
	const float kBulletSpeed = 1.0f;

	// 自機から狙い点への方向ベクトルを計算
	const Vector3& playerPos = worldTransform_.translation_;

	// 方向ベクトル(プレイヤー → 狙い点)
	Vector3 direction = targetWorld - playerPos;

	// 正規化して速度ベクトルに変換
	direction = Normalized(direction);

	//実際の速度
	Vector3 velocity = direction * kBulletSpeed;

	// 弾を生成し、初期化
	PlayerBullet* newBullet = new PlayerBullet();
	newBullet->Initialize(model_, playerPos, velocity);

	// 弾を登録する
	bullets_.push_back(newBullet);
}


Player::~Player() {

	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
}


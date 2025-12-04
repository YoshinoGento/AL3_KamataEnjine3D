#include "Player.h"
#include "MatrixMath.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

void Player::Initialize(Model* playerModel, Model* playerBulletModel, Camera* camera, const Vector3& position) {
	assert(playerModel);
	assert(playerBulletModel);
	model_ = playerModel;
	player_bullet_model_ = playerBulletModel;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	input_ = Input::GetInstance();
	hitPoint_ = 3;
	invincibleTimer_ = 0;
}

void Player::Update() {
	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	Vector3 move = {0.0f, 0.0f, 0.0f};
	const float kCharacterSpeed = 0.01f;
	const float kFriction = 0.9f;
	const float kMaxSpeed = 3.0f;

	if (invincibleTimer_ > 0) {
		--invincibleTimer_;
	}

	if (input_->PushKey(DIK_W)) {
		move.y += kCharacterSpeed;
	}
	if (input_->PushKey(DIK_S)) {
		move.y -= kCharacterSpeed;
	}
	if (input_->PushKey(DIK_D)) {
		move.x += kCharacterSpeed;
	}
	if (input_->PushKey(DIK_A)) {
		move.x -= kCharacterSpeed;

	}

	velocity_ += move;
	if (Length(velocity_) > kMaxSpeed) {
		velocity_ = Normalized(velocity_) * kMaxSpeed;
	}

	velocity_ *= kFriction;
	worldTransform_.translation_ += velocity_;

	const float kMoveLimitX = 6.0f;
	const float kMoveLimitY = 3.0f;

	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -kMoveLimitX, +kMoveLimitX);
	worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, -kMoveLimitY, +kMoveLimitY);

	Attack();

	 // ✅ ←★ ここに追加！弾の段階的発射
	if (isFiringFanMissiles_) {
		fireTimer_++;

		if (fireTimer_ >= fireInterval_) {
			fireTimer_ = 0;

			if (lockedEnemy_ && fireCount_ < 6) {
				Vector3 enemyPos = lockedEnemy_->GetWorldPosition();

				// ランダム制御点生成
				Vector3 offset = {((rand() % 200) - 100) / 10.0f, ((rand() % 200)) / 10.0f + 5.0f, ((rand() % 200) - 100) / 10.0f};

				HomingArcBullet* arcBullet = new HomingArcBullet();
				arcBullet->Initialize(player_bullet_model_, worldTransform_.translation_, enemyPos, offset);
				arcBullets_.push_back(arcBullet);

				fireCount_++;
			}

			if (fireCount_ >= 6) {
				isFiringFanMissiles_ = false;
			}
		}
	}

	//--------ビーム---------------
	for (auto* beam : beams_) {
		beam->Update();
	}
	beams_.remove_if([](Beam* b) {
		if (b->IsDead()) {
			delete b;
			return true;
		}
		return false;
	});




	for (PlayerBullet* bullet : bullets_) {
		bullet->Update();
	}

	for (HomingArcBullet* arcBullet : arcBullets_) {
		arcBullet->Update();
	}


	missileParticles_.remove_if([](MissilePartocle* p) {
		if (p->IsDead()) {
			delete p;
			return true;
		}
		return false;
	});

	arcBullets_.remove_if([](HomingArcBullet* b) {
		if (b->IsDead()) {
			delete b;
			return true;
		}
		return false;
	});

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void Player::Draw() {

	 // ★ 無敵中は点滅させる
	if (invincibleTimer_ > 0) {
		// 5フレームごとに ON/OFF 切り替え（点滅周期）
		if (((invincibleTimer_ / 5) % 2) == 0) {
			// このフレームは描画しない → 透明になる
			return;
		}
	}

	model_->Draw(worldTransform_, *camera_);

	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw(*camera_);
	}
	for (HomingArcBullet* bullet : arcBullets_) {
		bullet->Draw(*camera_);
	}
	for (auto* beam : beams_) {
		beam->Draw(*camera_);
	}
}

void Player::Attack() {
	// スペースキー or 左クリックで通常ショット
	if (input_->TriggerKey(DIK_SPACE) || input_->IsTriggerMouse(0)) {
		const float kBulletSpeed = 1.0f;

		// ローカルZ+方向（前）に飛ぶ速度ベクトル
		Vector3 velocity(0.0f, 0.0f, kBulletSpeed);
		velocity = TransformNormal(velocity, worldTransform_.matWorld_);

		// 弾の生成
		PlayerBullet* newBullet = new PlayerBullet();

		newBullet->Initialize(player_bullet_model_, worldTransform_.translation_, velocity);

		// 弾リストに登録
		bullets_.push_back(newBullet);
	}
	//=================================


	//================ロックオン・ミサイル発射処理================
	// ロックオン処理（右クリックなど）
	if (input_->IsTriggerMouse(1)) { // 右クリックでロックオン
		lockedEnemy_ = enemy_;
	}

	// 発射（左クリック）
	if (input_->IsTriggerMouse(0)) {
		if (!isFiringFanMissiles_ && lockedEnemy_) {
			isFiringFanMissiles_ = true;
			fireTimer_ = 0;
			fireCount_ = 0;
		}
	}
	// キーでロック解除（例：Rキー）
	if (input_->TriggerKey(DIK_R)) {
		lockedEnemy_ = nullptr;
	}
	//========================================================

	//==========ビーム===========
	if (input_->TriggerKey(DIK_E)) {
		Beam* beam = new Beam();
		Vector3 front = TransformNormal({0, 0, 1}, worldTransform_.matWorld_); // Z+方向 = 正面
		Vector3 target = worldTransform_.translation_ + front * 100.0f;        // 遠くへまっすぐ
		beam->Initialize(worldTransform_.translation_, target);
		beams_.push_back(beam);
	}
	//===========================

	// ★ それ以外のロックオン / ホーミング処理は一旦すべて削除
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

void Player::OnHitByBeam() {
	//既に無敵中ならスルー
	if (invincibleTimer_ > 0) {
		return;
	}

	//HP減少
	--hitPoint_;
	if (hitPoint_ < 0) {
		hitPoint_ = 0;
	}

	// 無敵時間セット（60フレーム＝1秒間）
	invincibleTimer_ = 60; // 1秒間

	// 余裕があれば演出を追加
	//worldTransform_.translation_.z -= 0.5f; // 少し後退


}


Player::~Player() {
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	for (HomingArcBullet* bullet : arcBullets_) {
		delete bullet;
	}
}



void Player::SetEnemy(Enemy* enemy) { enemy_ = enemy; }



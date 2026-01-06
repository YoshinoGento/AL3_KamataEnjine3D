#include "Player.h"
#include "MatrixMath.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

void Player::Initialize(Model* model, Camera* camera, const Vector3& position, uint32_t lockonTexture) {
	assert(model);
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	input_ = Input::GetInstance();
	lolckOn_.Initialize(lockonTexture);
}

void Player::Update() {

	 // ★無敵タイマーを減らす（これが無いと永久無敵 or 判定がおかしくなる）
	if (invincibleTimer_ > 0) {
		--invincibleTimer_;
	}

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

	for (PlayerBullet* bullet : bullets_) {
		bullet->Update();
	}

	for (HomingArcBullet* arcBullet : arcBullets_) {
		arcBullet->Update();
	}

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

void Player::Draw3D() {
	model_->Draw(worldTransform_, *camera_);
	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw(*camera_);
	}
	for (HomingArcBullet* bullet : arcBullets_) {
		bullet->Draw(*camera_);
	}
}

void Player::Draw2D() { lolckOn_.DrawMarkers(*camera_); }
void Player::Attack() {

	// ロックオンしたポインタが enemies_ に残ってるか判定（ポインタ比較だけなので安全）
	auto IsAlivePointerInEnemies = [&](Enemy* enemy_) -> bool {
		if (!enemies_ || !enemy_)
			return false;
		return std::find(enemies_->begin(), enemies_->end(), enemy_) != enemies_->end();
	};

	// ---------- 右クリック：ロックオン ----------
	if (input_->IsTriggerMouse(1)) {
		if (enemies_) {
			lolckOn_.TryLockOn(input_->GetMousePosition(), *enemies_, *camera_);
		}
	}

	// ---------- 左クリック：ロックオンがあればミサイル / なければ直進弾 ----------
	if (input_->IsTriggerMouse(0)) {

		std::vector<Enemy*> locked = lolckOn_.GetLockedEnemies();

		if (!locked.empty()) {

			for (Enemy* enemy : locked) {

				// ★まず「今の enemies_ に存在するか」だけを見る（安全）
				if (!IsAlivePointerInEnemies(enemy)) {
					continue;
				}

				// ★ここから先は敵が存在すると保証できるので触ってOK
				if (enemy->IsDead()) {
					continue;
				}

				HomingArcBullet* arc = new HomingArcBullet();

				Vector3 offset = {(rand() % 200 - 100) / 10.0f, (rand() % 100) / 10.0f + 5.0f, (rand() % 200 - 100) / 10.0f};

				Model* missileModel = playerMissileModel_ ? playerMissileModel_ : model_;
				arc->Initialize(missileModel, worldTransform_.translation_, enemy->GetWorldPosition(), offset);
				arcBullets_.push_back(arc);

				if (seMissile_ != 0) {
					Audio::GetInstance()->PlayWave(seMissile_, false, 0.1f);
				}

			}

			lolckOn_.Clear(); // 撃ったら解除
		}
		// B) ロックオンなし → クリック地点（XY） + 敵スポーンZ に向けて直進弾
		else {
			const float kBulletSpeed = 1.0f;

			// ★敵リスポーンのZ
			const float enemySpawnZ = 10.0f;

			Vector2 mouse = input_->GetMousePosition();

			// ★Z = enemySpawnZ の平面に当てる
			Vector3 target = CalcMouseHitOnZPlane(mouse, enemySpawnZ);

			// ★プレイヤーからその点へ撃つ
			Vector3 dir = Normalized(target - worldTransform_.translation_);
			Vector3 vel = dir * kBulletSpeed;

			PlayerBullet* bullet = new PlayerBullet();
			Model* bulletModel = playerBulletModel_ ? playerBulletModel_ : model_;
			bullet->Initialize(bulletModel, worldTransform_.translation_, vel);
			bullets_.push_back(bullet);

			if (seShot_ != 0) {
				Audio::GetInstance()->PlayWave(seShot_, false, 0.1f);
			}
		}
	}

	// ---------- R：解除 ----------
	if (input_->TriggerKey(DIK_R)) {
		lolckOn_.Clear();
	}
}

Player::~Player() {
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	for (HomingArcBullet* bullet : arcBullets_) {
		delete bullet;
	}
}

void Player::SetEnemies(const std::vector<Enemy*>* enemies) { enemies_ = enemies; }

Vector3 Player::CalcMouseHitOnZPlane(const Vector2& mouse, float planeZ) const {

	Vector3 nearW = UnProjectToWorldSpace(mouse, 0.0f, camera_->matView, camera_->matProjection, WinApp::kWindowWidth, WinApp::kWindowHeight);

	Vector3 farW = UnProjectToWorldSpace(mouse, 1.0f, camera_->matView, camera_->matProjection, WinApp::kWindowWidth, WinApp::kWindowHeight);

	Vector3 rayDir = Normalized(farW - nearW);

	// レイが平面と平行なら、とりあえず「プレイヤーの正面のそのZ」を返す
	if (std::fabs(rayDir.z) < 1e-6f) {
		Vector3 p = worldTransform_.translation_;
		p.z = planeZ;
		return p;
	}

	float t = (planeZ - nearW.z) / rayDir.z;
	return nearW + rayDir * t;
}

void Player::OnHit() {
	if (invincibleTimer_ > 0)
		return;

	--hitPoint_;
	if (hitPoint_ < 0)
		hitPoint_ = 0;

	invincibleTimer_ = 60; // 無敵時間
}

void Player::OnHitByBeam() {

	// 無敵時間中なら何もしない
	if (invincibleTimer_ > 0) {
		return;
	}

	// HP 減らす
	hitPoint_--;

	// 無敵時間を付与（1秒）
	invincibleTimer_ = 60;

	if (hitPoint_ <= 0) {
		hitPoint_ = 0;
		// TODO: 死亡処理（今は空でOK）
	}
}

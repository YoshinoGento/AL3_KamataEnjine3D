#include "ShooterEnemy.h"
#include <cstdlib>

// 乱数用ヘルパー関数
static float RandomRange(float a, float b) {
	float t = float(rand()) / float(RAND_MAX);
	return a + (b - a) * t;
}

void ShooterEnemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	Enemy::Initialize(model, camera, position);

	hp_ = 3;
	radius_ = 1.2f;

	fixedZ_ = position.z; // Z座標を記憶

	state_ = State::Shoot;
	shotsLeft_ = 6;
	shotTimer_ = 0;

	// ★Setされてなければ従来通り自分のモデルを使う
	if (!bulletModel_) {
		bulletModel_ = model_;
	}

	// ★修正：移動範囲をかなり狭めました（±15 → ±9）
	// これで画面端で見切れるのを防ぎます
	minX_ = -9.0f;
	maxX_ = 9.0f;
	minY_ = 1.0f;
	maxY_ = 7.0f;
}

void ShooterEnemy::Update(const Vector3& playerPos) {

	// ---- 敵弾の更新 ----
	for (auto it = bullets_.begin(); it != bullets_.end();) {
		EnemyBullet* b = *it;
		b->Update();
		if (b->IsDead()) {
			delete b;
			it = bullets_.erase(it);
		} else {
			++it;
		}
	}

	// ---- 状態ごとの行動 ----
	switch (state_) {
	case State::Shoot:
		UpdateShoot(playerPos);
		break;

	case State::Move:
		UpdateMove();
		break;
	}

	// ==================================================
	// 1. 移動ロジック（basePos_）が画面外に出ないように制限
	// ==================================================
	if (basePos_.x < minX_)
		basePos_.x = minX_;
	if (basePos_.x > maxX_)
		basePos_.x = maxX_;
	if (basePos_.y < minY_)
		basePos_.y = minY_;
	if (basePos_.y > maxY_)
		basePos_.y = maxY_;

	// ==================================================
	// 2. 親クラス更新（ここで basePos_ + 演出 = worldTransform_ が計算される）
	// ==================================================
	Enemy::Update(playerPos);

	// ==================================================
	// ★追加修正：ノックバック演出を含めた「最終的な位置」も画面内に収める
	// 演出で吹っ飛んでも、この壁でピタッと止まります
	// ==================================================
	if (worldTransform_.translation_.x < minX_)
		worldTransform_.translation_.x = minX_;
	if (worldTransform_.translation_.x > maxX_)
		worldTransform_.translation_.x = maxX_;
	if (worldTransform_.translation_.y < minY_)
		worldTransform_.translation_.y = minY_;
	if (worldTransform_.translation_.y > maxY_)
		worldTransform_.translation_.y = maxY_;

	// 見た目：プレイヤーの方向を向く
	FaceTo(playerPos);

	// ★向き修正：背中を向けてしまうので、180度回転させて補正
	worldTransform_.rotation_.y += 3.14159265f;

	// 最終的な行列更新（クランプや回転をしたので更新が必要）
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void ShooterEnemy::UpdateShoot(const Vector3& playerPos) {
	if (shotTimer_ > 0) {
		--shotTimer_;
		return;
	}

	Shoot(playerPos);
	--shotsLeft_;
	shotTimer_ = shotInterval_;

	if (shotsLeft_ <= 0) {
		BeginMove();
		state_ = State::Move;
	}
}

void ShooterEnemy::Shoot(const Vector3& playerPos) {
	if (!bulletModel_) {
		return;
	} // ★保険

	Vector3 dir = Normalized(playerPos - worldTransform_.translation_); // ここは見た目の位置から発射でOK
	Vector3 vel = dir * bulletSpeed_;

	EnemyBullet* b = new EnemyBullet();
	b->Initialize(bulletModel_, worldTransform_.translation_, vel);
	bullets_.push_back(b);
}

void ShooterEnemy::BeginMove() {
	// スタート地点は「本来の座標」から
	moveStart_ = basePos_;

	// 移動範囲の中からランダムに次の場所を決める
	moveTarget_ = {RandomRange(minX_, maxX_), RandomRange(minY_, maxY_), fixedZ_};
	moveTimer_ = 0.0f;
}

void ShooterEnemy::UpdateMove() {
	moveTimer_ += 1.0f / 60.0f;

	// イージング（滑らかな移動）
	float t = moveTimer_ / moveDuration_;
	if (t > 1.0f) {
		t = 1.0f;
	}

	// EaseInOut
	float t2 = t * t * (3.0f - 2.0f * t);

	// 線形補間
	// basePos_ を更新する（演出オフセットは親クラスが後で足してくれる）
	basePos_ = Lerp(moveStart_, moveTarget_, t2);

	if (t >= 1.0f) {
		state_ = State::Shoot;
		shotsLeft_ = 6;
	}
}

void ShooterEnemy::Draw3D() {
	Enemy::Draw3D();
	for (EnemyBullet* b : bullets_) {
		b->Draw(*camera_);
	}
}
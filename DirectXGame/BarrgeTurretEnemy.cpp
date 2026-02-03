#include "BarrgeTurretEnemy.h"
#include "MatrixMath.h"
#include <cmath>

static const float kPi = 3.1415926535f;

void BarrageTurretEnemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	Enemy::Initialize(model, camera, position);

	hp_ = 10; // 少し硬くする
	radius_ = 2.0f;

	pattern_ = Pattern::Wait;
	timer_ = 60; // 最初は60フレーム待機

	// ★修正：Z軸（奥行き）を固定値にする
	// 25.0f くらいが丁度いい距離感
	fixedZ_ = 15.0f;

	// 内部座標と見た目の座標も、その固定Z値に強制的に合わせる
	basePos_.z = fixedZ_;
	worldTransform_.translation_.z = fixedZ_;

	// ★追加修正：最初から手前（プレイヤー側＝180度）を向くようにする
	// これをしないと、スポーン時に奥（0度）を向いてしまい、背中が見えてしまいます
	worldTransform_.rotation_.y = kPi;

	// ノックバック変数の初期化（念のため）
	damageOffset_ = {0, 0, 0};
	knockbackVelocity_ = {0, 0, 0};

	// 行列を更新して、Z位置の変更を反映させる
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	if (!bulletModel_) {
		bulletModel_ = model_;
	}
}

void BarrageTurretEnemy::Update(const Vector3& playerPos) {
	// 親クラスの更新（もしあれば）
	Enemy::Update(playerPos);

	// ★追加修正：ノックバック処理をここで明示的に行う
	// 親クラスのUpdateでうまくいっていない可能性があるので、ここでも処理します。
	if (Length(knockbackVelocity_) > 0.001f) {
		// 速度をオフセットに加算（吹っ飛ぶ）
		damageOffset_ += knockbackVelocity_;
		// 速度の減衰（摩擦）
		knockbackVelocity_ *= 0.8f;

		if (Length(knockbackVelocity_) < 0.01f) {
			knockbackVelocity_ = {0, 0, 0};
		}
	}
	// ★重要：ズレた位置を徐々に元の位置に戻す（復元力）
	// これがないとズレっぱなしになるか、動いていないように見えます
	damageOffset_ *= 0.9f;
	if (Length(damageOffset_) < 0.01f) {
		damageOffset_ = {0, 0, 0};
	}

	UpdateBullets();

	// ----------------------------------------------------
	// 1. 移動ロジック
	// ----------------------------------------------------
	if (enableMove_) {
		basePos_ += velocity_;

		// ここでもZ座標を固定値に保つ
		basePos_.z = fixedZ_;

		// 壁反射
		if (basePos_.x < minX_) {
			basePos_.x = minX_;
			velocity_.x *= -1.0f;
		}
		if (basePos_.x > maxX_) {
			basePos_.x = maxX_;
			velocity_.x *= -1.0f;
		}
		if (basePos_.y < minY_) {
			basePos_.y = minY_;
			velocity_.y *= -1.0f;
		}
		if (basePos_.y > maxY_) {
			basePos_.y = maxY_;
			velocity_.y *= -1.0f;
		}
	} else {
		// 移動しない場合もZ位置を維持
		basePos_.z = fixedZ_;
	}

	// ----------------------------------------------------
	// 2. 演出（振動など）を加えた最終座標
	// ----------------------------------------------------
	// basePos_（本来のいるべき場所）に damageOffset_（衝撃のズレ）を足す
	Vector3 finalPos = basePos_ + damageOffset_;
	worldTransform_.translation_ = finalPos;

	// ----------------------------------------------------
	// 3. プレイヤーの方を向く（Y軸回転のみ）
	// ----------------------------------------------------
	Vector3 toPlayer = playerPos - finalPos;
	if (Length(toPlayer) > 0.1f) {
		Vector3 dir = Normalized(toPlayer);
		float targetAngle = std::atan2(dir.x, dir.z);
		// 補間してスムーズに向かせる
		float currentAngle = worldTransform_.rotation_.y;
		// 角度差を計算して少しずつ回す（簡易LERP）
		float diff = targetAngle - currentAngle;
		// -PI ~ PI に正規化が必要だが、ここでは簡易的に
		while (diff > kPi)
			diff -= 2 * kPi;
		while (diff < -kPi)
			diff += 2 * kPi;

		// ★少し回転速度を上げました（0.1 -> 0.2）
		worldTransform_.rotation_.y += diff * 0.2f;
	}

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	// ----------------------------------------------------
	// 4. 攻撃パターン制御
	// ----------------------------------------------------
	switch (pattern_) {
	case Pattern::Wait:
		timer_--;
		if (timer_ <= 0) {
			// ランダムに攻撃へ移行
			if (rand() % 2 == 0) {
				pattern_ = Pattern::Spiral;
				timer_ = 120; // 2秒間撃つ
			} else {
				pattern_ = Pattern::Aimed;
				timer_ = 60; // 5WAYを数回撃つための猶予
			}
		}
		break;

	case Pattern::Spiral:
		// 4フレームに1回発射
		if (timer_ % 4 == 0) {
			ShootSpiral(playerPos);
		}
		timer_--;
		if (timer_ <= 0) {
			pattern_ = Pattern::Wait;
			timer_ = 60;
		}
		break;

	case Pattern::Aimed:
		// 20フレームに1回、5WAY発射
		if (timer_ % 20 == 0) {
			ShootAimed(playerPos);
		}
		timer_--;
		if (timer_ <= 0) {
			pattern_ = Pattern::Wait;
			timer_ = 60;
		}
		break;
	}
}

void BarrageTurretEnemy::UpdateBullets() {
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
}

// ★回転弾幕（スパイラル）
void BarrageTurretEnemy::ShootSpiral(const Vector3& playerPos) {
	// 1. プレイヤーへの方向ベクトルを軸にする
	Vector3 toPlayer = playerPos - worldTransform_.translation_;
	if (Length(toPlayer) < 0.1f)
		toPlayer = {0, 0, 1}; // 近すぎる場合の保険

	Vector3 axisZ = Normalized(toPlayer);

	// 2. 軸に垂直なベクトルを作る（axisX, axisY）
	Vector3 up = {0.0f, 1.0f, 0.0f};
	if (std::abs(axisZ.y) > 0.95f)
		up = {1.0f, 0.0f, 0.0f}; // ジンバルロック対策

	Vector3 axisX = Normalized(Cross(axisZ, up));
	Vector3 axisY = Normalized(Cross(axisX, axisZ));

	// 角度を少しずつずらす
	spiralAngle_ += 15.0f * (kPi / 180.0f); // 15度ずつ回転

	// 広がり具合（0.0なら一直線、1.0なら45度くらい広がる）
	float spread = 0.15f;

	// 十字（4方向）にばらまく
	for (int i = 0; i < 4; ++i) {
		float angle = spiralAngle_ + (i * kPi / 2.0f); // 90度ずつずらす

		// 円周上のオフセットを計算 (axisX と axisY を合成)
		Vector3 offset = axisX * std::cos(angle) + axisY * std::sin(angle);

		// プレイヤー方向(axisZ) に 広がり(offset) を足す
		Vector3 dir = Normalized(axisZ + offset * spread);

		ShootOnce(dir);
	}
}

// ★自機狙い5WAY
void BarrageTurretEnemy::ShootAimed(const Vector3& playerPos) {
	// 1. プレイヤーへの方向ベクトルを計算
	Vector3 toPlayer = playerPos - worldTransform_.translation_;
	if (Length(toPlayer) < 0.1f)
		toPlayer = {0, -1, 0}; // 真下にいるときなどの保険
	Vector3 centerDir = Normalized(toPlayer);

	// 2. 右方向ベクトルを算出（拡散させるため）
	Vector3 up = {0.0f, 1.0f, 0.0f};
	if (std::abs(centerDir.y) > 0.95f)
		up = {1.0f, 0.0f, 0.0f};

	Vector3 right = Normalized(Cross(centerDir, up));

	// 5方向発射
	int way = 5;

	// ★修正：角度を狭くする（10度 -> 4度）
	// これにより弾の間を抜けにくくする
	float angleStep = 4.0f * (kPi / 180.0f);

	float startAngle = -((way - 1) * angleStep) / 2.0f; // -8度から開始

	for (int i = 0; i < way; ++i) {
		float angle = startAngle + (i * angleStep);

		// centerDir を軸に、right方向へ少し振る簡易計算
		// （本来は回転行列を使うほうが厳密だが、小角度ならこれで十分近似できる）
		Vector3 dir = centerDir + (right * std::sin(angle));
		dir = Normalized(dir);

		ShootOnce(dir);
	}
}

void BarrageTurretEnemy::ShootOnce(const Vector3& dir) {
	if (!bulletModel_)
		return;

	// 弾の生成
	EnemyBullet* b = new EnemyBullet();
	// 速度ベクトル
	Vector3 vel = dir * bulletSpeed_;

	b->Initialize(bulletModel_, worldTransform_.translation_, vel);
	bullets_.push_back(b);
}

void BarrageTurretEnemy::Draw3D() {
	Enemy::Draw3D(); // 本体描画

	for (EnemyBullet* b : bullets_) {
		b->Draw(*camera_);
	}
}

BarrageTurretEnemy::~BarrageTurretEnemy() {
	for (EnemyBullet* b : bullets_) {
		delete b;
	}
	bullets_.clear();
}
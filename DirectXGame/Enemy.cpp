#include "Enemy.h"
#include "GameScene.h"
#include "MatrixMath.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {

	assert(model);
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();

	// 初期位置
	worldTransform_.translation_ = position;

	// サイズ調整
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
}

void Enemy::Update(const Vector3 playerPos) {

	// プレイヤーへの方向ベクトル
	Vector3 toPlayer = playerPos - worldTransform_.translation_;
	toPlayer = Normalized(toPlayer);

	// Y軸回転を求める
	worldTransform_.rotation_.y = std::atan2(toPlayer.x, toPlayer.z);

	// --- ② 一定間隔で弾を発射 ---
	attackTimer_++;
	if (attackTimer_ >= 60) { // 約1秒ごとに攻撃（60fps前提）
		Attack(playerPos);
		attackTimer_ = 0;
	}

	// --- ③ 弾の更新 ---
	for (EnemyBullet* bullet : bullets_) {
		bullet->Update();
	}

	// --- ④ 死亡した弾の削除 ---
	// 弾リストから「死んだ弾」を削除する処理
	bullets_.erase(
	    // remove_ifは「削除したい要素を後ろに寄せて」、削除すべき範囲の先頭イテレータを返す
	    std::remove_if(
	        bullets_.begin(),           // 検索の開始位置
	        bullets_.end(),             // 検索の終了位置
	        [](EnemyBullet* bullet) {   // 各要素に対してチェックするラムダ式
		        if (bullet->IsDead()) { // 弾が死んでいたら
			        delete bullet;      // メモリを解放
			        return true;        // 「削除対象」としてマーク
		        }
		        return false; // 生きてる弾は残す
	        }),
	    bullets_.end() // remove_ifで後ろに寄せられた削除対象を実際に消す
	);

	// 行列を更新
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void Enemy::Attack(const Vector3& playerPos) {
	// 弾速
	const float kBulletSpeed = 0.3f;

	// プレイヤーへの方向を求める
	Vector3 toPlayer = playerPos - worldTransform_.translation_;
	toPlayer = Normalized(toPlayer);

	Vector3 velocity = toPlayer * kBulletSpeed;

	// 弾生成
	EnemyBullet* newBullet = new EnemyBullet();
	newBullet->Initialize(model_, worldTransform_.translation_, velocity);

	bullets_.push_back(newBullet);
}

void Enemy::Draw() {
	// プレイヤーモデル描画
	model_->Draw(worldTransform_, *camera_);

	// 弾の描画
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw(*camera_);
	}
}

Enemy::~Enemy() {
	for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}
}

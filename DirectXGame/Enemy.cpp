#include "Enemy.h"
#include "MatrixMath.h"
#include <cassert>
#include <cmath>

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	// ★本来の座標を初期化
	basePos_ = position;

	worldTransform_.Initialize();
	// 念のため見た目も合わせる
	worldTransform_.translation_ = basePos_;

	// ★追加：ここで最初の行列を確定＆GPUへ
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void Enemy::Update(const Vector3& playerPos) {
	// 基本は何もしない（派生で上書き）
	(void)playerPos;

	// ---------------------------------------------------------
	// 1. ノックバック（ダメージ演出）の計算
	// ---------------------------------------------------------
	if (Length(knockbackVelocity_) > 0.01f) {
		// 速度分をオフセットに加算
		damageOffset_ += knockbackVelocity_;

		// 減衰（摩擦）
		knockbackVelocity_ *= 0.85f;
		if (Length(knockbackVelocity_) < 0.05f) {
			knockbackVelocity_ = {0, 0, 0};
		}
	}

	// オフセット自体も徐々に0に戻ろうとする（バネのような復元力）
	damageOffset_ *= 0.90f;
	if (Length(damageOffset_) < 0.01f) {
		damageOffset_ = {0, 0, 0};
	}

	// ---------------------------------------------------------
	// 2. 最終的な座標を Vector3 で計算する
	// ---------------------------------------------------------

	// まず「本来の位置」＋「ノックバックのズレ」を基準にする
	Vector3 finalPos = basePos_ + damageOffset_;

	// ★ダメージ演出：ヒットシェイク（振動）
	// Vector3の変数に対して直接計算を行う
	if (hitShakeTimer_ > 0) {
		hitShakeTimer_--;

		// 乱数で少し位置をずらす（ガクガクさせる）
		float shakePower = 0.3f; // 揺れ幅
		float rx = (float(rand()) / RAND_MAX - 0.5f) * shakePower;
		float ry = (float(rand()) / RAND_MAX - 0.5f) * shakePower;
		float rz = (float(rand()) / RAND_MAX - 0.5f) * shakePower;

		// 基準位置に振動分を足し込む
		finalPos.x += rx;
		finalPos.y += ry;
		finalPos.z += rz;
	}

	// ---------------------------------------------------------
	// 3. 最後にトランスフォームへ反映
	// ---------------------------------------------------------
	worldTransform_.translation_ = finalPos;

	// 行列更新
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

bool Enemy::CheckCollision(const Vector3& playerPos, float playerRadius) {
	// 当たり判定は「見た目の位置」で行う（吹っ飛んでいる最中は当たり判定も移動する）
	float dist = Length(worldTransform_.translation_ - playerPos);

	// お互いの半径の合計より近ければ当たっている
	if (dist <= radius_ + playerRadius) {
		return true;
	}
	return false;
}

void Enemy::Draw3D() {
	if (model_) {
		model_->Draw(worldTransform_, *camera_);
	}
}

void Enemy::Damage(int amount) {
	hp_ -= amount;
	if (hp_ <= 0) {
		hp_ = 0;
		isDead_ = true;
	} else {
		// ★生きていればダメージ演出

		// 1. ノックバック：少し上に跳ねて、後ろ（ランダム気味）に下がる
		// 簡易的に「上方向」＋「ランダムな水平方向」に力を加える
		float kickBack = 0.5f;
		float jump = 0.3f;

		float rx = (float(rand()) / RAND_MAX - 0.5f);
		float rz = (float(rand()) / RAND_MAX - 0.5f);
		Vector3 dir = Normalized({rx, 0, rz}); // ランダムな方向

		// 自分の正面の逆（後ろ）に飛ばすのが理想だが、簡易的にランダム拡散させる
		knockbackVelocity_ = dir * kickBack;
		knockbackVelocity_.y = jump; // 少し浮く

		// 2. ヒットシェイク開始
		hitShakeTimer_ = 10; // 10フレーム（約0.16秒）振動
	}
}

void Enemy::FaceTo(const Vector3& targetPos) {
	// 向き計算は basePos_（本来の位置）を基準にすると安定する
	Vector3 dir = targetPos - basePos_;
	if (Length(dir) < 1e-6f) {
		dir = {0, 0, 1};
	}
	// Y軸成分を消して水平回転のみにする
	dir.y = 0;
	dir = Normalized(dir);

	// Y軸回転を計算 (atan2)
	float angleY = std::atan2(dir.x, dir.z);
	worldTransform_.rotation_.y = angleY;
}
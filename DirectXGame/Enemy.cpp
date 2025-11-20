#include "Enemy.h"
#include "KamataEngine.h"

using namespace KamataEngine;

void Enemy::Initialize(const Vector3& bossBasePosition) {

	// ★ 可視化用モデルをロード（暫定で "player" を使用）
	//   → 自前の "enemy" / "boss" モデルができたら名前を差し替えてOK
	model_ = Model::CreateFromOBJ("player");

	// 0: 中央のコア
	bodyParts_[0].centerPosition = bossBasePosition + Vector3{0.0f, 0.5f, 0.0f};
	bodyParts_[0].boxSize = Vector3{2.0f, 1.0f, 1.0f}; // X,Y,Z の長さ
	bodyParts_[0].hitPoint = 5;
	bodyParts_[0].isDestroyed = false;

	// 1: 左側
	bodyParts_[1].centerPosition = bossBasePosition + Vector3{-1.5f, 0.0f, 0.0f};
	bodyParts_[1].boxSize = Vector3{1.0f, 1.0f, 1.0f};
	bodyParts_[1].hitPoint = 3;
	bodyParts_[1].isDestroyed = false;

	// 2: 右側
	bodyParts_[2].centerPosition = bossBasePosition + Vector3{+1.5f, 0.0f, 0.0f};
	bodyParts_[2].boxSize = Vector3{1.0f, 1.0f, 1.0f};
	bodyParts_[2].hitPoint = 3;
	bodyParts_[2].isDestroyed = false;

	// ★ 各部位のワールドトランスフォーム初期化
	for (int i = 0; i < kBodyPartCount; ++i) {
		worldTransforms_[i].Initialize();
		worldTransforms_[i].translation_ = bodyParts_[i].centerPosition;

		// AABB のサイズをそのままスケールとして使う（見た目上は箱っぽい伸び方になる）
		// ※ここは好みに合わせて 0.5f 掛けるなどで調整してOK
		worldTransforms_[i].scale_ = bodyParts_[i].boxSize;
	}
}

void Enemy::Update() {
	// いまは特に動かさない。あとでボスの移動処理などを追加するとよい。
}

void Enemy::Draw(const Camera& camera) {

	if (!model_) {
		return;
	}

	// 壊れていない部位だけ描画
	for (int i = 0; i < kBodyPartCount; ++i) {

		const BodyPart& part = bodyParts_[i];
		if (part.isDestroyed) {
			continue;
		}

		// centerPosition / boxSize が将来動く可能性を見越して同期しておく
		worldTransforms_[i].translation_ = part.centerPosition;
		worldTransforms_[i].scale_ = part.boxSize;

		// 行列更新
		WorldTransformUpdate(worldTransforms_[i]);

		// モデル描画（テクスチャはなし）
		model_->Draw(worldTransforms_[i], camera);
	}
}

bool Enemy::CheckHit(const Vector3& bulletPosition) {

	for (int bodyPartIndex = 0; bodyPartIndex < kBodyPartCount; ++bodyPartIndex) {

		BodyPart& bodyPart = bodyParts_[bodyPartIndex];

		// すでに破壊済みなら無視
		if (bodyPart.isDestroyed) {
			continue;
		}

		// AABB の min / max を計算
		Vector3 halfBoxSize = bodyPart.boxSize * 0.5f;
		Vector3 minPosition = bodyPart.centerPosition - halfBoxSize;
		Vector3 maxPosition = bodyPart.centerPosition + halfBoxSize;

		// 点（弾）の位置が AABB 内にあるか判定
		bool isInside = (bulletPosition.x >= minPosition.x && bulletPosition.x <= maxPosition.x) && (bulletPosition.y >= minPosition.y && bulletPosition.y <= maxPosition.y) &&
		                (bulletPosition.z >= minPosition.z && bulletPosition.z <= maxPosition.z);

		if (isInside) {
			// ヒットしたのでダメージ
			bodyPart.hitPoint--;

			if (bodyPart.hitPoint <= 0) {
				bodyPart.isDestroyed = true;
			}

			// 1発で複数部位に当たらないように、ここで終了
			return true;
		}
	}

	return false;
}

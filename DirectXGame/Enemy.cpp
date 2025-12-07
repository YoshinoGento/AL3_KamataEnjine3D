#include "Enemy.h"
#include "KamataEngine.h"
#include <cmath> // atan2f, sqrtf

using namespace KamataEngine;

// フェーズごとのフレーム数（好みに合わせて調整）
namespace {
const int kMoveToPlaneFrames = 30;    // フェーズ1：ボス → プレイヤーZ平面
const int kMoveSideToEdgeFrames = 30; // フェーズ2：Zを合わせた位置 → 画面端
const int kChargingFrames = 120;       // チャージ時間
const int kFiringFrames = 120;        // フェーズ3：照射継続時間

const int kRightExtraChargeFrames = 30; // 右側ファンネルだけチャージ時間を少し長くする

// ビームの太さ（円柱の半径として扱う）
const float kBeamRadius = 0.3f;
} // namespace

void Enemy::Initialize(const Vector3& bossBasePosition) {

	// 可視化用モデルをロード（暫定で "player" を使用）
	model_ = Model::CreateFromOBJ("Boss");

	// ビーム用モデルをロード（円柱モデル）
	beamModel_ = Model::CreateFromOBJ("beam");

	// -----------------------------
	// ボス本体の部位データを設定
	// -----------------------------

	// 0: 中央のコア
	bodyParts_[0].centerPosition = bossBasePosition + Vector3{0.0f, 0.5f, 0.0f};
	bodyParts_[0].boxSize = Vector3{2.0f, 1.0f, 1.0f}; // X,Y,Z の長さ
	bodyParts_[0].hitPoint = 5;
	bodyParts_[0].isDestroyed = false;

	// 1: 左側パーツ
	bodyParts_[1].centerPosition = bossBasePosition + Vector3{-1.5f, 0.0f, 0.0f};
	bodyParts_[1].boxSize = Vector3{1.0f, 1.0f, 1.0f};
	bodyParts_[1].hitPoint = 3;
	bodyParts_[1].isDestroyed = false;

	// 2: 右側パーツ
	bodyParts_[2].centerPosition = bossBasePosition + Vector3{+1.5f, 0.0f, 0.0f};
	bodyParts_[2].boxSize = Vector3{1.0f, 1.0f, 1.0f};
	bodyParts_[2].hitPoint = 3;
	bodyParts_[2].isDestroyed = false;

	// 見た目用ワールドトランスフォーム初期化
	for (int i = 0; i < kBodyPartCount; ++i) {
		worldTransforms_[i].Initialize();
		worldTransforms_[i].translation_ = bodyParts_[i].centerPosition;
		worldTransforms_[i].scale_ = bodyParts_[i].boxSize;
	}

	// -----------------------------
	// ファンネル攻撃用初期化
	// -----------------------------
	for (int i = 0; i < kFunnelCount; ++i) {
		funnels_[i].wt.Initialize();
		funnels_[i].wt.scale_ = {0.5f, 0.5f, 0.5f}; // ファンネル本体の大きさ
		funnels_[i].state = Funnel::Inactive;
		funnels_[i].timer = 0;
	}

	// ビーム描画用 WorldTransform（全ビーム共通で使い回し）
	beamWorldTransform_.Initialize();
	// X/Y が半径、Z が長さ方向（後で length によって変える）
	beamWorldTransform_.scale_ = {kBeamRadius, kBeamRadius, 1.0f};

	// 最初の攻撃まで少し待つ
	funnelAttackCoolTimer_ = 180;
}

void Enemy::Update(const Vector3& playerPosition) {

	// ファンネル攻撃クールタイム処理
	if (funnelAttackCoolTimer_ > 0) {
		--funnelAttackCoolTimer_;
	}

	// クールタイム終了 ＆ 空きファンネルあり → 攻撃開始
	if (funnelAttackCoolTimer_ == 0) {
		StartFunnelAttack(playerPosition);
	}

	// ファンネルの状態更新（L字移動＋照射）
	UpdateFunnels(playerPosition);
}

void Enemy::Draw(const Camera& camera) {

	if (!model_) {
		return;
	}

	// 壊れていない本体部位だけ描画
	for (int i = 0; i < kBodyPartCount; ++i) {

		const BodyPart& part = bodyParts_[i];
		if (part.isDestroyed) {
			continue;
		}

		worldTransforms_[i].translation_ = part.centerPosition;
		worldTransforms_[i].scale_ = part.boxSize;

		WorldTransformUpdate(worldTransforms_[i]);
		model_->Draw(worldTransforms_[i], camera);
	}

	// ファンネル描画（本体＋ビーム）
	DrawFunnels(camera);
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


// ============================
// ファンネル攻撃：開始
// ============================
void Enemy::StartFunnelAttack(const Vector3& playerPosition) {

	// 左ファンネル（index 0）
	{
		Funnel& f = funnels_[0];
		if (f.state == Funnel::Inactive && !bodyParts_[1].isDestroyed) {

			f.state = Funnel::MoveToPlane;
			f.timer = kMoveToPlaneFrames;

			const BodyPart& base = bodyParts_[1];

			f.startPosition = base.centerPosition;
			f.wt.translation_ = f.startPosition;
			f.targetPlanePosition = {base.centerPosition.x, base.centerPosition.y, playerPosition.z};

			float edgeX = -7.5f;
			f.edgePosition = {edgeX, playerPosition.y, playerPosition.z};

			f.beamTarget = playerPosition;
			f.fromLeft = true;
		}
	}

	// 右ファンネル（index 1）
	{
		Funnel& f = funnels_[1];
		if (f.state == Funnel::Inactive && !bodyParts_[2].isDestroyed) {

			f.state = Funnel::MoveToPlane;
			f.timer = kMoveToPlaneFrames;

			const BodyPart& base = bodyParts_[2];

			f.startPosition = base.centerPosition;
			f.wt.translation_ = f.startPosition;
			f.targetPlanePosition = {base.centerPosition.x, base.centerPosition.y, playerPosition.z};

			float edgeX = +7.5f;
			f.edgePosition = {edgeX, playerPosition.y, playerPosition.z};

			f.beamTarget = playerPosition;
			f.fromLeft = false;
		}
	}

	// 次の攻撃までのクール
	funnelAttackCoolTimer_ = 240;
}


// ============================
// ファンネル攻撃：状態更新
// ============================
void Enemy::UpdateFunnels(const Vector3& playerPosition) {
	for (int i = 0; i < kFunnelCount; ++i) {

		Funnel& f = funnels_[i];

		// ★ 右ファンネル専用の追従処理（攻撃前のみ）
		if (i == 1 && (f.state == Funnel::MoveToPlane || f.state == Funnel::MoveSideToEdge || f.state == Funnel::Charging)) {
			// 右ファンネルは攻撃前はずっとプレイヤーを追う
			f.beamTarget = playerPosition;

			// Z合わせ
			f.targetPlanePosition.z = playerPosition.z;

			// 右側の端座標も追従
			float edgeX = +7.5f;
			f.edgePosition = {edgeX, playerPosition.y, playerPosition.z};

			// ファンネル本体を向ける
			Vector3 dir = f.beamTarget - f.wt.translation_;
			float len = Length(dir);
			if (len > 0.01f) {
				Vector3 nd = dir / len;
				float yaw = std::atan2f(nd.x, nd.z);
				float pitch = std::atan2f(-nd.y, sqrtf(nd.x * nd.x + nd.z * nd.z));
				f.wt.rotation_ = {pitch, yaw, 0.0f};
			}
		}

		// ===============================
		// ★ 通常フェーズ
		// ===============================
		switch (f.state) {
		case Funnel::Inactive:
			break;

		case Funnel::MoveToPlane:
			if (f.timer > 0) {
				float t = 1.0f - (float)f.timer / kMoveToPlaneFrames;
				f.wt.translation_ = Lerp(f.startPosition, f.targetPlanePosition, t);
				--f.timer;
			} else {
				f.state = Funnel::MoveSideToEdge;
				f.timer = kMoveSideToEdgeFrames;
			}
			break;

		case Funnel::MoveSideToEdge:
			if (f.timer > 0) {
				float t = 1.0f - (float)f.timer / kMoveSideToEdgeFrames;
				f.wt.translation_ = Lerp(f.targetPlanePosition, f.edgePosition, t);
				--f.timer;
			} else {
				f.state = Funnel::Charging;

				// ★ 左右でチャージ時間変える
				if (i == 0) {
					// 左：普通のチャージ
					f.timer = kChargingFrames;
				} else {
					// 右：遅れて撃つ
					f.timer = kChargingFrames + kRightExtraChargeFrames;
				}
			}
			break;

		case Funnel::Charging:
			if (f.timer > 0) {
				--f.timer;
			} else {
				// ★ この行に絶対到達すること！
				f.state = Funnel::Firing;
				f.timer = kFiringFrames;
			}
			break;

		case Funnel::Firing:
			if (f.timer > 0) {
				--f.timer;
			} else {
				f.state = Funnel::Inactive;
			}
			break;
		}
	}
}


// ============================
// ファンネル描画（本体＋円柱ビーム）
// ============================
void Enemy::DrawFunnels(const Camera& camera) {

	if (!model_) {
		return;
	}

	for (int i = 0; i < kFunnelCount; ++i) {
		Funnel& f = funnels_[i];
		if (f.state == Funnel::Inactive) {
			continue;
		}

		// ---------------------
		// ファンネル本体の描画
		// ---------------------
		WorldTransformUpdate(f.wt);
		model_->Draw(f.wt, camera);

		// ---------------------
		// 照射ビーム本体（円柱）の描画
		// ---------------------
		if (f.state == Funnel::Firing) {

			// ビームの始点（ファンネル位置）と終点（ターゲット位置）
			Vector3 beamStart = f.wt.translation_;
			Vector3 beamEnd = f.beamTarget;

			// 方向ベクトルと長さ
			Vector3 dir = beamEnd - beamStart;
			float length = Length(dir);
			if (length < 0.001f) {
				continue;
			}

			// 方向の正規化
			Vector3 nd = Normalized(dir);

			// ★ ファンネル位置から少しだけ前方にずらして発射
			const float kStartOffset = 2.0f; // 好みで調整
			beamStart += nd * kStartOffset;

			// ビームの中心位置（中点）
			Vector3 midPos = (beamStart + beamEnd) * 0.5f;
			midPos.y -= 1.2f; // 少し下にずらす（見た目調整）
			

			// 円柱モデルが「ローカルZ軸方向」に伸びている前提で、
			// Z軸→nd へ回すためのヨー・ピッチを求める（簡易版）
			float yaw = std::atan2f(nd.x, nd.z);                                     // Y軸回転
			float pitch = std::atan2f(-nd.y, std::sqrtf(nd.x * nd.x + nd.z * nd.z)); // X軸回転

			beamWorldTransform_.translation_ = midPos;
			beamWorldTransform_.rotation_ = {pitch, yaw, 0.0f};

			// X/Y は半径、Z は長さ方向にスケール（モデル原点から±Zなので 0.5 を掛ける）
			beamWorldTransform_.scale_ = {kBeamRadius, kBeamRadius, length * 0.5f};

			WorldTransformUpdate(beamWorldTransform_);

			// ビーム専用モデルがあればそれを使う
			if (beamModel_) {
				beamModel_->Draw(beamWorldTransform_, camera);
			} else {
				model_->Draw(beamWorldTransform_, camera); // フォールバック
			}
		}
	}
}

// ============================
// ビーム当たり判定
// ============================
bool Enemy::IsPlayerHitByFunnelBeam(const Vector3& playerPosition, float playerRadius) {

	// プレイヤー球＋ビーム円柱の「合成半径」の2乗
	float combinedRadius = playerRadius + kBeamRadius;
	float combinedRadiusSq = combinedRadius * combinedRadius;

	// 全ファンネルについてチェック
	for (int i = 0; i < kFunnelCount; ++i) {

		const Funnel& f = funnels_[i];

		// 照射中のファンネルのみ当たり判定対象
		if (f.state != Funnel::Firing) {
			continue;
		}

		// ビームの始点（ファンネル位置）と終点（ターゲット位置）
		Vector3 beamStart = f.wt.translation_;
		Vector3 beamEnd = f.beamTarget;

		// 線分ベクトル
		Vector3 seg = beamEnd - beamStart;

		// 線分の長さ^2
		float segLenSq = seg.x * seg.x + seg.y * seg.y + seg.z * seg.z;
		if (segLenSq <= 0.0001f) {
			continue; // 長さゼロならスキップ
		}

		// 始点からプレイヤーへのベクトル
		Vector3 toPlayer = playerPosition - beamStart;

		// 線分上の最近接点を求めるための t（0〜1 にクランプ）
		float dot = toPlayer.x * seg.x + toPlayer.y * seg.y + toPlayer.z * seg.z;
		float t = dot / segLenSq;
		if (t < 0.0f) {
			t = 0.0f;
		}
		if (t > 1.0f) {
			t = 1.0f;
		}

		// 線分上の最近接点
		Vector3 closestPoint = beamStart + seg * t;

		// プレイヤー中心との距離^2
		Vector3 diff;
		diff.x = playerPosition.x - closestPoint.x;
		diff.y = playerPosition.y - closestPoint.y;
		diff.z = playerPosition.z - closestPoint.z;

		float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		// 合成半径以内ならビーム被弾
		if (distSq <= combinedRadiusSq) {
			return true;
		}
	}

	return false;
}

Vector3 Enemy::GetWorldPosition() const { return worldTransforms_->translation_; }

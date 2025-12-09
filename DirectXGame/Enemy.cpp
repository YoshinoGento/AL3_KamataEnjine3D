#include "Enemy.h"
#include "KamataEngine.h"
#include "Player.h"
#include <cmath> // atan2f, sqrtf

using namespace KamataEngine;

// フェーズごとのフレーム数（好みに合わせて調整）
namespace {
const int kMoveToPlaneFrames = 30;    // フェーズ1：ボス → プレイヤーZ平面
const int kMoveSideToEdgeFrames = 30; // フェーズ2：Zを合わせた位置 → 画面端
const int kChargingFrames = 120;       // チャージ時間
const int kFiringFrames = 120;        // フェーズ3：照射継続時間

// ビームの太さ（円柱の半径として扱う）
const float kBeamRadius = 0.3f;
} // namespace

void Enemy::Initialize(const Vector3& bossBasePosition) {

	// 可視化用モデルをロード（暫定で "player" を使用）
	model_ = Model::CreateFromOBJ("player");

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
	bullets_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

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

	fireTimer--;
	// 指定時間に達した
	if (fireTimer <= 0) {
		// 弾を発射
		ShootMissile();
		// 発射タイマーを初期化
		fireTimer = kFireInterval;
	}

	// 弾更新
	for (EnemyBullet* bullet : bullets_) {
		bullet->Update();
	}
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

	// 空いているファンネルを1機だけ使う
	Funnel* freeFunnel = nullptr;
	for (int i = 0; i < kFunnelCount; ++i) {
		if (funnels_[i].state == Funnel::Inactive) {
			freeFunnel = &funnels_[i];
			break;
		}
	}
	if (!freeFunnel) {
		return; // 全部使用中
	}

	Funnel& f = *freeFunnel;

	// どの部位から飛ばすか（今回は index=1 をファンネルベースとする）
	const BodyPart& basePart = bodyParts_[1];

	// フェーズ1開始位置：ベース部位の位置
	f.startPosition = basePart.centerPosition;
	f.wt.translation_ = f.startPosition;

	// フェーズ1終了位置：プレイヤーと同じZで、X/Yはベースのまま（まずZだけ合わせる）
	f.targetPlanePosition = {basePart.centerPosition.x, basePart.centerPosition.y, playerPosition.z};

	// どちら側の端から撃つか（とりあえずプレイヤーの左側/右側で決定）
	f.fromLeft = (playerPosition.x >= 0.0f); // プレイヤーが右寄りなら左から撃つ 等

	// 画面端のX（Playerの移動制限と同じぐらい＋α）
	const float kVisibleEdgeX = 7.5f;
	float edgeX = f.fromLeft ? -kVisibleEdgeX : kVisibleEdgeX;

	// フェーズ2終了位置：画面端での照射位置（X:端 / Y,Z: プレイヤーと同一）
	f.edgePosition = {edgeX, playerPosition.y, playerPosition.z};

	// プレイヤーの現在位置をロックオンしておく（ビーム先端のターゲット）
	f.beamTarget = playerPosition;

	// フェーズ1：Z を合わせる移動時間
	f.timer = kMoveToPlaneFrames;
	f.state = Funnel::MoveToPlane;

	// 次の攻撃までのクールタイム再セット
	funnelAttackCoolTimer_ = 240; // 4秒ぐらい（好みで調整）
}

// ============================
// ファンネル攻撃：状態更新
// ============================
void Enemy::UpdateFunnels(const Vector3& /*playerPosition*/) {

	for (int i = 0; i < kFunnelCount; ++i) {

		Funnel& f = funnels_[i];

		switch (f.state) {
		case Funnel::Inactive:
			// 何もしない
			break;

		case Funnel::MoveToPlane: {
			// フェーズ1: startPosition → targetPlanePosition へ前進（Z を合わせる）
			if (f.timer > 0) {
				float t = 1.0f - static_cast<float>(f.timer) / static_cast<float>(kMoveToPlaneFrames); // 0→1
				f.wt.translation_ = Lerp(f.startPosition, f.targetPlanePosition, t);
				--f.timer;
			} else {
				// Z を合わせ終わったら、フェーズ2：横移動開始
				f.state = Funnel::MoveSideToEdge;
				f.timer = kMoveSideToEdgeFrames;
			}
			break;
		}

		case Funnel::MoveSideToEdge: {
			// フェーズ2: targetPlanePosition → edgePosition へ横移動
			if (f.timer > 0) {
				float t = 1.0f - static_cast<float>(f.timer) / static_cast<float>(kMoveSideToEdgeFrames); // 0→1
				f.wt.translation_ = Lerp(f.targetPlanePosition, f.edgePosition, t);
				--f.timer;
			} else {
				// 画面端に到達 → ★チャージフェーズへ
				f.wt.translation_ = f.edgePosition;
				f.state = Funnel::Charging; // ← ここを Firing ではなく Charging に
				f.timer = kChargingFrames;  // チャージに使う時間
			}
			break;
		}

		case Funnel::Charging: {
			// 画面端でビームを溜めている状態（ここではビームをまだ出さない）
			if (f.timer > 0) {
				--f.timer;
				// ここで「チャージ用の光」だけ描画する、なども後で足せる
			} else {
				// チャージ完了 → 照射フェーズ開始
				f.state = Funnel::Firing;
				f.timer = kFiringFrames;
			}
			break;
		}

		case Funnel::Firing: {
			// フェーズ3: 画面端から照射中
			if (f.timer > 0) {
				--f.timer;
				// ここで本当はビームのアニメーションやエフェクトを入れる
			} else {
				// 終了 → 待機へ戻る
				f.state = Funnel::Inactive;
			}
			break;
		}

		default:
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

	// ファンネル描画
	DrawFunnels(camera);
 }

	// 弾描画
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw(camera);
	}
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

void Enemy::ShootMissile() {
	// 弾の速さ
	const float kBulletSpeed = 2.0f;
	Vector3 velocity = {};

	// 敵のワールド座標を取得する
	Vector3 playerPos = player_->GetWorldPosition();
	const BodyPart& bodyPart = bodyParts_[0];
	// 敵キャラのワールド座標を取得する
	Vector3 enemyPos = bodyPart.centerPosition;
	// 敵キャラから自キャラへの差分ベクトルを求める
	Vector3 e2p = playerPos - enemyPos;
	// ベクトルの正規化
	Normalized(e2p);
	// ベクトルの長さを、速さに合わせる
	velocity = e2p * kBulletSpeed;

	// 弾を生成し、初期化
	EnemyBullet* newBullet = new EnemyBullet();
	newBullet->Initialize(model_, worldTransforms_[0].translation_, velocity);
	newBullet->SetPlayer(player_);

	// 弾を登録する
	bullets_.push_back(newBullet);
}

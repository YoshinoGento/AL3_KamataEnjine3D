#include "Enemy.h"
#include "KamataEngine.h"

// フェーズごとのフレーム数（好みに合わせて調整）
namespace {
const int kMoveToPlaneFrames = 30;    // フェーズ1：ボス → プレイヤーZ平面
const int kMoveSideToEdgeFrames = 30; // フェーズ2：Zを合わせた位置 → 画面端
const int kFiringFrames = 120;        // フェーズ3：照射継続時間
} // namespace

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

	// === ファンネル攻撃用初期化 ===
	for (int i = 0; i < kFunnelCount; ++i) {
		funnels_[i].wt.Initialize();
		funnels_[i].wt.scale_ = {0.5f, 0.5f, 0.5f}; // ファンネルの大きさ
		funnels_[i].state = Funnel::Inactive;
		funnels_[i].timer = 0;
	}

	// ビーム描画用 WorldTransform（全セグメントで使い回す）
	beamWorldTransform_.Initialize();
	beamWorldTransform_.scale_ = {0.2f, 0.2f, 0.2f};

	funnelAttackCoolTimer_ = 180; // 最初の攻撃まで少し待つ
}

void Enemy::Update(const Vector3& playerPosition) {

	// === ファンネル攻撃処理 ===

	// 攻撃クールタイマーを減らす
	if (funnelAttackCoolTimer_ > 0) {
		--funnelAttackCoolTimer_;
	}

	// クールタイム終了 ＆ 開いてるファンネルがあれば攻撃開始
	if (funnelAttackCoolTimer_ == 0) {
		StartFunnelAttack(playerPosition);
	}

	// ファンネルのステート更新
	UpdateFunnels(playerPosition);
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
	const float kEdgeX = 10.0f;
	float edgeX = f.fromLeft ? -kEdgeX : kEdgeX;

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
				// 画面端に到達 → 照射フェーズへ
				f.wt.translation_ = f.edgePosition;
				f.state = Funnel::Firing;
				f.timer = kFiringFrames; // 2秒ぐらい照射
			}
			break;
		}

		case Funnel::Firing: {
			// フェーズ3: 画面端から照射中
			if (f.timer > 0) {
				--f.timer;
				// ここで本当はビームのアニメーションや当たり判定を入れる
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

	// ファンネル描画
	DrawFunnels(camera);
}

void Enemy::DrawFunnels(const Camera& camera) {

	if (!model_) {
		return;
	}

	for (int i = 0; i < kFunnelCount; ++i) {
		Funnel& f = funnels_[i];
		if (f.state == Funnel::Inactive) {
			continue;
		}

		// =====================
		// ファンネル本体の描画
		// =====================
		WorldTransformUpdate(f.wt);
		model_->Draw(f.wt, camera);

		// =====================
		// 照射ビームの描画（見た目のみ）
		// =====================
		if (f.state == Funnel::Firing) {

			const int kBeamSegmentCount = 12;
			const float denom = 11.0f; // kBeamSegmentCount - 1

			const Vector3 beamStart = f.wt.translation_; // ファンネル位置
			const Vector3 beamEnd = f.beamTarget;        // ロックしたプレイヤー位置

			for (int seg = 0; seg < kBeamSegmentCount; ++seg) {

				// 0.0f ～ 1.0f の補間係数
				float t = static_cast<float>(seg) / denom;

				// 始点→終点の途中位置
				Vector3 segmentPos = Lerp(beamStart, beamEnd, t);

				// 共有の WorldTransform を使い回す
				beamWorldTransform_.translation_ = segmentPos;
				// 太さを変えたければここで scale_ を調整
				// beamWorldTransform_.scale_ = { 0.2f, 0.2f, 0.2f };

				WorldTransformUpdate(beamWorldTransform_);
				model_->Draw(beamWorldTransform_, camera);
			}
		}
	}
}

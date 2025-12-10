#include "Enemy.h"
#include "KamataEngine.h"
#include "Player.h"
#include <algorithm> // std::max
#include <cmath>     // atan2f, sqrtf

using namespace KamataEngine;

// フェーズごとのフレーム数（好みに合わせて調整）
namespace {
const int kMoveToPlaneFrames = 30;      // フェーズ1：ボス → プレイヤーZ平面
const int kMoveSideToEdgeFrames = 30;   // フェーズ2：Zを合わせた位置 → 画面端
const int kChargingFrames = 120;        // チャージ時間
const int kFiringFrames = 120;          // フェーズ3：照射継続時間
const int kRightExtraChargeFrames = 30; // 右側ファンネルだけチャージ時間を少し長くする

// ビームの太さ（見た目用：円柱モデルの半径）
const float kBeamRadius = 0.3f;

// 上から3本ビームの「軌道線」（地面に引くライン）の長さ
const float kTopBeamLaneLength = 40.0f; // 手前奥方向（Z方向）の長さ

// ★ 軌道線用
const float kTopBeamLaneHalfWidth = 0.5f; // X方向の半幅（太さ）
const float kTopBeamLaneHeight = 0.1f;    // 厚み（ほぼ0）


// 上から3本ビームの着弾マーカー用
const float kTopBeamMarkerRadius = 1.5f; // 床に出す丸の半径
const float kTopBeamMarkerHeight = 0.1f; // 厚み（ほぼペラ）
const float kTopBeamGroundY = 0.0f;      // 地面のY
} // namespace

namespace {

// dir（正規化済み）の方向を向く回転を計算して、モデル用のオフセットを足す
void SetWorldTransformLookDir(WorldTransform& wt, const Vector3& dir, float modelYawOffset = 0.0f, float modelPitchOffset = 0.0f) {

	float lenSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
	if (lenSq < 0.000001f) {
		return;
	}

	Vector3 nd = Normalized(dir);

	float yaw = std::atan2f(nd.x, nd.z);
	float pitch = std::atan2f(-nd.y, std::sqrtf(nd.x * nd.x + nd.z * nd.z));

	wt.rotation_.x = pitch + modelPitchOffset;
	wt.rotation_.y = yaw + modelYawOffset;
	wt.rotation_.z = 0.0f;
}

// プレイヤー方向に「水平だけ」向かせるヘルパ
// yawOffset でモデルの向き補正（左右で別オフセットを掛けたいとき用）
void FacePlayerHorizontally(WorldTransform& wt, const Vector3& funnelPos, const Vector3& playerPos, float yawOffset) {
	// プレイヤーへの水平ベクトル
	Vector3 dir{
	    playerPos.x - funnelPos.x,
	    0.0f, // ← 上下方向は無視して水平回転だけ
	    playerPos.z - funnelPos.z};

	float lenSq = dir.x * dir.x + dir.z * dir.z;
	if (lenSq < 0.000001f) {
		return;
	}

	float len = std::sqrt(lenSq);
	dir.x /= len;
	dir.z /= len;

	float yaw = std::atan2f(dir.x, dir.z);

	wt.rotation_.x = 0.0f; // 水平回転だけ
	wt.rotation_.y = yaw + yawOffset;
	wt.rotation_.z = 0.0f;
}

// ファンネル用：


// 左右で別オフセット（モデルの向き補正）
const float kFunnelModelYawOffsetLeft = ToRadians(90.0f);   // 左用
const float kFunnelModelYawOffsetRight = ToRadians(-90.0f); // 右用（必要に応じて ±90 / 180 に調整）

inline float GetFunnelYawOffset(int funnelIndex) { return (funnelIndex == 0) ? kFunnelModelYawOffsetLeft : kFunnelModelYawOffsetRight; }


// yaw は左右ごとの固定値にして、pitch だけターゲットの高さに向ける
void SetFunnelYawSidePitchToTarget(WorldTransform& wt, int funnelIndex, const Vector3& funnelPos, const Vector3& targetPos) {
	// 左右ごとの「真横」向きオフセット（既存のものを利用）
	float yawBase = GetFunnelYawOffset(funnelIndex);

	Vector3 diff = targetPos - funnelPos;

	// 水平距離（XZ 平面）
	float horiz = std::sqrt(diff.x * diff.x + diff.z * diff.z);
	float vert = diff.y;

	if (horiz < 0.0001f && std::fabs(vert) < 0.0001f) {
		wt.rotation_.x = 0.0f;
		wt.rotation_.y = yawBase;
		wt.rotation_.z = 0.0f;
		return;
	}

	// 上下の角度だけを見る（うなずき）
	float pitch = std::atan2f(-vert, horiz); // 上を見るときにマイナスになるように

	wt.rotation_.x = pitch;
	wt.rotation_.y = yawBase; // ★ yaw は常に左右固定
	wt.rotation_.z = 0.0f;
}

} // namespace

// ============================
// 初期化
// ============================
void Enemy::Initialize(const Vector3& bossBasePosition) {

	// 可視化用モデル
	model_ = Model::CreateFromOBJ("Boss");

	 // ★ 第二形態用モデルの読み込みポイント
	//   今は第一形態と同じモデルを使い回す。
	//   将来、第二形態専用モデルを使いたくなったら:
	//   secondFormModel_ = Model::CreateFromOBJ("Boss_Second");
	//   のように差し替える。
	secondFormModel_ = model_;

	beamModel_ = Model::CreateFromOBJ("beam");
	funnelModel_ = Model::CreateFromOBJ("Funnel");

	 // 上から3本ビームのマーカー用カラー初期化
	topBeamMarkerColor_.Initialize();
	// 薄い赤 (R G B A)
	topBeamMarkerColor_.SetColor(Vector4{1.0f, 0.3f, 0.3f, 0.3f});

	// -----------------------------
	// ボス本体の部位データを設定
	// -----------------------------

	// 0: 中央のコア
	bodyParts_[0].centerPosition = bossBasePosition + Vector3{0.0f, 0.5f, 0.0f};
	bodyParts_[0].boxSize = Vector3{2.0f, 1.0f, 1.0f};
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
		funnels_[i].wt.scale_ = {0.5f, 0.5f, 0.5f};
		funnels_[i].state = Funnel::Inactive;
		funnels_[i].timer = 0;
		funnels_[i].beamTarget = bossBasePosition; // 初期値は適当でOK（後で上書き）
	}

	// ★ 上から3本ビーム用初期化
	for (int i = 0; i < kVerticalBeamCount; ++i) {
		verticalBeams_[i].active = false;
		verticalBeams_[i].timer = 0;
		verticalBeams_[i].start = bossBasePosition;
		verticalBeams_[i].target = bossBasePosition;
	}

	// ビーム描画用 WorldTransform（全ビーム共通で使い回し）
	beamWorldTransform_.Initialize();
	beamWorldTransform_.scale_ = {kBeamRadius, kBeamRadius, 1.0f};

	 // 軌道線用 WT
	topBeamLaneWT_.Initialize(); 

	// ★ 軌道線用 WT ＋ 色
	topBeamLaneWorldTransform_.Initialize();

	topBeamLaneColor_.Initialize();
	topBeamLaneColor_.SetColor(Vector4{1.0f, 0.3f, 0.3f, 0.4f}); // 薄い赤

	// 最初の攻撃まで少し待つ
	funnelAttackCoolTimer_ = 180;
	topBeamAttackCoolTimer_ = 300; // ← 上から3本ビームの初回待ち（適当値）

		// 壊れていない本体部位だけ描画
	for (int i = 0; i < kBodyPartCount; ++i) {

		const BodyPart& part = bodyParts_[i];
		if (part.isDestroyed) {
			continue;
		}

		worldTransforms_[i].translation_ = part.centerPosition;
		worldTransforms_[i].scale_ = part.boxSize;

		WorldTransformUpdate(worldTransforms_[i]);
	}
}

// ============================
// 更新
// ============================
void Enemy::Update(const Vector3& playerPosition) {
	// 第一形態の本体(コア)を倒したら、復活して第二形態へ移行
	if (form == Form::ONE && bodyParts_[0].isDestroyed) {
		StartSecondForm();
	}
  
	bullets_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	 // ==============================
	// 形態1のときだけ上から3本ビームも動かす
	// ==============================
	if (form == Form::ONE) {

		// ファンネルのクール＆開始
		if (funnelAttackCoolTimer_ > 0) {
			--funnelAttackCoolTimer_;
		}
		if (funnelAttackCoolTimer_ == 0) {
			StartFunnelAttack(playerPosition);
		}

		// ★ 上から3本ビームのクール＆開始
		if (topBeamAttackCoolTimer_ > 0) {
			--topBeamAttackCoolTimer_;
		}
		if (topBeamAttackCoolTimer_ == 0) {
			StartTopBeamAttack(playerPosition);
		}

		// 状態更新
		UpdateFunnels(playerPosition);
		UpdateTopBeams(playerPosition);
	} else {
		// 形態2は今までどおり（ファンネルのクールは止めたいならここから外す）
		UpdateFunnels(playerPosition);
	}


	UpdateFunnels(playerPosition);

	switch (form) {
	case Form::TWO:
		bullets_.remove_if([](EnemyBullet* bullet) {
			if (bullet->IsDead()) {
				delete bullet;
				return true;
			}
			return false;
		});

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
}

// ============================
// 描画
// ============================
void Enemy::Draw(const Camera& camera) {

	if (!model_) {
		return;
	}

	// ★ 今の形態に応じて使うモデルを決める
	Model* currentModel = model_;
	if (form == Form::TWO && secondFormModel_) {
		currentModel = secondFormModel_;
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
		currentModel->Draw(worldTransforms_[i], camera);
	}

	// ファンネル描画（本体＋ビーム）
	DrawFunnels(camera);

	 // 形態1：上から3本ビーム描画
	if (form == Form::ONE) {
		DrawTopBeams(camera);
	}

	// 弾描画
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw(camera);
	}
}

// ============================
// AABB での被弾判定（部位破壊）
// ============================
bool Enemy::CheckHit(const Vector3& bulletPosition) {

	for (int bodyPartIndex = 0; bodyPartIndex < kBodyPartCount; ++bodyPartIndex) {

		BodyPart& bodyPart = bodyParts_[bodyPartIndex];

		if (bodyPart.isDestroyed) {
			continue;
		}

		Vector3 halfBoxSize = bodyPart.boxSize * 0.5f;
		Vector3 minPosition = bodyPart.centerPosition - halfBoxSize;
		Vector3 maxPosition = bodyPart.centerPosition + halfBoxSize;

		bool isInside = (bulletPosition.x >= minPosition.x && bulletPosition.x <= maxPosition.x) && (bulletPosition.y >= minPosition.y && bulletPosition.y <= maxPosition.y) &&
		                (bulletPosition.z >= minPosition.z && bulletPosition.z <= maxPosition.z);

		if (isInside) {
			bodyPart.hitPoint--;
			if (bodyPart.hitPoint <= 0) {
				bodyPart.isDestroyed = true;
			}
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

			f.beamTarget = playerPosition; // 左は素直にロック
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

			f.beamTarget = playerPosition; // 初回は一旦ロック
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

		// ★ 右ファンネル専用の“弱追従”処理（攻撃前のみ）
		if (i == 1 && (f.state == Funnel::MoveToPlane || f.state == Funnel::MoveSideToEdge || f.state == Funnel::Charging)) {

			// 右ファンネルは攻撃前はプレイヤーに「弱く」追従
			Vector3 desiredTarget = playerPosition;
			// 追従の強さ（0〜1）：小さいほどヌルっと追従して避けやすい
			const float followRate = 0.12f;
			f.beamTarget = Lerp(f.beamTarget, desiredTarget, followRate);

			// Z合わせ＆端も追従
			f.targetPlanePosition.z = playerPosition.z;
			float edgeX = +7.5f;
			f.edgePosition = {edgeX, playerPosition.y, playerPosition.z};

			// 見た目上も「プレイヤーの方を向いてる」感じにする
			SetFunnelYawSidePitchToTarget(f.wt, i, f.wt.translation_, f.beamTarget);
		}

		// ===============================
		// 通常フェーズ
		// ===============================
		switch (f.state) {
		case Funnel::Inactive:
			break;


		case Funnel::MoveToPlane: {
			if (f.timer > 0) {
				float t = 1.0f - (float)f.timer / (float)kMoveToPlaneFrames;

				f.wt.translation_ = Lerp(f.startPosition, f.targetPlanePosition, t);
				--f.timer;

				// ★ 真横向き固定＋うなずきだけターゲットを見る
				SetFunnelYawSidePitchToTarget(f.wt, i, f.wt.translation_, f.beamTarget);
			} else {
				f.state = Funnel::MoveSideToEdge;
				f.timer = kMoveSideToEdgeFrames;
			}
			break;
		}

		case Funnel::MoveSideToEdge: {
			if (f.timer > 0) {
				float t = 1.0f - (float)f.timer / (float)kMoveSideToEdgeFrames;

				f.wt.translation_ = Lerp(f.targetPlanePosition, f.edgePosition, t);
				--f.timer;

				// ★ 端へ移動中も銃口はプレイヤー（or beamTarget）の方向を向いたまま
				float yawOffset = GetFunnelYawOffset(i);
				FacePlayerHorizontally(f.wt, f.wt.translation_, f.beamTarget, yawOffset);
			} else {
				f.state = Funnel::Charging;
				if (i == 0) {
					f.timer = kChargingFrames;
				} else {
					f.timer = kChargingFrames + kRightExtraChargeFrames;
				}
			}
			break;
		}

		case Funnel::Charging: {

			// チャージ中は「ロックした（or 追従中の）ターゲット」にうなずく
			SetFunnelYawSidePitchToTarget(f.wt, i, f.wt.translation_, f.beamTarget);

			if (f.timer > 0) {
				--f.timer;
			} else {
				f.state = Funnel::Firing;
				f.timer = kFiringFrames;
			}
			break;
		}

		case Funnel::Firing: {

			// 照射中も「ロックした方向」にうなずいた姿勢のまま
			SetFunnelYawSidePitchToTarget(f.wt, i, f.wt.translation_, f.beamTarget);

			if (f.timer > 0) {
				--f.timer;
			} else {
				f.state = Funnel::Inactive;
			}
			break;
		}
		}
	}
}



bool Enemy::IsDefeated() const {
	// 第二形態のコア(0番)を壊したときだけ、ボス撃破扱いにする
	return (form == Form::TWO) && bodyParts_[0].isDestroyed;
}


// ============================
// 上から3本ビーム攻撃：開始
// ============================
void Enemy::StartTopBeamAttack(const Vector3& playerPosition) {

	// すでにどれかが動いている場合は新規発動しない
	for (int i = 0; i < kVerticalBeamCount; ++i) {
		if (verticalBeams_[i].active) {
			return;
		}
	}

	const float kSpacing = 2.5f;   // X方向の間隔
	const float kHeight = 6.0f;    // プレイヤーの上何ユニットから撃つか
	const float kDepthDown = 4.0f; // プレイヤーよりどれだけ下まで落とすか
	const int kCoolFrames = 240;   // 再使用までのクールタイム

	float baseX = playerPosition.x;
	float baseY = playerPosition.y;
	float z = playerPosition.z;

	// i = 0,1,2 → -1,0,+1 として左右に並べる
	for (int i = 0; i < kVerticalBeamCount; ++i) {
		VerticalBeam& b = verticalBeams_[i];

		float offsetX = static_cast<float>(i - 1) * kSpacing;

		b.active = true;
		b.timer = kFiringFrames; // 既存の照射時間を流用

		b.start = {baseX + offsetX, baseY + kHeight, z};
		b.target = {baseX + offsetX, baseY - kDepthDown, z};
	}

	// 次にこの攻撃ができるまでのクールをセット
	topBeamAttackCoolTimer_ = kCoolFrames;
}

// ============================
// 上から3本ビーム攻撃：更新
// ============================
void Enemy::UpdateTopBeams(const Vector3& /*playerPosition*/) {

	for (int i = 0; i < kVerticalBeamCount; ++i) {

		VerticalBeam& b = verticalBeams_[i];
		if (!b.active) {
			continue;
		}

		if (b.timer > 0) {
			--b.timer;
		} else {
			b.active = false;
		}
	}

	// クールタイマーは Update 側で減らす（後述）
}




void Enemy::DrawTopBeams(const Camera& camera) {

	if (!beamModel_) {
		return;
	}

	// 見た目の最低長（短すぎると画面に映えないのでファンネルと同じ値）
	const float kMinVisualLength = 30.0f;
	const float kStartOffset = 0.5f;

	for (int i = 0; i < kVerticalBeamCount; ++i) {

		// ★ この1行で b を宣言している（この {} の中だけで有効）
		const VerticalBeam& b = verticalBeams_[i];
		if (!b.active || b.timer <= 0) {
			continue;
		}

		// ===== 縦ビーム本体の描画 =====
		Vector3 beamStart = b.start;
		Vector3 beamEnd = b.target;

		Vector3 dir{
		    beamEnd.x - beamStart.x,
		    beamEnd.y - beamStart.y,
		    beamEnd.z - beamStart.z,
		};

		float realLength = Length(dir);
		if (realLength < 0.01f) {
			continue;
		}

		Vector3 nd = Normalized(dir);

		float visualLength = realLength;
		if (visualLength < kMinVisualLength) {
			visualLength = kMinVisualLength;
		}

		// ちょっとだけ前に出してから中点を取る
		beamStart.x += nd.x * kStartOffset;
		beamStart.y += nd.y * kStartOffset;
		beamStart.z += nd.z * kStartOffset;

		Vector3 visualEnd{
		    beamStart.x + nd.x * visualLength,
		    beamStart.y + nd.y * visualLength,
		    beamStart.z + nd.z * visualLength,
		};

		Vector3 midPos{
		    (beamStart.x + visualEnd.x) * 0.5f,
		    (beamStart.y + visualEnd.y) * 0.5f,
		    (beamStart.z + visualEnd.z) * 0.5f,
		};

		// 縦ビーム円柱
		beamWorldTransform_.translation_ = midPos;
		beamWorldTransform_.scale_ = {kBeamRadius, kBeamRadius, visualLength * 0.5f};

		SetWorldTransformLookDir(beamWorldTransform_, nd);
		WorldTransformUpdate(beamWorldTransform_);

		beamModel_->Draw(beamWorldTransform_, camera);

		// ===== ここから「軌道線」（地面に伸びる赤ライン） =====

		// 本来の start→target ベクトル
		Vector3 fullDir{
		    b.target.x - b.start.x,
		    b.target.y - b.start.y,
		    b.target.z - b.start.z,
		};

		// Y = kTopBeamGroundY との交点 t を計算
		float t = 1.0f;
		if (std::fabs(fullDir.y) > 0.0001f) {
			t = (kTopBeamGroundY - b.start.y) / fullDir.y;
		}
		// 0〜1 にクランプ（線分内に限定）
		if (t < 0.0f)
			t = 0.0f;
		if (t > 1.0f)
			t = 1.0f;

		Vector3 hitPos{
		    b.start.x + fullDir.x * t,
		    b.start.y + fullDir.y * t,
		    b.start.z + fullDir.z * t,
		};

		// 軌道線は「着弾点の真下の床」を中心に、Z方向へ伸ばす
		topBeamLaneWorldTransform_.translation_ = hitPos;
		topBeamLaneWorldTransform_.translation_.y = kTopBeamGroundY;

		topBeamLaneWorldTransform_.scale_ = {
		    kTopBeamLaneHalfWidth,     // X方向の太さ
		    kTopBeamLaneHeight,        // 厚み
		    kTopBeamLaneLength * 0.5f, // Z方向の長さ/2
		};

		WorldTransformUpdate(topBeamLaneWorldTransform_);
		beamModel_->Draw(topBeamLaneWorldTransform_, camera, &topBeamLaneColor_);
	}
}


//第二形態の関数

void Enemy::StartSecondForm() {
	// 第二形態スタート
	form = Form::TWO;

	// --- コアの復活＆HP再設定 ---
	BodyPart& core = bodyParts_[0];
	core.hitPoint = 5; // 第二形態用HP（好みで調整してOK）
	core.isDestroyed = false;

	// 見た目のワールドトランスフォームも復活
	worldTransforms_[0].translation_ = core.centerPosition;
	worldTransforms_[0].scale_ = core.boxSize;
	WorldTransformUpdate(worldTransforms_[0]);

	// 第一形態専用攻撃のクールタイムをリセット
	topBeamAttackCoolTimer_ = 0;

	// 第二形態開始前に少し溜めを入れたいなら値を増やす
	funnelAttackCoolTimer_ = 60; // 1秒ぐらい。すぐ撃たせたいなら 0。
}



// ============================
// ファンネル描画（本体＋円柱ビーム）
// ============================
void Enemy::DrawFunnels(const Camera& camera) {

	if (!funnelModel_) {
		return;
	}

	for (int i = 0; i < kFunnelCount; ++i) {
		Funnel& f = funnels_[i];
		if (f.state == Funnel::Inactive) {
			continue;
		}

		// ファンネル本体
		WorldTransformUpdate(f.wt);
		funnelModel_->Draw(f.wt, camera);

		// ビーム描画（照射中のみ）
		if (f.state == Funnel::Firing) {

			// 始点：ファンネルの位置
			Vector3 beamStart = f.wt.translation_;
			// 少し下げる調整（銃口の位置合わせ）
			beamStart.y -= 1.4f;

			// 本来のターゲット（当たり判定はこれを使う）
			Vector3 realEnd = f.beamTarget;

			// 方向ベクトル
			Vector3 dir{realEnd.x - beamStart.x, realEnd.y - beamStart.y, realEnd.z - beamStart.z};

			// ★ 左ファンネルだけは見た目のビーム方向を「+X 方向」に固定
			if (i == 0) {
				// X軸まっすぐ（画面左→右）にしたいので +X 方向固定
				dir = {1.0f, 0.0f, 0.0f};
			}

			float realLength = Length(dir);
			if (realLength < 0.01f) {
				continue;
			}

			Vector3 nd = Normalized(dir);

			// 見た目用の長さ（最低 30.0f までは伸ばす）
			const float kMinVisualLength = 30.0f;
			float visualLength = realLength;
			if (visualLength < kMinVisualLength) {
				visualLength = kMinVisualLength;
			}

			// 見た目用の終点
			Vector3 visualEnd{beamStart.x + nd.x * visualLength, beamStart.y + nd.y * visualLength, beamStart.z + nd.z * visualLength};

			// 銃口から少し前に出す
			const float kStartOffset = 0.5f;
			beamStart.x += nd.x * kStartOffset;
			beamStart.y += nd.y * kStartOffset;
			beamStart.z += nd.z * kStartOffset;

			// 中点
			Vector3 midPos{(beamStart.x + visualEnd.x) * 0.5f, (beamStart.y + visualEnd.y) * 0.5f, (beamStart.z + visualEnd.z) * 0.5f};

			float yaw = std::atan2f(nd.x, nd.z);
			float pitch = std::atan2f(-nd.y, std::sqrtf(nd.x * nd.x + nd.z * nd.z));

			// ★ ここで Initialize() は呼ばない！
			beamWorldTransform_.translation_ = midPos;
			beamWorldTransform_.rotation_ = {pitch, yaw, 0.0f};
			beamWorldTransform_.scale_ = {kBeamRadius, kBeamRadius, visualLength * 0.5f};

			WorldTransformUpdate(beamWorldTransform_);

			if (beamModel_) {
				beamModel_->Draw(beamWorldTransform_, camera);
			} else if (model_) {
				model_->Draw(beamWorldTransform_, camera);
			}
		}
	}
}

// ============================
// ビーム当たり判定
// ============================
bool Enemy::IsPlayerHitByFunnelBeam(const Vector3& playerPosition, float playerRadius) {

	// ビーム見た目よりかなり小さい当たり判定にして、理不尽ヒットを減らす
	const float kBeamHitRadiusScale = 0.3f; // ← ここでさらに小さくしている
	float beamHitRadius = kBeamRadius * kBeamHitRadiusScale;
	float combinedRadius = playerRadius + beamHitRadius;
	float combinedRadiusSq = combinedRadius * combinedRadius;

	// 描画側と合わせるためのパラメータ
	const float kMinVisualLength = 30.0f; // DrawFunnels の kMinVisualLength と揃える
	const float kStartOffset = 0.5f;      // 銃口から少し前に出す量（DrawFunnels と揃える）
	const float kMuzzleYOffset = 1.4f;    // beamStart.y -= 1.4f と同じ

	for (int i = 0; i < kFunnelCount; ++i) {

		const Funnel& f = funnels_[i];
		if (f.state != Funnel::Firing) {
			continue;
		}

		// ★ 始点：描画と同じように補正する
		Vector3 beamStart = f.wt.translation_;
		beamStart.y -= kMuzzleYOffset;

		// 方向ベクトル dir を描画と揃える
		Vector3 dir;

		if (i == 0) {
			// 左ファンネル: 見た目は +X 方向固定
			dir = {1.0f, 0.0f, 0.0f};
		} else {
			// 右ファンネル: ロックしたターゲットに向かう
			Vector3 rawDir = f.beamTarget - beamStart;
			float len = Length(rawDir);
			if (len < 0.0001f) {
				continue;
			}
			dir = rawDir / len;
		}

		float visualLength = kMinVisualLength; // 当たり判定用の長さも最低値に固定でOK
		Vector3 beamEnd = {beamStart.x + dir.x * visualLength, beamStart.y + dir.y * visualLength, beamStart.z + dir.z * visualLength};

		// 銃口オフセット（描画と同じく少し前に出す）
		beamStart.x += dir.x * kStartOffset;
		beamStart.y += dir.y * kStartOffset;
		beamStart.z += dir.z * kStartOffset;

		// ここから先は「線分 [beamStart, beamEnd] と球の最短距離」判定
		Vector3 seg = beamEnd - beamStart;
		float segLenSq = seg.x * seg.x + seg.y * seg.y + seg.z * seg.z;
		if (segLenSq <= 0.0001f) {
			continue;
		}

		Vector3 toPlayer = playerPosition - beamStart;
		float dot = toPlayer.x * seg.x + toPlayer.y * seg.y + toPlayer.z * seg.z;
		float t = dot / segLenSq;
		if (t < 0.0f)
			t = 0.0f;
		if (t > 1.0f)
			t = 1.0f;

		Vector3 closestPoint = {beamStart.x + seg.x * t, beamStart.y + seg.y * t, beamStart.z + seg.z * t};

		Vector3 diff = {playerPosition.x - closestPoint.x, playerPosition.y - closestPoint.y, playerPosition.z - closestPoint.z};

		float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		if (distSq <= combinedRadiusSq) {
			return true;
		}
	}

	return false;
}

Vector3 Enemy::GetWorldPosition() const { return worldTransforms_->translation_; }

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

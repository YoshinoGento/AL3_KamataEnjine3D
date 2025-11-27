#pragma once
#include "EnemyBullet.h"
#include "KamataEngine.h"
#include "MatrixMath.h"


using namespace KamataEngine;

class Enemy {
public:
	// 初期化：bossBasePosition を中心に部位を配置する
	void Initialize(const Vector3& bossBasePosition);

	// 更新：プレイヤーのワールド座標を参照（ファンネル攻撃などに利用）
	void Update(const Vector3& playerPosition);


	// 描画
	void Draw(const Camera& camera);

	///< summary>
	/// 弾の位置を受け取って、どれかの部位に当たったらtrueを返す
	/// （当たった部位のHPを減らして、0以下なら部位破壊扱い）
	///</summary>
	bool CheckHit(const Vector3& bulletPosition);

private:
	// =========================
	// ボス本体の部位
	// =========================
	struct BodyPart {
		Vector3 centerPosition; // 矩形の中心座標（ワールド）
		Vector3 boxSize;        // 矩形のサイズ（X,Y,Z 幅）
		int32_t hitPoint;       // 耐久値
		bool isDestroyed;       // 破壊済みフラグ
	};

	static const int kBodyPartCount = 3;
	BodyPart bodyParts_[kBodyPartCount];

	// 各部位の見た目用のワールド変換
	WorldTransform worldTransforms_[kBodyPartCount];

	// 可視化用モデル（暫定で "player" モデルを流用）
	Model* model_ = nullptr;

	// =========================
	// ファンネル攻撃用
	// =========================
	struct Funnel {
		enum State {
			Inactive,       // 待機
			MoveToPlane,    // ボス付近 → プレイヤーと同じZ平面まで前進
			MoveSideToEdge, // プレイヤーと同じZ平面で、画面端まで横移動
			Firing,         // 画面端から照射中
		};

		WorldTransform wt; // ファンネル本体のワールド変換
		State state = Inactive;

		// 現在の状態に残っているフレーム数
		int timer = 0;

		// フェーズ1開始位置（ボス側の発進位置）
		Vector3 startPosition;

		// フェーズ1終了位置（プレイヤーと同じZの位置）
		Vector3 targetPlanePosition;

		// フェーズ2終了位置（画面端での照射位置）
		Vector3 edgePosition;

		// ビーム照射方向のターゲット（ロックしたプレイヤー位置）
		Vector3 beamTarget;

		// true: 左端から照射 / false: 右端から照射
		bool fromLeft = true;
	};

	static const int kFunnelCount = 2;
	Funnel funnels_[kFunnelCount];

	//ビーム攻撃全体で使いまわす一時
	WorldTransform beamWorldTransform_;

	// ファンネル攻撃のクールタイム（フレーム数）
	int funnelAttackCoolTimer_ = 0;

	// ファンネル攻撃を開始（空きがあれば1機だけ使用）
	void StartFunnelAttack(const Vector3& playerPosition);

	// ファンネルの状態更新
	void UpdateFunnels(const Vector3& playerPosition);

	// ファンネルの描画
	void DrawFunnels(const Camera& camera);
};

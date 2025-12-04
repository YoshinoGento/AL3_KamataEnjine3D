#pragma once
#include "KamataEngine.h"
#include "MatrixMath.h"

using namespace KamataEngine;

/// <summary>
/// ボス本体＋ファンネル照射攻撃をもつ敵クラス
/// </summary>
class Enemy {
public:
	/// <summary>
	/// 初期化：bossBasePosition を中心にボスの部位を配置する
	/// </summary>
	void Initialize(const Vector3& bossBasePosition);

	/// <summary>
	/// 更新：プレイヤーのワールド座標を参照してファンネル攻撃などを進行させる
	/// </summary>
	void Update(const Vector3& playerPosition);

	/// <summary>
	/// 描画：ボス本体＋ファンネル＋ビームを描画する
	/// </summary>
	void Draw(const Camera& camera);

	/// <summary>
	/// プレイヤー弾との当たり判定
	/// 弾のワールド座標を受け取り、どれかの部位に当たったら true を返す。
	/// 当たった部位の HP を減らし、0 以下ならその部位は破壊状態になる。
	/// </summary>
	bool CheckHit(const Vector3& bulletPosition);

	/// <summary>
	/// ファンネルの照射ビームにプレイヤーが当たっているか判定する
	/// </summary>
	/// <param name="playerPosition">プレイヤーのワールド座標</param>
	/// <param name="playerRadius">プレイヤー当たり判定の半径</param>
	/// <returns>true ならどれかのビームに被弾</returns>
	bool IsPlayerHitByFunnelBeam(const Vector3& playerPosition, float playerRadius);

	Vector3 GetWorldPosition() const;

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

	// ビーム専用モデル（円柱）
	Model* beamModel_ = nullptr;

	// =========================
	// ファンネル攻撃用
	// =========================
	struct Funnel {
		/// <summary>
		/// ファンネル 1 機の状態
		/// </summary>
		enum State {
			Inactive,       // 待機
			MoveToPlane,    // ボス付近 → プレイヤーと同じZ平面まで前進
			MoveSideToEdge, // プレイヤーと同じZ平面で、画面端まで横移動
			Charging,       // 画面端でチャージ中
			Firing,         // 画面端から照射中
		};

		WorldTransform wt; // ファンネル本体のワールド変換

		WorldTransform worldTransform_; // Bossのワールド変換

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

	// ビーム描画用 WorldTransform（全ビームで使い回し）
	WorldTransform beamWorldTransform_;

	// ファンネル攻撃のクールタイム（フレーム数）
	int funnelAttackCoolTimer_ = 0;

	/// <summary>
	/// ファンネル攻撃を開始（空きがあれば1機だけ使用）
	/// </summary>
	void StartFunnelAttack(const Vector3& playerPosition);

	/// <summary>
	/// ファンネルの状態更新（L字移動＋照射時間の進行）
	/// </summary>
	void UpdateFunnels(const Vector3& playerPosition);

	/// <summary>
	/// ファンネルの描画（本体＋ビーム）
	/// </summary>
	void DrawFunnels(const Camera& camera);
};

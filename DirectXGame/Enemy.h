#pragma once
#include "KamataEngine.h"
#include "MatrixMath.h"
#include "EnemyBullet.h"

class Player;

using namespace KamataEngine;

/// <summary>
/// ボス本体＋ファンネル照射攻撃をもつ敵クラス
/// </summary>
class Enemy {
public:
	enum struct Form {
		ONE,
		TWO,
	};


	 // ★ 形態取得
	Form GetForm() const { return form; }

	// ★ 撃破判定（コア破壊で true）
	bool IsDefeated() const;

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

	void SetPlayer(Player* player) { player_ = player; }

	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }

	// ファンネル攻撃を開始（空きがあれば1機だけ使用）
	void StartFunnelAttack(const Vector3& playerPosition);

	/// <summary>
	/// ファンネルの状態更新（L字移動＋照射時間の進行）
	/// </summary>
	void UpdateFunnels(const Vector3& playerPosition);

	/// <summary>
	/// ファンネルの描画（本体＋ビーム）
	/// </summary>
	void DrawFunnels(const Camera& camera);

	// 上から3本ビームの描画（第一形態）
	void DrawTopBeams(const Camera& camera);

	// ミサイルの発射
	void ShootMissile();

private:

	 // 上から3本ビーム攻撃（開始／更新）
	void StartTopBeamAttack(const Vector3& playerPosition);
	void UpdateTopBeams(const Vector3& playerPosition);

	 // 第一形態撃破 → 第二形態へ移行するときの処理
	void StartSecondForm();

	// =========================
	// ボス本体の部位
	// =========================
	struct BodyPart {
		Vector3 centerPosition; // 矩形の中心座標（ワールド）
		Vector3 boxSize;        // 矩形のサイズ（X,Y,Z 幅）
		int32_t hitPoint;       // 耐久値
		bool isDestroyed;       // 破壊済みフラグ
	};

	// =========================
	// 上から3本のファンネルビーム（第一形態用）
	// =========================
	struct VerticalBeam {
		bool active = false; // true なら照射中
		int timer = 0;       // 残りフレーム数
		Vector3 start;       // ビーム始点（上側）
		Vector3 target;      // ビーム終点（下側 or ロックした位置）
	};

	static const int kVerticalBeamCount = 3;
	VerticalBeam verticalBeams_[kVerticalBeamCount];

	// 上から3本ビーム攻撃のクールタイム（フレーム数）
	int topBeamAttackCoolTimer_ = 0;


	static const int kBodyPartCount = 3;
	BodyPart bodyParts_[kBodyPartCount];

	// 各部位の見た目用のワールド変換
	WorldTransform worldTransforms_[kBodyPartCount];

	// 可視化用モデル（暫定で "player" モデルを流用）
	Model* model_ = nullptr;

	// ★ 第二形態用モデル
	//   今は第一形態と同じモデルを使い回す。
	//   見た目を変えたくなったら Initialize() 内で
	//   secondFormModel_ に別OBJを読み込んで、
	//   Draw() で form に応じて使い分ける。
	Model* secondFormModel_ = nullptr;
	
	

	// ビーム専用モデル（円柱）
	Model* beamModel_ = nullptr;

	// ファンネル専用モデル
	Model* funnelModel_ = nullptr;

	WorldTransform beamWorldTransform_;
	// 追加：軌道線用の WT
	WorldTransform topBeamLaneWT_;

	// 上から3本ビームの「着弾マーカー」用カラー
	ObjectColor topBeamMarkerColor_;

	// =========================
	// 上から3本ビーム用（軌道線）
	// =========================
	WorldTransform topBeamLaneWorldTransform_;
	ObjectColor topBeamLaneColor_;

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
	//WorldTransform beamWorldTransform_;

	// ファンネル攻撃のクールタイム（フレーム数）
	int funnelAttackCoolTimer_ = 0;

	// プレイヤー
	Player* player_ = nullptr;

	// ミサイル
	std::list<EnemyBullet*> bullets_;
	
	// 発射クールタイム
	int fireTimer = 0;
	// 発射間隔
	static const int kFireInterval = 600;

	// 形態
	Form form = Form::ONE;

	
};

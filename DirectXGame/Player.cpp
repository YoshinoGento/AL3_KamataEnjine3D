#include "Player.h"
#include "MatrixMath.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

void Player::Initialize(Model* playerModel, Model* playerBulletModel, Camera* camera, const Vector3& position) {
	assert(playerModel);
	assert(playerBulletModel);
	model_ = playerModel;
	player_bullet_model_ = playerBulletModel;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	input_ = Input::GetInstance();
	hitPoint_ = 3;
	invincibleTimer_ = 0;
	beamCharger_.Initialize();
	barrier_.Initialize();
	isChargingBeam_ = false;

	lockonTexHandle_ = TextureManager::Load("lockon.png");
	lockonSprite_ = Sprite::Create(lockonTexHandle_, {0, 0}, {1, 1, 1, 1}, {0.5f, 1.0f});
	lockonSprite_->SetSize({64.0f, 64.0f}); // ここでサイズ調整！
}

void Player::Update() {

	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	Vector3 move = {0.0f, 0.0f, 0.0f};
	const float kCharacterSpeed = 0.01f;
	const float kFriction = 0.9f;
	const float kMaxSpeed = 3.0f;

	if (invincibleTimer_ > 0) {
		--invincibleTimer_;
	}

	if (input_->PushKey(DIK_W)) {
		move.y += kCharacterSpeed;
	}
	if (input_->PushKey(DIK_S)) {
		move.y -= kCharacterSpeed;
	}
	if (input_->PushKey(DIK_D)) {
		move.x += kCharacterSpeed;
	}
	if (input_->PushKey(DIK_A)) {
		move.x -= kCharacterSpeed;
	}

	velocity_ += move;
	if (Length(velocity_) > kMaxSpeed) {
		velocity_ = Normalized(velocity_) * kMaxSpeed;
	}

	velocity_ *= kFriction;
	worldTransform_.translation_ += velocity_;

	const float kMoveLimitX = 6.0f;
	const float kMoveLimitY = 3.0f;

	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -kMoveLimitX, +kMoveLimitX);
	worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, -kMoveLimitY, +kMoveLimitY);

	// 入力の現在状態を取得
	bool isEPressed = input_->PushKey(DIK_E);

	// クールタイム中だったらトグル処理だけスキップ
	bool skipToggle = barrier_.IsInCooldown();

	// Eキーが「前回離れてて、今回押された」場合
	if (!wasETrigger_ && isEPressed && !skipToggle) {
		isChargingBeam_ = !isChargingBeam_;

		if (isChargingBeam_) {
			if (barrier_.IsBroken()) {
				barrier_.Reset();
			}
			barrier_.SetActive(true);
		} else {
			barrier_.SetActive(false);

			float chargePower = beamCharger_.Consume();

			Beam* beam = new Beam();
			Vector3 front = TransformNormal({0, 0, 1}, worldTransform_.matWorld_);
			Vector3 target = worldTransform_.translation_ + front * 100.0f;
			beam->Initialize(worldTransform_.translation_, target, chargePower);
			beams_.push_back(beam);

			barrier_.Break();
		}
	}

	// 次フレーム用に保存
	wasETrigger_ = isEPressed;

	Attack();

	// ✅ ←★ ここに追加！弾の段階的発射
	if (isFiringFanMissiles_) {
		fireTimer_++;

		if (fireTimer_ >= fireInterval_) {
			fireTimer_ = 0;

			if (lockedEnemy_ && fireCount_ < 6) {
				Vector3 enemyPos = lockedEnemy_->GetWorldPosition();

				Vector3 offset = {((rand() % 200) - 100) / 10.0f, ((rand() % 200)) / 10.0f + 5.0f, ((rand() % 200) - 100) / 10.0f};

				HomingArcBullet* arcBullet = new HomingArcBullet();
				arcBullet->Initialize(player_bullet_model_, worldTransform_.translation_, enemyPos, offset);
				arcBullets_.push_back(arcBullet);

				fireCount_++;
			}

			if (fireCount_ >= 6) {
				isFiringFanMissiles_ = false;
				lockedEnemy_ = nullptr; // ← ★ これでロック解除！
			}
		}
	}

	//--------ビーム---------------
	for (auto* beam : beams_) {
		beam->Update();
	}
	beams_.remove_if([](Beam* b) {
		if (b->IsDead()) {
			delete b;
			return true;
		}
		return false;
	});

	for (PlayerBullet* bullet : bullets_) {
		bullet->Update();
	}

	for (HomingArcBullet* arcBullet : arcBullets_) {
		arcBullet->Update();
	}

	missileParticles_.remove_if([](MissilePartocle* p) {
		if (p->IsDead()) {
			delete p;
			return true;
		}
		return false;
	});

	arcBullets_.remove_if([](HomingArcBullet* b) {
		if (b->IsDead()) {
			delete b;
			return true;
		}
		return false;
	});

	// 最後の方がベスト
	barrier_.Update();

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void Player::Draw() {

	// 無敵中は点滅させる
	if (invincibleTimer_ > 0) {
		// 5フレームごとに ON/OFF 切り替え（点滅周期）
		if (((invincibleTimer_ / 5) % 2) == 0) {
			// このフレームは描画しない → 透明になる
			return;
		}
	}

	model_->Draw(worldTransform_, *camera_);

	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw(*camera_);
	}
	for (HomingArcBullet* bullet : arcBullets_) {
		bullet->Draw(*camera_);
	}
	for (auto* beam : beams_) {
		beam->Draw(*camera_);
	}
	if (!barrier_.IsBroken()) {
		barrier_.Draw(*camera_, worldTransform_.translation_);
	}

	beamCharger_.Draw(*camera_);


	// 🔽 スプライト描画前処理
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());

	// 🔽 ロックオンUI描画
	if (lockedEnemy_ && lockonSprite_) {
		// ① 敵のワールド座標を取得
		Vector3 enemyWorldPos = lockedEnemy_->GetWorldPosition();

		// ② 頭上にオフセット（例：+1.5f 上に）
		enemyWorldPos.y += 1.5f;

		// ③ スクリーン座標に変換
		Vector2 screenPos = ProjectToScreen(enemyWorldPos, camera_->matView, camera_->matProjection, 1280, 720);

		// ④ UI表示位置にセット＆描画
		lockonSprite_->SetPosition(screenPos);
		lockonSprite_->Draw();
	}


	// 🔼 スプライト描画後処理
	Sprite::PostDraw();

	/*if (isChargingBeam_) {
	    DrawChargeEffect(*camera_);
	}*/
}

void Player::Attack() {
	// スペースキー or 左クリックで通常ショット
	if (input_->TriggerKey(DIK_SPACE) || input_->IsTriggerMouse(0)) {
		const float kBulletSpeed = 1.0f;

		// ローカルZ+方向（前）に飛ぶ速度ベクトル
		Vector3 velocity(0.0f, 0.0f, kBulletSpeed);
		velocity = TransformNormal(velocity, worldTransform_.matWorld_);

		// 弾の生成
		PlayerBullet* newBullet = new PlayerBullet();

		newBullet->Initialize(player_bullet_model_, worldTransform_.translation_, velocity);

		// 弾リストに登録
		bullets_.push_back(newBullet);
	}
	//=================================

	//================ロックオン・ミサイル発射処理================
	// ロックオン処理（右クリックなど）
	if (input_->IsTriggerMouse(1)) { // 右クリックでロックオン
		lockedEnemy_ = enemy_;
	}

	// 発射（左クリック）
	if (input_->IsTriggerMouse(0)) {
		if (!isFiringFanMissiles_ && lockedEnemy_) {
			isFiringFanMissiles_ = true;
			fireTimer_ = 0;
			fireCount_ = 0;
		}
	}
	// キーでロック解除（例：Rキー）
	if (input_->TriggerKey(DIK_R)) {
		lockedEnemy_ = nullptr;
	}
	//========================================================
}

void Player::FireToward(const Vector3& targetWorld) {

	// 弾の速度
	const float kBulletSpeed = 1.0f;

	// 自機から狙い点への方向ベクトルを計算
	const Vector3& playerPos = worldTransform_.translation_;

	// 方向ベクトル(プレイヤー → 狙い点)
	Vector3 direction = targetWorld - playerPos;

	// 正規化して速度ベクトルに変換
	direction = Normalized(direction);

	// 実際の速度
	Vector3 velocity = direction * kBulletSpeed;

	// 弾を生成し、初期化
	PlayerBullet* newBullet = new PlayerBullet();
	newBullet->Initialize(model_, playerPos, velocity);

	// 弾を登録する
	bullets_.push_back(newBullet);
}

void Player::OnHitByBeam() {
	// バリアが壊れている状態なら通常のダメージ処理
	if (barrier_.IsBroken()) {
		if (invincibleTimer_ > 0)
			return;

		--hitPoint_;
		if (hitPoint_ < 0)
			hitPoint_ = 0;

		invincibleTimer_ = 60; // 無敵時間セット
		return;
	}

	// ここに来た = バリアがまだある → 攻撃を吸収
	float absorbPower = 30.0f; // 1発分の吸収量（仮）

	barrier_.Absorb(absorbPower);     // バリアに吸収させる
	beamCharger_.Absorb(absorbPower); // チャージにも吸収反映

	// バリアがここで壊れたら、ここでエフェクトなど
	if (barrier_.IsBroken()) {
		barrier_.Break(); // ← これでクールタイムへ
	}
}

bool Player::OnHitMissile(const Vector3& bulletPosition) {
	// AABB の min / max を計算
	const float kPlayerRadius = 0.5f; // プレイヤー当たり判定の半径
	Vector3 minPosition = GetWorldPosition() - kPlayerRadius;
	Vector3 maxPosition = GetWorldPosition() + kPlayerRadius;

	// 点（弾）の位置が AABB 内にあるか判定
	bool isInside = (bulletPosition.x >= minPosition.x && bulletPosition.x <= maxPosition.x) && (bulletPosition.y >= minPosition.y && bulletPosition.y <= maxPosition.y) &&
	                (bulletPosition.z >= minPosition.z && bulletPosition.z <= maxPosition.z);

	if (isInside) {
		// ヒットしたのでダメージ
		hitPoint_--;

		// 1発で複数部位に当たらないように、ここで終了
		return true;
	}

	return false;
}

Player::~Player() {
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	for (HomingArcBullet* bullet : arcBullets_) {
		delete bullet;
	}
}

void Player::SetEnemy(Enemy* enemy) { enemy_ = enemy; }

void Player::DrawChargeEffect(const Camera& camera) {
	float rate = beamCharger_.GetChargeRate(); // 0.0〜1.0

	if (rate <= 0.0f)
		return;

	// チャージエフェクトの色（青→赤）
	Vector4 color = Lerp(Vector4{0, 0, 1, 1}, Vector4{1, 0, 0, 1}, rate);

	// チャージ演出（Sphereなど）
	WorldTransform wt;
	wt.Initialize();
	wt.translation_ = worldTransform_.translation_ + Vector3{0, 0.5f, 0}; // 頭上
	wt.scale_ = Vector3{0.5f, 0.5f, 0.5f} * rate;

	WorldTransformUpdate(wt);

	Model* chargeModel = Model::CreateFromOBJ("ChargeEffect");
	if (!chargeModel)
		return;

	// 正しい使い方
	ObjectColor objColor;
	objColor.Initialize();
	objColor.SetColor(color);

	chargeModel->Draw(wt, camera, &objColor);
}

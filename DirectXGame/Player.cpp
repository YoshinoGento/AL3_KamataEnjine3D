#include "Player.h"
#include "MatrixMath.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

void Player::Initialize(Model* model, Camera* camera, const Vector3& position, uint32_t lockonTexture) {
	assert(model);
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	input_ = Input::GetInstance();
	lolckOn_.Initialize(lockonTexture);

	// 狙う位置の奥行き
	aimPlaneZ_ = 40.0f;

	// HP初期化
	hitPoint_ = 10;

	// 当たり判定半径
	radius_ = 1.0f;

	// HP画像の読み込み
	// ※「hp.png」という画像をResourcesフォルダに入れてください
	hpTextureHandle_ = TextureManager::Load("hp.png");

	// HP表示用スプライトの生成
	// 最大HP分（10個）作っておく
	for (int i = 0; i < 10; ++i) {
		Sprite* s = Sprite::Create(hpTextureHandle_, {0, 0});
		// サイズを小さく指定
		s->SetSize({32.0f, 32.0f});
		hpIcons_.push_back(s);
	}
}

void Player::Update() {

	// 無敵タイマーを減らす
	if (invincibleTimer_ > 0) {
		--invincibleTimer_;
	}

	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	arcBullets_.remove_if([](HomingArcBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	Vector3 move = {0.0f, 0.0f, 0.0f};
	const float kCharacterSpeed = 0.01f; // 移動速度調整
	const float kFriction = 0.9f;
	const float kMaxSpeed = 3.0f;

	// キーボード移動
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

	// 速度加算
	velocity_ += move;

	// 摩擦（減速）
	velocity_ *= kFriction;

	// 最大速度制限
	if (Length(velocity_) > kMaxSpeed) {
		velocity_ = Normalized(velocity_) * kMaxSpeed;
	}

	// 座標更新
	worldTransform_.translation_ += velocity_;

	// ★追加：移動制限（画面外に出ないように）
	// 数値は画面サイズやカメラ距離に応じて調整してください
	const float kMoveLimitX = 6.0f;
	const float kMoveLimitY = 4.0f;
	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -kMoveLimitX, kMoveLimitX);
	worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, -kMoveLimitY, kMoveLimitY);

	// 行列更新
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	// 攻撃処理
	Attack();

	// 弾の更新
	for (PlayerBullet* bullet : bullets_) {
		bullet->Update();
	}
	// ミサイルの更新
	for (HomingArcBullet* bullet : arcBullets_) {
		bullet->Update();
	}
}

void Player::Draw3D() {
	if (invincibleTimer_ % 2 == 0) {
		model_->Draw(worldTransform_, *camera_);
	}

	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw(*camera_);
	}
	for (HomingArcBullet* bullet : arcBullets_) {
		bullet->Draw(*camera_);
	}
}

void Player::Draw2D() {
	// ロックオンマーカー描画
	lolckOn_.DrawMarkers(*camera_);

	// HPアイコンの描画
	for (int i = 0; i < hitPoint_; ++i) {
		if (i >= hpIcons_.size())
			break;

		Vector2 pos = {30.0f + i * 35.0f, 30.0f};

		hpIcons_[i]->SetPosition(pos);
		hpIcons_[i]->Draw();
	}
}

void Player::Attack() {
	// ---------- 右クリック：ロックオン ----------
	if (input_->IsTriggerMouse(1)) { // 1:右クリック
		if (enemies_) {
			lolckOn_.TryLockOn(input_->GetMousePosition(), *enemies_, *camera_);
		}
	}

	// ---------- 左クリック：ロックオンがあればミサイル / なければ直進弾 ----------
	if (input_->IsTriggerMouse(0)) { // 0:左クリック

		std::vector<Enemy*> locked = lolckOn_.GetLockedEnemies();

		// ロックオンしている敵がいればミサイル発射
		if (!locked.empty()) {

			for (Enemy* enemy : locked) {
				// ★まず「今の enemies_ に存在するか」だけを見る（安全対策）
				if (!IsAlivePointerInEnemies(enemy)) {
					continue;
				}

				// ★ここから先は敵が存在すると保証できるので触ってOK
				if (enemy->IsDead()) {
					continue;
				}

				HomingArcBullet* arc = new HomingArcBullet();

				// ランダムなオフセット生成
				Vector3 offset = {(rand() % 200 - 100) / 10.0f, (rand() % 100) / 10.0f + 5.0f, (rand() % 200 - 100) / 10.0f};

				Model* missileModel = playerMissileModel_ ? playerMissileModel_ : model_;
				arc->Initialize(missileModel, worldTransform_.translation_, enemy->GetWorldPosition(), offset);
				arcBullets_.push_back(arc);

				if (seMissile_ != 0) {
					Audio::GetInstance()->PlayWave(seMissile_, false, 0.01f);
				}
			}

			lolckOn_.Clear(); // 撃ったら解除
		}
		// ロックオンしていなければ通常弾発射
		else {
			if (playerBulletModel_) {
				// マウス座標を取得
				Vector2 mousePos = input_->GetMousePosition();

				// aimPlaneZ_ (40.0f) を使って、奥の平面上の座標を計算
				Vector3 targetPos = CalcMouseHitOnZPlane(mousePos, aimPlaneZ_);

				// プレイヤーからターゲットへのベクトル
				Vector3 direction = targetPos - worldTransform_.translation_;
				direction = Normalized(direction);

				Vector3 velocity = direction * 1.0f; // 弾速

				// 弾生成
				PlayerBullet* newBullet = new PlayerBullet();
				newBullet->Initialize(playerBulletModel_, worldTransform_.translation_, velocity);
				bullets_.push_back(newBullet);

				// SE再生
				if (seShot_ != 0) {
					Audio::GetInstance()->PlayWave(seShot_, false, 0.1f);
				}
			}
		}
	}

	// ---------- R：解除 ----------
	if (input_->TriggerKey(DIK_R)) {
		lolckOn_.Clear();
	}
}

// ポインタが有効な敵リストに含まれているか確認
bool Player::IsAlivePointerInEnemies(Enemy* enemy) const {
	if (!enemies_)
		return false;
	for (Enemy* e : *enemies_) {
		if (e == enemy) {
			return true;
		}
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

	// HPスプライトの解放
	for (Sprite* s : hpIcons_) {
		delete s;
	}
	hpIcons_.clear();
}

void Player::SetEnemies(const std::vector<Enemy*>* enemies) { enemies_ = enemies; }

Vector3 Player::CalcMouseHitOnZPlane(const Vector2& mouse, float planeZ) const {

	// ニアクリップ（画面手前）のワールド座標
	Vector3 nearW = UnProjectToWorldSpace(mouse, 0.0f, camera_->matView, camera_->matProjection, WinApp::kWindowWidth, WinApp::kWindowHeight);

	// ファークリップ（画面奥）のワールド座標
	Vector3 farW = UnProjectToWorldSpace(mouse, 1.0f, camera_->matView, camera_->matProjection, WinApp::kWindowWidth, WinApp::kWindowHeight);

	// レイの方向ベクトル
	Vector3 rayDir = farW - nearW;
	rayDir = Normalized(rayDir);

	// レイが平面と平行なら、とりあえず「プレイヤーの正面のそのZ」を返す
	if (std::fabs(rayDir.z) < 1e-6f) {
		Vector3 p = worldTransform_.translation_;
		p.z = planeZ;
		return p;
	}

	// 平面の方程式: z = planeZ
	// レイの式: P = nearW + t * rayDir
	// nearW.z + t * rayDir.z = planeZ
	// t = (planeZ - nearW.z) / rayDir.z

	float t = (planeZ - nearW.z) / rayDir.z;

	// 衝突点計算
	Vector3 hitPos = nearW + rayDir * t;

	return hitPos;
}

void Player::OnHit() {
	if (invincibleTimer_ > 0)
		return;

	hitPoint_--;
	invincibleTimer_ = 60; // 1秒無敵
}

void Player::OnHitByBeam() {
	// ビームなど特定の攻撃を受けたときの処理（今は通常ダメージと同じ）
	OnHit();
}
#pragma once
#include <KamataEngine.h>
#include "MatrixMath.h"

class Beam {
public:
	void Initialize(const Vector3& start, const Vector3& target, float chargePower);
	void Update();
	void Draw(const Camera& camera);
	bool IsDead() const { return isDead_; }

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr; // 細長いCylinderやBoxなど
	float lifeTime_ = 20.0f;
	float time_ = 0.0f;
	bool isDead_ = false;
	float damage_ = 0.0f;
};

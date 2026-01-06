#pragma once
#include "KamataEngine.h"
#include "MatrixMath.h"

using namespace KamataEngine;

class SkyDome {
public:
	// objName: "SkySphere" みたいなOBJ名
	// scale : 天球の大きさ（でかいほど良い）
	void Initialize(const char* objName, float scale = 200.0f);

	// カメラ位置に追従させる（これが天球の肝）
	void Update(const Camera& camera);

	// 描画
	void Draw(const Camera& camera);

	// 後始末（KamataEngineのModelがdelete必要なら）
	void Finalize();

	// 使わないなら消してOK
	void SetScale(float s) { scale_ = s; }

private:
	Model* model_ = nullptr;
	WorldTransform wt_{};
	float scale_ = 200.0f;
};

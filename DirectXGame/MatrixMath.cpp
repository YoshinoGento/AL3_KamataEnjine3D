#include "MatrixMath.h"
#include <cmath>
#include <numbers>

// 02_14 29枚目 単項演算子オーバーロード
Vector3 operator+(const Vector3& v) { return v; }

Vector3 operator-(const Vector3& v) { return Vector3(-v.x, -v.y, -v.z); }


const Vector3 operator-(const Vector3& lhv, const Vector3& rhv) { 
	 return {lhv.x - rhv.x, lhv.y - rhv.y, lhv.z - rhv.z};
}

// 02_06の29枚目(CameraControllerのUpdate)で必要
const Vector3 operator*(const Vector3& v1, const float f) {

	Vector3 temp(v1);
	return temp *= f;
}

// Vector3 + Vector3
const Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	Vector3 temp(v1);
	return temp += v2;
}

// Vector3 - Vector3
//const Vector3 operator-(const Vector3& v1, const Vector3& v2) {
//	Vector3 temp(v1);
//	return temp -= v2;
//}

// 02_06のスライド24枚目のLerp関数
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) { return Vector3(Lerp(v1.x, v2.x, t), Lerp(v1.y, v2.y, t), Lerp(v1.z, v2.z, t)); }

// Vector3 / float
const Vector3 operator/(const Vector3& v, float s) {
	Vector3 temp(v);
	return temp /= s;
}

Vector3& operator+=(Vector3& lhv, const Vector3& rhv) {
	lhv.x += rhv.x;
	lhv.y += rhv.y;
	lhv.z += rhv.z;
	return lhv;
}

Vector3& operator-=(Vector3& lhv, const Vector3& rhv) {
	lhv.x -= rhv.x;
	lhv.y -= rhv.y;
	lhv.z -= rhv.z;
	return lhv;
}

Vector3& operator*=(Vector3& v, float s) {
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

Vector3& operator/=(Vector3& v, float s) {
	v.x /= s;
	v.y /= s;
	v.z /= s;
	return v;
}

Matrix4x4 MakeIdentityMatrix() {
	static const Matrix4x4 result{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	return result;
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale) {

	Matrix4x4 result{scale.x, 0.0f, 0.0f, 0.0f, 0.0f, scale.y, 0.0f, 0.0f, 0.0f, 0.0f, scale.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	return result;
}

Matrix4x4 MakeRotateXMatrix(float theta) {
	float sin = std::sin(theta);
	float cos = std::cos(theta);

	Matrix4x4 result{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, cos, sin, 0.0f, 0.0f, -sin, cos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	return result;
}

Matrix4x4 MakeRotateYMatrix(float theta) {
	float sin = std::sin(theta);
	float cos = std::cos(theta);

	Matrix4x4 result{cos, 0.0f, -sin, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, sin, 0.0f, cos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	return result;
}

Matrix4x4 MakeRotateZMatrix(float theta) {
	float sin = std::sin(theta);
	float cos = std::cos(theta);

	Matrix4x4 result{cos, sin, 0.0f, 0.0f, -sin, cos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, translate.x, translate.y, translate.z, 1.0f};

	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rot, const Vector3& translate) {

	// スケーリング行列の作成
	Matrix4x4 matScale = MakeScaleMatrix(scale);

	Matrix4x4 matRotX = MakeRotateXMatrix(rot.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rot.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rot.z);
	// 回転行列の合成
	Matrix4x4 matRot = matRotZ * matRotX * matRotY;

	// 平行移動行列の作成
	Matrix4x4 matTrans = MakeTranslateMatrix(translate);

	// スケーリング、回転、平行移動の合成
	Matrix4x4 matTransform = matScale * matRot * matTrans;

	return matTransform;
}

Matrix4x4& operator*=(Matrix4x4& lhm, const Matrix4x4& rhm) {
	Matrix4x4 result{};

	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			for (size_t k = 0; k < 4; k++) {
				result.m[i][j] += lhm.m[i][k] * rhm.m[k][j];
			}
		}
	}
	lhm = result;
	return lhm;
}

Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = m1;
	return result *= m2;
}

// ワールドトランスフォーム更新(02_03の最後)
void WorldTransformUpdate(WorldTransform& worldTransform) {

	Matrix4x4 affin_mat = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);

	worldTransform.matWorld_ = affin_mat;

	// 定数バッファに転送する
	worldTransform.TransferMatrix();
}

float Lerp(float x1, float x2, float t) { return (1.0f - t) * x1 + t * x2; }

float EaseIn(float x1, float x2, float t) {
	float easedT = t * t;
	return Lerp(x1, x2, easedT);
}

float EaseOut(float x1, float x2, float t) {
	float easedT = 1.0f - std::powf(1.0f - t, 3.0f);
	return Lerp(x1, x2, easedT);
}

float EaseInOut(float x1, float x2, float t) {
	float easedT = -(std::cosf(std::numbers::pi_v<float> * t) - 1.0f) / 2.0f;
	return Lerp(x1, x2, easedT);
}

bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	return (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && // x軸
	       (aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && // y軸
	       (aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);   // z軸
}

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;

	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];

	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;

}

float Length(const Vector3& v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

Vector3 Normalized(const Vector3& v) {
	float len = Length(v);
	if (len == 0.0f) {
		return {0.0f, 0.0f, 0.0f};
	}
	return {v.x / len, v.y / len, v.z / len};
}

Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m) {

	Vector3 result{v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0], v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1], v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2]};

	return result;
}

Vector3 UnProjectToWorldSpace(const Vector2& screenPos, float z, const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, int screenWidth, int screenHeight) {
	// 1. スクリーン座標 → 正規化デバイス座標（NDC: -1〜+1）
	float ndcX = (screenPos.x / static_cast<float>(screenWidth)) * 2.0f - 1.0f;
	float ndcY = 1.0f - (screenPos.y / static_cast<float>(screenHeight)) * 2.0f;

	// 2. NDCを4次元ベクトルに変換（z は仮で1.0fなど）
	Vector4 ndc = {ndcX, ndcY, z, 1.0f};

	// 3. ビュー × プロジェクション の逆行列を計算
	Matrix4x4 invViewProj = Inverse(viewMatrix * projMatrix);

	// 4. 逆変換
	Vector4 world = Transform(ndc, invViewProj);

	// 5. 同次座標を通常の座標に戻す（w除算）
	if (world.w != 0.0f) {
		world.x /= world.w;
		world.y /= world.w;
		world.z /= world.w;
	}

	return {world.x, world.y, world.z};
}

#include "MatrixMath.h"
#include <cassert>

// 逆行列計算（シンプルなガウスジョルダン消去法の実装）
Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 result = MakeIdentityMatrix();
	Matrix4x4 copy = m;

	for (int i = 0; i < 4; ++i) {
		float pivot = copy.m[i][i];
		assert(pivot != 0.0f); // ゼロ除算防止

		float invPivot = 1.0f / pivot;
		for (int j = 0; j < 4; ++j) {
			copy.m[i][j] *= invPivot;
			result.m[i][j] *= invPivot;
		}

		for (int k = 0; k < 4; ++k) {
			if (k != i) {
				float factor = copy.m[k][i];
				for (int j = 0; j < 4; ++j) {
					copy.m[k][j] -= copy.m[i][j] * factor;
					result.m[k][j] -= result.m[i][j] * factor;
				}
			}
		}
	}

	return result;
}

// Vector4 × Matrix4x4
Vector4 Transform(const Vector4& v, const Matrix4x4& m) {
	Vector4 result;
	result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + v.w * m.m[3][0];
	result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + v.w * m.m[3][1];
	result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + v.w * m.m[3][2];
	result.w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + v.w * m.m[3][3];
	return result;
}

Vector2 ProjectToScreen(const Vector3& worldPos, const Matrix4x4& view, const Matrix4x4& proj, int width, int height) {
	Matrix4x4 vp = view * proj;
	Vector3 clip = Transform(worldPos, vp);

	if (clip.z == 0.0f)
		clip.z = 0.0001f; // 除算防止

	float x = (clip.x / clip.z) * 0.5f + 0.5f;
	float y = (-clip.y / clip.z) * 0.5f + 0.5f;

	return {x * width, y * height};
}

#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

// 円周率
const float PI = 3.141592654f;

struct AABB {
	Vector3 min;
	Vector3 max;
};

// 02_14 29枚目 単項演算子オーバーロード
Vector3 operator+(const Vector3& v);
Vector3 operator-(const Vector3& v);

// Vector3 * float
const Vector3 operator*(const Vector3& v1, float f);

const Vector3 operator*(const float f, const Vector3& v1);

// Vector3 + Vector3
const Vector3 operator+(const Vector3& v1, const Vector3& v2);

// Vector3 - Vector3
const Vector3 operator-(const Vector3& v1, const Vector3& v2);

// 02_06のスライド24枚目のLerp関数
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

// Vector3 / float
const Vector3 operator/(const Vector3& v, float s);

// 代入演算子オーバーロード
Vector3& operator+=(Vector3& lhs, const Vector3& rhv);
Vector3& operator-=(Vector3& lhs, const Vector3& rhv);
Vector3& operator*=(Vector3& v, float s);
Vector3& operator/=(Vector3& v, float s);

// 単位行列の作成
Matrix4x4 MakeIdentityMatrix();
// スケーリング行列の作成
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
// 回転行列の作成
Matrix4x4 MakeRotateXMatrix(float theta);
Matrix4x4 MakeRotateYMatrix(float theta);
Matrix4x4 MakeRotateZMatrix(float theta);
// 平行移動行列の作成
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
// アフィン変換行列の作成
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rot, const Vector3& translate);

// 代入演算子オーバーロード
Matrix4x4& operator*=(Matrix4x4& lhm, const Matrix4x4& rhm);

// 2項演算子オーバーロード
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);

// ワールドトランスフォーム更新(02_03の最後)
void WorldTransformUpdate(WorldTransform& worldTransform);

float Lerp(float x1, float x2, float t);
float EaseIn(float x1, float x2, float t);
float EaseOut(float x1, float x2, float t);
float EaseInOut(float x1, float x2, float t);

bool IsCollision(const AABB& aabb1, const AABB& aabb2);

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

// 02_15 で追加
inline float ToRadians(float degrees) { return degrees * (3.1415f / 180.0f); }
inline float ToDegrees(float radians) { return radians * (180.0f / 3.1415f); }

// ベクトルの長さを求める
float Length(const Vector3& v);

// ベクトルを正規化する（方向だけにする）
Vector3 Normalized(const Vector3& v);

// ベクトル変換（法線用）
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

// 画面座標→ワールド（マウスレイなど）
Vector3 UnProjectToWorldSpace(const Vector2& screenPos, float z, const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, int screenWidth, int screenHeight);

// 4x4行列の逆行列を求める
Matrix4x4 Inverse(const Matrix4x4& m);

// 4Dベクトル × 4x4行列の変換
Vector4 Transform(const Vector4& v, const Matrix4x4& m);

// ワールド→スクリーン
Vector2 ProjectToScreen(const Vector3& worldPos, const Matrix4x4& view, const Matrix4x4& proj, int width, int height);

// 球面線形補間
Vector3 Slerp(const Vector3& v1, const Vector3& v2, float t);

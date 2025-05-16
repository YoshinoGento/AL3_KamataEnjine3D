#pragma once  
#include "KamataEngine.h"  

/// AL3サンプルプログラム用の数学ライブラリ。  
/// MT3準拠で、KamataEngine内部の数学ライブラリと重複する。  
/*  
struct Matrix4x4 final {  
  float m[4][4];  
};  

struct Vector4 final {  
  float x;  
  float y;  
  float z;  
  float w;  
};  

struct Vector3 final {  
  float x;  
  float y;  
  float z;  
};  

struct Vector2 final {  
  float x;  
  float y;  
};  
*/  
namespace KamataEngine {  
  struct Vector3;  
  struct Matrix4x4;  
}  

class Math {  
public:  
  // 修正: `using namespace` をクラススコープ内で使用するのは非推奨のため、代わりにスコープ解決演算子を使用します。
  // 円周率  
  const float PI = 3.141592654f;  

  // 代入演算子オーバーロード  
 /* KamataEngine::Vector3& operator+=(KamataEngine::Vector3& lhs, const KamataEngine::Vector3& rhv);  
  KamataEngine::Vector3& operator-=(KamataEngine::Vector3& lhs, const KamataEngine::Vector3& rhv);  
  KamataEngine::Vector3& operator*=(KamataEngine::Vector3& v, float s);  
  KamataEngine::Vector3& operator/=(KamataEngine::Vector3& v, float s);  */

  // 単位行列の作成  
  KamataEngine::Matrix4x4 MakeIdentityMatrix();  
  // スケーリング行列の作成  
  KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale);  
  // 回転行列の作成  
  KamataEngine::Matrix4x4 MakeRotateXMatrix(float theta);  
  KamataEngine::Matrix4x4 MakeRotateYMatrix(float theta);  
  KamataEngine::Matrix4x4 MakeRotateZMatrix(float theta);  
  // 平行移動行列の作成  
  KamataEngine::Matrix4x4 MakeTranslateMatrix(const KamataEngine::Vector3& translate);  
  // アフィン変換行列の作成  
  KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rot, const KamataEngine::Vector3& translate);  

  //// 代入演算子オーバーロード  
  //KamataEngine::Matrix4x4& operator*=(KamataEngine::Matrix4x4& lhm, const KamataEngine::Matrix4x4& rhm);  

  //// 2項演算子オーバーロード  
  //KamataEngine::Matrix4x4 operator*(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);  
};
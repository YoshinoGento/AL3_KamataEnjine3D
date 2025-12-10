#pragma once

class Scene {
public:
	virtual ~Scene() = default; // ← 必須（安全のため）

	virtual void Initialize() = 0;
	virtual void Update() = 0;

	// 描画を2つに分ける（3D と 2D）
	virtual void Draw3D() = 0;
	virtual void Draw2D() = 0;

	virtual void Finalize() = 0;

	virtual bool IsEnd() = 0;
	virtual int NextScene() = 0;
};

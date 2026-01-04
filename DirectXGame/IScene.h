#pragma once

class GameManager;

class IScene {
public:
	virtual ~IScene() = default;
	virtual void Initialize(GameManager* manager) = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Finalize() = 0;
};

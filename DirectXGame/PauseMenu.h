#pragma once
#include "KamataEngine.h"
using namespace KamataEngine;

class PauseMenu {
public:
	enum class Result { None, Resume, Restart, ToTitle };

	void Initialize(Camera* camera); // ★追加
	void Open() {
		isOpen_ = true;
		cursor_ = 0;
	}
	void Close() { isOpen_ = false; }
	bool IsOpen() const { return isOpen_; }

	Result Update();
	void Draw(); // OBJで描く

private:
	bool isOpen_ = false;
	int cursor_ = 0; // 0:Resume 1:Restart 2:Title

	// ===== OBJ UI =====
	Camera* camera_ = nullptr;

	Model* quadModel_ = nullptr;

	WorldTransform wtTitle_{};
	WorldTransform wtResume_{};
	WorldTransform wtRestart_{};
	WorldTransform wtToTitle_{};

	uint32_t texTitle_ = 0;
	uint32_t texResume_ = 0;
	uint32_t texResumeSel_ = 0;
	uint32_t texRestart_ = 0;
	uint32_t texRestartSel_ = 0;
	uint32_t texToTitle_ = 0;
	uint32_t texToTitleSel_ = 0;

private:
	void SetupTransform_();
};

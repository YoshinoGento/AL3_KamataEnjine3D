#include "GameScene.h"
#include "KamataEngine.h"
#include "SceneManager.h"
#include <Windows.h>

using namespace KamataEngine;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	KamataEngine::Initialize(L"第3次ファンネル大戦～母さん、俺もうダメかも～");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// SceneManager を正しく初期化
	SceneManager sceneManager;
	sceneManager.Initialize(); // ★ 必須！

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		sceneManager.Update();

		dxCommon->PreDraw();
		sceneManager.Draw();
		dxCommon->PostDraw();

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			break;
		}
	}

	sceneManager.Finalize();
	KamataEngine::Finalize();
	return 0;
}

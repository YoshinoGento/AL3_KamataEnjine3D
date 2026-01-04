#include "GameManager.h"
#include "KamataEngine.h"
#include <Windows.h>

using namespace KamataEngine;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	KamataEngine::Initialize();

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	GameManager gameManager;
	gameManager.Initialize();

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		dxCommon->PreDraw();

		gameManager.Update();
		gameManager.Draw();

		dxCommon->PostDraw();
	}

	gameManager.Finalize();
	KamataEngine::Finalize();

	return 0;
}

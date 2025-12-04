#include "GameScene.h"
#include "KamataEngine.h"
#include <Windows.h>

using namespace KamataEngine;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	KamataEngine::Initialize();
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	GameScene* gameScene = new GameScene();
	gameScene->Initialize();


	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		dxCommon->PreDraw();
		gameScene->Update();
		gameScene->Draw();
		dxCommon->PostDraw();
	}

	gameScene->Delete();
	delete gameScene;
	KamataEngine::Finalize();
	return 0;
}

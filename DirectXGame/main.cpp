#include "GameScene.h"
#include "KamataEngine.h"
#include <Windows.h>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"褪せロボ");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	GameScene* gameScene = new GameScene();
	gameScene->Initialize();


	while (true) {
		if (KamataEngine::Update()) {
			break;
		}



		// 描画開始
		dxCommon->PreDraw();
		gameScene->Update();
		gameScene->Draw();
		dxCommon->PostDraw();

		 // ★ ESCキーで終了（追加）
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			break;
		}
	}

	gameScene->Delete();
	delete gameScene;
	KamataEngine::Finalize();
	return 0;
}

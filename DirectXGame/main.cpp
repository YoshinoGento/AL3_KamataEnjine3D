#include "GameScene.h"
#include "KamataEngine.h"
#include "SceneManager.h"
#include <Windows.h>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"第3次ファンネル大戦～母さん、俺もうダメかも～");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	// ★ SceneManager を使う
	SceneManager sceneManager;
	sceneManager.ChangeScene((int)SceneType::TITLE); // ← 最初のシーン


	while (true) {
		if (KamataEngine::Update()) {
			break;
		}
		//gameScene->Update();

		sceneManager.Update();


		// 描画開始
		dxCommon->PreDraw();
		sceneManager.Draw();
		dxCommon->PostDraw();

		 // ★ ESCキーで終了（追加）
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			break;
		}
	}

	sceneManager.Finalize();
	KamataEngine::Finalize();
	return 0;
}

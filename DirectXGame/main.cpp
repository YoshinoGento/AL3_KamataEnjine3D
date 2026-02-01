#include "GameManager.h"
#include "KamataEngine.h"
#include <Windows.h>

using namespace KamataEngine;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	KamataEngine::Initialize(L"LE2C_28_ヨシノ_ゲント_ミサイルヒッター");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	GameManager gameManager;
	gameManager.Initialize();

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		// ★最重要：Updateは必ず PreDraw の「前」に行う！
		// ※もし dxCommon->PreDraw(); の「後」にも gameManager.Update(); が書いてあったら、
		//   それは消してください！（2回呼ぶとエラーになります）
		gameManager.Update();

		dxCommon->PreDraw();

		// 描画処理
		gameManager.Draw();

		dxCommon->PostDraw();
	}

	gameManager.Finalize();
	KamataEngine::Finalize();

	return 0;
}
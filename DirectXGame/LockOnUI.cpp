#include "LockOnUI.h"
#include <imgui.h>
#include "MatrixMath.h" // ProjectToScreen() を使うため

void LockOnUI::Draw(const Enemy* enemy, const Camera* camera, int screenWidth, int screenHeight) {
	if (!enemy || !camera)
		return;

	// 敵のワールド座標 → スクリーン座標へ
	Vector3 enemyWorldPos = enemy->GetWorldPosition();
	Vector2 screenPos = ProjectToScreen(enemyWorldPos, camera->matView, camera->matProjection, screenWidth, screenHeight);

	// 表示位置調整（少し上にずらす）
	screenPos.y -= 30;

	// ImGuiで描画
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::SetNextWindowPos(ImVec2(screenPos.x, screenPos.y), ImGuiCond_Always);
	ImGui::Begin(
	    "LockOn", nullptr,
	    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
	        ImGuiWindowFlags_NoBackground);
	ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "LOCK ON");
	ImGui::End();
}

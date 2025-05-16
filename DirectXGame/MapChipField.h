#pragma once
#include "KamataEngine.h"
#include "Math.h"
#include <cstdint>
#include <string>
#include <vector>

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

/// <summary>
/// マップチップフィールド
/// </summary>
class MapChipField {

public:
	MapChipData mapChipData_;

	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	// マップチップフィールド
	MapChipField* mapChipField_;

	
	//カプセル化
	// 縦方向のブロック数を取得
	uint32_t GetNumBlockVirtical() const { return kNumBlockVertical; }

	// 横方向のブロック数を取得
	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }

private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	static inline const uint32_t kNumBlockVertical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;
};

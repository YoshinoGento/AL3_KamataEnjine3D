#pragma once
#include <cstdint>
#include <string>
#include "KamataEngine.h"
#include <vector>
#include "Math.h"


enum class MapChipType {
	kBlank,//空白
	kBlock,//ブロック
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


private:

	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	static inline const uint32_t kNumBlockVertical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	//マップチップフィールド
	MapChipField* mapChipField_;
};

#include "GalleryLayout.h"

void GalleryLayout::applyGrid(std::vector<std::shared_ptr<MediaItem>> & items) const {
	if (items.empty()) {
		return;
	}

	const int columns = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(items.size()))));
	const int rows = static_cast<int>(std::ceil(static_cast<float>(items.size()) / columns));
	const float startX = -0.5f * spacingX * static_cast<float>(columns - 1);
	const float startY = -0.5f * spacingY * static_cast<float>(rows - 1);

	for (std::size_t i = 0; i < items.size(); ++i) {
		const int column = static_cast<int>(i) % columns;
		const int row = static_cast<int>(i) / columns;
		const float x = startX + column * spacingX;
		const float y = startY + row * spacingY;
		const float z = ofMap(items[i]->getMetadata().meanLuminance, 0.0f, 1.0f, -maxDepthOffset, maxDepthOffset, true);
		items[i]->setClusterIndex(-1, 0);
		items[i]->setTargetPosition(glm::vec3(x, y, z));
	}
}

void GalleryLayout::applyClusterIslands(std::vector<std::shared_ptr<MediaItem>> & items, const std::vector<int> & assignments, int clusterCount) const {
	if (items.empty() || assignments.size() != items.size() || clusterCount <= 0) {
		applyGrid(items);
		return;
	}

	std::vector<std::vector<std::shared_ptr<MediaItem>>> clusters(clusterCount);
	for (std::size_t i = 0; i < items.size(); ++i) {
		const int clusterIndex = ofClamp(assignments[i], 0, clusterCount - 1);
		clusters[clusterIndex].push_back(items[i]);
		items[i]->setClusterIndex(clusterIndex, clusterCount);
	}

	const int islandColumns = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(clusterCount))));
	const int islandRows = static_cast<int>(std::ceil(static_cast<float>(clusterCount) / islandColumns));
	const float islandStartX = -0.5f * islandSpacingX * static_cast<float>(islandColumns - 1);
	const float islandStartY = -0.5f * islandSpacingY * static_cast<float>(islandRows - 1);

	for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
		const int islandColumn = clusterIndex % islandColumns;
		const int islandRow = clusterIndex / islandColumns;
		const glm::vec3 islandCenter(
			islandStartX + islandColumn * islandSpacingX,
			islandStartY + islandRow * islandSpacingY,
			static_cast<float>(clusterIndex) * 22.0f);

		auto & clusterItems = clusters[clusterIndex];
		const int rowLength = std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<float>(clusterItems.size())))));
		const int rows = std::max(1, static_cast<int>(std::ceil(static_cast<float>(clusterItems.size()) / rowLength)));
		const float localStartX = -0.5f * spacingX * static_cast<float>(rowLength - 1);
		const float localStartY = -0.5f * spacingY * static_cast<float>(rows - 1);

		for (std::size_t itemIndex = 0; itemIndex < clusterItems.size(); ++itemIndex) {
			const int column = static_cast<int>(itemIndex) % rowLength;
			const int row = static_cast<int>(itemIndex) / rowLength;
			const float localX = localStartX + column * spacingX;
			const float localY = localStartY + row * spacingY;
			const float localZ = ofMap(clusterItems[itemIndex]->getMetadata().meanLuminance, 0.0f, 1.0f, -maxDepthOffset, maxDepthOffset, true);
			clusterItems[itemIndex]->setTargetPosition(islandCenter + glm::vec3(localX, localY, localZ));
		}
	}
}

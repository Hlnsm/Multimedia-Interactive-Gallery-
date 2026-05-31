#pragma once

#include "MediaItem.h"
#include "ofMain.h"

class GalleryLayout {
public:
	void applyGrid(std::vector<std::shared_ptr<MediaItem>> & items) const;
	void applyClusterIslands(std::vector<std::shared_ptr<MediaItem>> & items, const std::vector<int> & assignments, int clusterCount) const;

private:
	float spacingX = 390.0f;
	float spacingY = 290.0f;
	float maxDepthOffset = 90.0f;
	float islandSpacingX = 1180.0f;
	float islandSpacingY = 780.0f;
};

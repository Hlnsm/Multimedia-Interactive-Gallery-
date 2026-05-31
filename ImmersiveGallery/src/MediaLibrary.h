#pragma once

#include "ClusterManager.h"
#include "GalleryLayout.h"
#include "MetadataManager.h"
#include "ofMain.h"

class MediaLibrary {
public:
	void setup(const std::string & mediaDirectory, MetadataManager & metadataManager);
	void reload();
	void update();
	void draw(bool showMetadata);
	void updateHover(const glm::vec2 & mousePosition, const ofCamera & camera);
	bool selectHovered();
	void clearSelection();
	bool hasSelection() const;
	void drawSelectedFocus(const ofRectangle & bounds);
	void selectNext(int direction);
	void toggleVideoPlayback();
	void toggleSelectedPlayback();
	void setFeatureMotionEnabled(bool enabled);
	void toggleFeatureMotion();
	void applyClusters(int requestedClusterCount);
	void resetGrid();
	std::size_t size() const;
	bool empty() const;
	int getActiveClusterCount() const;
	bool isFeatureMotionEnabled() const;
	std::string getSelectedInfo() const;

private:
	std::string mediaDir = "media";
	MetadataManager * metadata = nullptr;
	ClusterManager clusterManager;
	GalleryLayout layout;
	std::vector<std::shared_ptr<MediaItem>> items;
	std::vector<int> clusterAssignments;
	int activeClusterCount = 0;
	int hoveredIndex = -1;
	int selectedIndex = -1;
	bool featureMotionEnabled = true;

	bool isSupportedMedia(const ofFile & file) const;
	std::string detectType(const ofFile & file) const;
	int getNextIndexInCurrentScope(int direction) const;
};

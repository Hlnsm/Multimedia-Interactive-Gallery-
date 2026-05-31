#pragma once

#include "MediaMetadata.h"
#include "ofMain.h"

class MediaItem {
public:
	bool load(const ofFile & file, const std::string & detectedType);
	void setMetadata(const MediaMetadata & newMetadata);
	void setPosition(const glm::vec3 & newPosition);
	void setTargetPosition(const glm::vec3 & newTargetPosition);
	void setClusterIndex(int newClusterIndex, int newClusterCount = 0);
	void setHovered(bool newHovered);
	void setFeatureMotionEnabled(bool enabled);
	void update();
	void draw(bool showMetadata);
	void drawFocus(const ofRectangle & bounds);
	void togglePlayback();
	void setPaused(bool paused);

	bool isVideo() const;
	const std::string & getFileName() const;
	const MediaMetadata & getMetadata() const;
	const ofPixels * getPixelsForMetadata() const;
	const glm::vec3 & getPosition() const;
	int getClusterIndex() const;
	bool hitTestScreen(const glm::vec2 & screenPoint, const ofCamera & camera) const;
	std::string getFocusInfo() const;

private:
	std::string fileName;
	std::string filePath;
	std::string type = "image";
	MediaMetadata metadata;
	ofImage image;
	ofVideoPlayer video;
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 targetPosition = glm::vec3(0.0f);
	float drawWidth = 260.0f;
	float drawHeight = 170.0f;
	bool videoPaused = false;
	bool featureMotionEnabled = true;
	int clusterIndex = -1;
	int clusterCount = 0;
	bool hovered = false;
	float hoverAmount = 0.0f;

	void updateDrawSize();
	float getSourceWidth() const;
	float getSourceHeight() const;
	glm::vec3 getFeatureMotionOffset() const;
	ofRectangle getScreenBounds(const ofCamera & camera) const;
	ofColor getAccentColor() const;
	std::string getMetadataLabel() const;
	std::string getShortFileName() const;
};

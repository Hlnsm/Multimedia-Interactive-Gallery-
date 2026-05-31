#include "MediaItem.h"

#include <cmath>

bool MediaItem::load(const ofFile & file, const std::string & detectedType) {
	fileName = file.getFileName();
	filePath = file.getAbsolutePath();
	type = detectedType;

	bool loaded = false;
	if (isVideo()) {
		loaded = video.load(filePath);
		if (loaded) {
			video.setLoopState(OF_LOOP_NORMAL);
			video.play();
			video.setPaused(false);
		}
	} else {
		loaded = image.load(filePath);
	}

	updateDrawSize();
	return loaded;
}

void MediaItem::setMetadata(const MediaMetadata & newMetadata) {
	metadata = newMetadata;
	updateDrawSize();
}

void MediaItem::setPosition(const glm::vec3 & newPosition) {
	position = newPosition;
	targetPosition = newPosition;
}

void MediaItem::setTargetPosition(const glm::vec3 & newTargetPosition) {
	targetPosition = newTargetPosition;
}

void MediaItem::setClusterIndex(int newClusterIndex, int newClusterCount) {
	clusterIndex = newClusterIndex;
	clusterCount = newClusterCount;
}

void MediaItem::setHovered(bool newHovered) {
	hovered = newHovered;
}

void MediaItem::setFeatureMotionEnabled(bool enabled) {
	featureMotionEnabled = enabled;
}

void MediaItem::update() {
	const float follow = 1.0f - std::pow(0.0005f, static_cast<float>(ofGetLastFrameTime()));
	position = glm::mix(position, targetPosition, ofClamp(follow, 0.0f, 1.0f));
	const float hoverFollow = 1.0f - std::pow(0.00005f, static_cast<float>(ofGetLastFrameTime()));
	hoverAmount = ofLerp(hoverAmount, hovered ? 1.0f : 0.0f, ofClamp(hoverFollow, 0.0f, 1.0f));

	if (isVideo()) {
		video.update();
	}
}

void MediaItem::draw(bool showMetadata) {
	const glm::vec3 featureOffset = getFeatureMotionOffset();
	const glm::vec3 drawPosition = position + featureOffset;

	ofPushMatrix();
	ofTranslate(drawPosition.x, drawPosition.y, drawPosition.z);
	ofTranslate(0.0f, -42.0f * hoverAmount, 42.0f * hoverAmount);
	const float hoverScale = 1.0f + 0.14f * hoverAmount;
	ofScale(hoverScale, hoverScale, 1.0f);
	ofDisableDepthTest();

	const float framePad = 18.0f;
	const ofColor accent = getAccentColor();

	ofPushStyle();
	ofFill();
	ofSetColor(8, 10, 14);
	ofPushMatrix();
	ofTranslate(0.0f, 0.0f, -3.0f);
	ofDrawRectangle(-drawWidth * 0.5f - framePad, -drawHeight * 0.5f - framePad, drawWidth + framePad * 2.0f, drawHeight + framePad * 2.0f);
	ofPopMatrix();

	ofNoFill();
	ofSetLineWidth(3.0f + 3.0f * hoverAmount);
	ofColor hoverColor = accent;
	hoverColor.lerp(ofColor(255), hoverAmount * 0.45f);
	ofSetColor(hoverColor);
	ofPushMatrix();
	ofTranslate(0.0f, 0.0f, -1.0f);
	ofDrawRectangle(-drawWidth * 0.5f - framePad, -drawHeight * 0.5f - framePad, drawWidth + framePad * 2.0f, drawHeight + framePad * 2.0f);
	ofPopMatrix();

	if (hoverAmount > 0.01f) {
		ofSetColor(255, static_cast<int>(95 * hoverAmount));
		ofSetLineWidth(1.0f);
		ofDrawRectangle(-drawWidth * 0.5f - framePad - 8.0f, -drawHeight * 0.5f - framePad - 8.0f, drawWidth + framePad * 2.0f + 16.0f, drawHeight + framePad * 2.0f + 16.0f);
	}

	ofFill();
	ofSetColor(255);
	ofPushMatrix();
	ofTranslate(0.0f, 0.0f, 1.0f);
	if (isVideo()) {
		video.draw(-drawWidth * 0.5f, -drawHeight * 0.5f, drawWidth, drawHeight);
		if (videoPaused) {
			ofSetColor(0, 150);
			ofDrawRectangle(-drawWidth * 0.5f, -drawHeight * 0.5f, drawWidth, drawHeight);
			ofSetColor(255);
			ofDrawBitmapString("pausado", -30.0f, 5.0f);
		}
	} else {
		image.draw(-drawWidth * 0.5f, -drawHeight * 0.5f, drawWidth, drawHeight);
	}
	ofPopMatrix();

	ofSetColor(255);
	ofDrawBitmapString(getShortFileName(), -drawWidth * 0.5f, drawHeight * 0.5f + 34.0f);

	if (showMetadata) {
		ofSetColor(220);
		ofDrawBitmapString(getMetadataLabel(), -drawWidth * 0.5f, drawHeight * 0.5f + 52.0f);
	}
	ofPopStyle();

	ofEnableDepthTest();
	ofPopMatrix();
}

void MediaItem::drawFocus(const ofRectangle & bounds) {
	const float sourceWidth = getSourceWidth();
	const float sourceHeight = getSourceHeight();
	const float aspect = sourceWidth / std::max(1.0f, sourceHeight);

	ofRectangle mediaRect = bounds;
	if (mediaRect.getWidth() / mediaRect.getHeight() > aspect) {
		const float width = mediaRect.getHeight() * aspect;
		mediaRect.x += (mediaRect.getWidth() - width) * 0.5f;
		mediaRect.width = width;
	} else {
		const float height = mediaRect.getWidth() / aspect;
		mediaRect.y += (mediaRect.getHeight() - height) * 0.5f;
		mediaRect.height = height;
	}

	ofPushStyle();
	ofDisableDepthTest();
	ofFill();
	ofSetColor(8, 10, 14);
	ofDrawRectangle(mediaRect.x - 18.0f, mediaRect.y - 18.0f, mediaRect.width + 36.0f, mediaRect.height + 36.0f);
	ofNoFill();
	ofSetLineWidth(3.0f);
	ofSetColor(getAccentColor());
	ofDrawRectangle(mediaRect.x - 18.0f, mediaRect.y - 18.0f, mediaRect.width + 36.0f, mediaRect.height + 36.0f);

	ofFill();
	ofSetColor(255);
	if (isVideo()) {
		video.draw(mediaRect);
		if (videoPaused) {
			ofSetColor(0, 150);
			ofDrawRectangle(mediaRect);
			ofSetColor(255);
			ofDrawBitmapString("pausa", mediaRect.getCenter().x - 30.0f, mediaRect.getCenter().y);
		}
	} else {
		image.draw(mediaRect);
	}
	ofEnableDepthTest();
	ofPopStyle();
}

void MediaItem::togglePlayback() {
	if (!isVideo()) {
		return;
	}

	videoPaused = !videoPaused;
	video.setPaused(videoPaused);
}

void MediaItem::setPaused(bool paused) {
	if (!isVideo()) {
		return;
	}

	videoPaused = paused;
	video.setPaused(videoPaused);
}

bool MediaItem::isVideo() const {
	return type == "video";
}

const std::string & MediaItem::getFileName() const {
	return fileName;
}

const MediaMetadata & MediaItem::getMetadata() const {
	return metadata;
}

const ofPixels * MediaItem::getPixelsForMetadata() const {
	if (isVideo() || !image.isAllocated()) {
		return nullptr;
	}
	return &image.getPixels();
}

const glm::vec3 & MediaItem::getPosition() const {
	return position;
}

int MediaItem::getClusterIndex() const {
	return clusterIndex;
}

bool MediaItem::hitTestScreen(const glm::vec2 & screenPoint, const ofCamera & camera) const {
	return getScreenBounds(camera).inside(screenPoint.x, screenPoint.y);
}

std::string MediaItem::getFocusInfo() const {
	std::stringstream ss;
	ss << fileName << "\n" << getMetadataLabel();
	if (isVideo()) {
		ss << "\nEspaco: play/pause";
	}
	return ss.str();
}

void MediaItem::updateDrawSize() {
	float sourceWidth = 16.0f;
	float sourceHeight = 9.0f;

	if (isVideo()) {
		if (video.getWidth() > 0.0f && video.getHeight() > 0.0f) {
			sourceWidth = video.getWidth();
			sourceHeight = video.getHeight();
		}
	} else if (image.isAllocated()) {
		sourceWidth = image.getWidth();
		sourceHeight = image.getHeight();
	}

	const float maxWidth = 285.0f;
	const float maxHeight = 190.0f;
	const float aspect = sourceWidth / std::max(1.0f, sourceHeight);
	drawWidth = maxWidth;
	drawHeight = drawWidth / aspect;
	if (drawHeight > maxHeight) {
		drawHeight = maxHeight;
		drawWidth = drawHeight * aspect;
	}

	const float varianceScale = ofMap(metadata.luminanceVariance, 0.0f, 0.12f, 0.92f, 1.12f, true);
	drawWidth *= varianceScale;
	drawHeight *= varianceScale;
}

float MediaItem::getSourceWidth() const {
	if (isVideo() && video.getWidth() > 0.0f) {
		return video.getWidth();
	}
	if (!isVideo() && image.isAllocated()) {
		return image.getWidth();
	}
	return 16.0f;
}

float MediaItem::getSourceHeight() const {
	if (isVideo() && video.getHeight() > 0.0f) {
		return video.getHeight();
	}
	if (!isVideo() && image.isAllocated()) {
		return image.getHeight();
	}
	return 9.0f;
}

glm::vec3 MediaItem::getFeatureMotionOffset() const {
	if (!featureMotionEnabled || (isVideo() && videoPaused)) {
		return glm::vec3(0.0f);
	}

	const float t = ofGetElapsedTimef();
	const float luminance = ofClamp(metadata.meanLuminance, 0.0f, 1.0f);
	const float variance = ofClamp(metadata.luminanceVariance, 0.0f, 0.25f);
	const float edge = ofClamp(metadata.edgeDensity, 0.0f, 1.0f);
	const float texture = ofClamp(metadata.texture, 0.0f, 1.0f);
	const float motion = ofClamp(metadata.motionEnergy, 0.0f, 1.0f);
	const float rhythm = ofClamp(metadata.videoRhythm, 0.0f, 1.0f);

	const float sideAmplitude = 8.0f + edge * 120.0f + texture * 150.0f + motion * 80.0f;
	const float liftAmplitude = 6.0f + luminance * 24.0f + variance * 260.0f + motion * 90.0f;
	const float sideSpeed = 0.45f + texture * 1.4f + rhythm * 2.4f;
	const float liftSpeed = 0.55f + edge * 1.1f + motion * 2.6f;
	const float phase = static_cast<float>(std::hash<std::string>{}(fileName) % 628) * 0.01f;

	const float x = std::sin(t * sideSpeed + phase) * sideAmplitude;
	const float y = -std::abs(std::sin(t * liftSpeed + phase * 0.73f)) * liftAmplitude;
	const float z = std::cos(t * (0.35f + rhythm) + phase) * motion * 38.0f;
	return glm::vec3(x, y, z);
}

ofRectangle MediaItem::getScreenBounds(const ofCamera & camera) const {
	const float framePad = 26.0f;
	const float halfWidth = drawWidth * 0.5f + framePad;
	const float halfHeight = drawHeight * 0.5f + framePad;
	const glm::vec3 drawCenter = position + getFeatureMotionOffset() + glm::vec3(0.0f, -42.0f * hoverAmount, 42.0f * hoverAmount);
	const float scale = 1.0f + 0.14f * hoverAmount;

	std::vector<glm::vec3> corners = {
		drawCenter + glm::vec3(-halfWidth * scale, -halfHeight * scale, 0.0f),
		drawCenter + glm::vec3(halfWidth * scale, -halfHeight * scale, 0.0f),
		drawCenter + glm::vec3(halfWidth * scale, halfHeight * scale, 0.0f),
		drawCenter + glm::vec3(-halfWidth * scale, halfHeight * scale, 0.0f)
	};

	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float maxY = std::numeric_limits<float>::lowest();

	for (const auto & corner : corners) {
		const glm::vec3 screen = camera.worldToScreen(corner);
		minX = std::min(minX, screen.x);
		minY = std::min(minY, screen.y);
		maxX = std::max(maxX, screen.x);
		maxY = std::max(maxY, screen.y);
	}

	return ofRectangle(minX, minY, maxX - minX, maxY - minY);
}

ofColor MediaItem::getAccentColor() const {
	if (clusterIndex >= 0) {
		static const std::vector<ofColor> palette = {
			ofColor(65, 190, 130),
			ofColor(238, 92, 92),
			ofColor(80, 155, 235),
			ofColor(235, 184, 65),
			ofColor(215, 110, 205),
			ofColor(80, 210, 215),
			ofColor(245, 135, 75),
			ofColor(165, 200, 90)
		};
		return palette[clusterIndex % static_cast<int>(palette.size())];
	}

	const float hue = ofMap(metadata.meanLuminance, 0.0f, 1.0f, 150.0f, 32.0f, true);
	const float brightness = ofMap(metadata.edgeDensity, 0.0f, 0.45f, 145.0f, 255.0f, true);
	return ofColor::fromHsb(static_cast<unsigned char>(hue), 150, static_cast<unsigned char>(brightness));
}

std::string MediaItem::getMetadataLabel() const {
	std::stringstream ss;
	if (clusterIndex >= 0) {
		ss << "cluster " << (clusterIndex + 1);
		if (clusterCount > 0) {
			ss << "/" << clusterCount;
		}
		ss << " | ";
	}
	ss << "lum " << ofToString(metadata.meanLuminance, 2)
	   << " | var " << ofToString(metadata.luminanceVariance, 2)
	   << " | edge " << ofToString(metadata.edgeDensity, 2)
	   << "\ntex " << ofToString(metadata.texture, 2)
	   << " | kp " << metadata.keypoints
	   << " | motion " << ofToString(metadata.motionEnergy, 2);
	if (isVideo()) {
		ss << " | rhythm " << ofToString(metadata.videoRhythm, 2);
	}
	return ss.str();
}

std::string MediaItem::getShortFileName() const {
	const int maxChars = 34;
	if (fileName.size() <= maxChars) {
		return fileName;
	}
	return fileName.substr(0, maxChars - 3) + "...";
}

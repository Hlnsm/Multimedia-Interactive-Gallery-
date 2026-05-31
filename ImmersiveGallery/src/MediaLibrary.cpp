#include "MediaLibrary.h"

void MediaLibrary::setup(const std::string & mediaDirectory, MetadataManager & metadataManager) {
	mediaDir = mediaDirectory;
	metadata = &metadataManager;
	reload();
}

void MediaLibrary::reload() {
	items.clear();
	clusterAssignments.clear();
	hoveredIndex = -1;
	selectedIndex = -1;

	ofDirectory dir(mediaDir);
	if (!dir.exists()) {
		dir.create(true);
	}

	dir.listDir();
	dir.sort();

	for (std::size_t i = 0; i < dir.size(); ++i) {
		ofFile file = dir.getFile(i);
		if (!isSupportedMedia(file)) {
			continue;
		}

		const std::string type = detectType(file);
		auto item = std::make_shared<MediaItem>();
		if (!item->load(file, type)) {
			ofLogWarning("MediaLibrary") << "Erro carregar" << file.getFileName();
			continue;
		}

		const MediaMetadata mediaMetadata = metadata->loadOrCreate(file, type, item->getPixelsForMetadata());
		item->setMetadata(mediaMetadata);
		item->setFeatureMotionEnabled(featureMotionEnabled);
		items.push_back(item);
	}

	layout.applyGrid(items);
	activeClusterCount = 0;
}

void MediaLibrary::update() {
	for (auto & item : items) {
		item->update();
	}
}

void MediaLibrary::draw(bool showMetadata) {
	for (auto & item : items) {
		item->draw(showMetadata);
	}
}

void MediaLibrary::updateHover(const glm::vec2 & mousePosition, const ofCamera & camera) {
	hoveredIndex = -1;

	for (int i = static_cast<int>(items.size()) - 1; i >= 0; --i) {
		if (items[i]->hitTestScreen(mousePosition, camera)) {
			hoveredIndex = i;
			break;
		}
	}

	for (std::size_t i = 0; i < items.size(); ++i) {
		items[i]->setHovered(static_cast<int>(i) == hoveredIndex);
	}
}

bool MediaLibrary::selectHovered() {
	if (hoveredIndex < 0 || hoveredIndex >= static_cast<int>(items.size())) {
		return false;
	}

	selectedIndex = hoveredIndex;
	return true;
}

void MediaLibrary::clearSelection() {
	selectedIndex = -1;
}

bool MediaLibrary::hasSelection() const {
	return selectedIndex >= 0 && selectedIndex < static_cast<int>(items.size());
}

void MediaLibrary::drawSelectedFocus(const ofRectangle & bounds) {
	if (!hasSelection()) {
		return;
	}

	items[selectedIndex]->drawFocus(bounds);
}

void MediaLibrary::selectNext(int direction) {
	if (items.empty()) {
		return;
	}

	selectedIndex = getNextIndexInCurrentScope(direction);
}

void MediaLibrary::toggleVideoPlayback() {
	for (auto & item : items) {
		item->togglePlayback();
	}
}

void MediaLibrary::toggleSelectedPlayback() {
	if (!hasSelection()) {
		return;
	}

	items[selectedIndex]->togglePlayback();
}

void MediaLibrary::setFeatureMotionEnabled(bool enabled) {
	featureMotionEnabled = enabled;
	for (auto & item : items) {
		item->setFeatureMotionEnabled(featureMotionEnabled);
	}
}

void MediaLibrary::toggleFeatureMotion() {
	setFeatureMotionEnabled(!featureMotionEnabled);
}

void MediaLibrary::applyClusters(int requestedClusterCount) {
	if (items.empty()) {
		return;
	}

	activeClusterCount = ofClamp(requestedClusterCount, 1, static_cast<int>(items.size()));
	clusterAssignments = clusterManager.cluster(items, activeClusterCount);
	layout.applyClusterIslands(items, clusterAssignments, activeClusterCount);
}

void MediaLibrary::resetGrid() {
	activeClusterCount = 0;
	clusterAssignments.clear();
	layout.applyGrid(items);
}

std::size_t MediaLibrary::size() const {
	return items.size();
}

bool MediaLibrary::empty() const {
	return items.empty();
}

int MediaLibrary::getActiveClusterCount() const {
	return activeClusterCount;
}

bool MediaLibrary::isFeatureMotionEnabled() const {
	return featureMotionEnabled;
}

std::string MediaLibrary::getSelectedInfo() const {
	if (!hasSelection()) {
		return "";
	}

	return items[selectedIndex]->getFocusInfo();
}

bool MediaLibrary::isSupportedMedia(const ofFile & file) const {
	if (file.isDirectory()) {
		return false;
	}

	const std::string ext = ofToLower(ofFilePath::getFileExt(file.getFileName()));
	return ext == "jpg" || ext == "jpeg" || ext == "png" ||
	       ext == "gif" || ext == "mp4";
}

std::string MediaLibrary::detectType(const ofFile & file) const {
	const std::string ext = ofToLower(ofFilePath::getFileExt(file.getFileName()));
	if (ext == "mp4") {
		return "video";
	}
	return "image";
}

int MediaLibrary::getNextIndexInCurrentScope(int direction) const {
	const int itemCount = static_cast<int>(items.size());
	if (itemCount == 0) {
		return -1;
	}

	const int currentIndex = hasSelection() ? selectedIndex : 0;
	const int step = direction >= 0 ? 1 : -1;

	if (activeClusterCount > 0 &&
	    clusterAssignments.size() == items.size() &&
	    currentIndex >= 0 &&
	    currentIndex < static_cast<int>(clusterAssignments.size())) {
		const int currentCluster = clusterAssignments[currentIndex];
		std::vector<int> clusterIndices;
		for (int i = 0; i < itemCount; ++i) {
			if (clusterAssignments[i] == currentCluster) {
				clusterIndices.push_back(i);
			}
		}

		if (!clusterIndices.empty()) {
			auto currentIt = std::find(clusterIndices.begin(), clusterIndices.end(), currentIndex);
			int localIndex = currentIt == clusterIndices.end() ? 0 : static_cast<int>(std::distance(clusterIndices.begin(), currentIt));
			localIndex = (localIndex + step + static_cast<int>(clusterIndices.size())) % static_cast<int>(clusterIndices.size());
			return clusterIndices[localIndex];
		}
	}

	return (currentIndex + step + itemCount) % itemCount;
}

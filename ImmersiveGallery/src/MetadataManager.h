#pragma once

#include "MediaMetadata.h"
#include "ofMain.h"
#include "ofxXmlSettings.h"

class MetadataManager {
public:
	void setup(const std::string & mediaDirectory, const std::string & metadataDirectory);
	MediaMetadata loadOrCreate(const ofFile & mediaFile, const std::string & type, const ofPixels * imagePixels);
	const std::string & getMediaDirectory() const;
	const std::string & getMetadataDirectory() const;

private:
	std::string mediaDir = "media";
	std::string metadataDir = "metadata";

	MediaMetadata createDefaultMetadata(const ofFile & mediaFile, const std::string & type, const ofPixels * imagePixels) const;
	MediaMetadata computeImageMetadata(const ofFile & mediaFile, const std::string & type, const ofPixels & pixels) const;
	bool readMetadata(const std::string & xmlPath, MediaMetadata & metadata, const ofFile & mediaFile, const std::string & type) const;
	void writeMetadata(const std::string & xmlPath, const MediaMetadata & metadata) const;
	std::string getXmlPathFor(const std::string & fileName) const;
	std::string sanitizeFileName(const std::string & fileName) const;
	float luminanceAt(const ofPixels & pixels, int x, int y) const;
};

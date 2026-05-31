#include "MetadataManager.h"

#include <cctype>
#include <cmath>
#include <numeric>
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

namespace {
	const int kFeatureVersion = 3;
	const float kMotionEnergyScale = 14.0f;
	const float kVideoRhythmScale = 18.0f;

	cv::Mat pixelsToGrayMat(const ofPixels & pixels) {
		if (!pixels.isAllocated() || pixels.getWidth() <= 0 || pixels.getHeight() <= 0) {
			return cv::Mat();
		}

		const int channels = pixels.getNumChannels();
		cv::Mat source(pixels.getHeight(), pixels.getWidth(), CV_8UC(channels), const_cast<unsigned char *>(pixels.getData()));
		cv::Mat gray;

		if (channels == 1) {
			gray = source.clone();
		} else if (channels == 3) {
			cv::cvtColor(source, gray, cv::COLOR_RGB2GRAY);
		} else if (channels == 4) {
			cv::cvtColor(source, gray, cv::COLOR_RGBA2GRAY);
		}

		return gray;
	}

	int countOrbKeypoints(const cv::Mat & gray) {
		if (gray.empty()) {
			return 0;
		}

		cv::Mat working = gray;
		const int maxDimension = std::max(gray.cols, gray.rows);
		if (maxDimension > 720) {
			const double scale = 720.0 / static_cast<double>(maxDimension);
			cv::resize(gray, working, cv::Size(), scale, scale, cv::INTER_AREA);
		}

		std::vector<cv::KeyPoint> keypoints;
		cv::Ptr<cv::ORB> orb = cv::ORB::create(700);
		orb->detect(working, keypoints);
		return static_cast<int>(keypoints.size());
	}

	void fillGrayImageFeatures(MediaMetadata & metadata, const cv::Mat & gray) {
		if (gray.empty()) {
			return;
		}

		cv::Scalar mean;
		cv::Scalar stddev;
		cv::meanStdDev(gray, mean, stddev);
		metadata.meanLuminance = static_cast<float>(mean[0] / 255.0);
		metadata.luminanceVariance = static_cast<float>((stddev[0] / 255.0) * (stddev[0] / 255.0));

		cv::Mat resized;
		const int maxDimension = std::max(gray.cols, gray.rows);
		if (maxDimension > 480) {
			const double scale = 480.0 / static_cast<double>(maxDimension);
			cv::resize(gray, resized, cv::Size(), scale, scale, cv::INTER_AREA);
		} else {
			resized = gray;
		}

		double edgeHits = 0.0;
		double edgeSamples = 0.0;
		double textureSum = 0.0;
		for (int y = 0; y < resized.rows - 1; y += 2) {
			for (int x = 0; x < resized.cols - 1; x += 2) {
				const float here = resized.at<unsigned char>(y, x) / 255.0f;
				const float right = resized.at<unsigned char>(y, x + 1) / 255.0f;
				const float down = resized.at<unsigned char>(y + 1, x) / 255.0f;
				const float gradient = std::sqrt((here - right) * (here - right) + (here - down) * (here - down));
				textureSum += gradient;
				edgeHits += gradient > 0.18f ? 1.0 : 0.0;
				edgeSamples += 1.0;
			}
		}

		if (edgeSamples > 0.0) {
			metadata.edgeDensity = static_cast<float>(edgeHits / edgeSamples);
			metadata.texture = static_cast<float>(textureSum / edgeSamples);
		}

		metadata.keypoints = countOrbKeypoints(gray);
	}

	void fillVideoMotionFeatures(MediaMetadata & metadata, const std::string & path) {
		cv::VideoCapture capture(path);
		if (!capture.isOpened()) {
			return;
		}

		std::vector<float> frameEnergies;
		cv::Mat frame;
		cv::Mat previousGray;
		int samples = 0;

		while (samples < 60 && capture.read(frame)) {
			if (frame.empty()) {
				continue;
			}

			cv::Mat gray;
			cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
			if (samples == 0) {
				fillGrayImageFeatures(metadata, gray);
			}

			cv::Mat small;
			cv::resize(gray, small, cv::Size(160, 90), 0.0, 0.0, cv::INTER_AREA);

			if (!previousGray.empty()) {
				cv::Mat diff;
				cv::absdiff(previousGray, small, diff);
				const float energy = static_cast<float>(cv::mean(diff)[0] / 255.0);
				frameEnergies.push_back(energy);
			}

			previousGray = small;
			++samples;
		}

		if (frameEnergies.empty()) {
			return;
		}

		const float sum = std::accumulate(frameEnergies.begin(), frameEnergies.end(), 0.0f);
		const float rawMotionEnergy = sum / static_cast<float>(frameEnergies.size());

		float variance = 0.0f;
		for (float energy : frameEnergies) {
			const float delta = energy - rawMotionEnergy;
			variance += delta * delta;
		}
		const float rawVideoRhythm = std::sqrt(variance / static_cast<float>(frameEnergies.size()));

		metadata.motionEnergy = ofClamp(rawMotionEnergy * kMotionEnergyScale, 0.0f, 1.0f);
		metadata.videoRhythm = ofClamp(rawVideoRhythm * kVideoRhythmScale, 0.0f, 1.0f);
	}
}

void MetadataManager::setup(const std::string & mediaDirectory, const std::string & metadataDirectory) {
	mediaDir = mediaDirectory;
	metadataDir = metadataDirectory;

	ofDirectory media(mediaDir);
	if (!media.exists()) {
		media.create(true);
	}

	ofDirectory metadata(metadataDir);
	if (!metadata.exists()) {
		metadata.create(true);
	}
}

MediaMetadata MetadataManager::loadOrCreate(const ofFile & mediaFile, const std::string & type, const ofPixels * imagePixels) {
	const std::string xmlPath = getXmlPathFor(mediaFile.getFileName());
	MediaMetadata metadata;

	if (readMetadata(xmlPath, metadata, mediaFile, type)) {
		return metadata;
	}

	metadata = createDefaultMetadata(mediaFile, type, imagePixels);
	writeMetadata(xmlPath, metadata);
	return metadata;
}

const std::string & MetadataManager::getMediaDirectory() const {
	return mediaDir;
}

const std::string & MetadataManager::getMetadataDirectory() const {
	return metadataDir;
}

MediaMetadata MetadataManager::createDefaultMetadata(const ofFile & mediaFile, const std::string & type, const ofPixels * imagePixels) const {
	if (type == "image" && imagePixels != nullptr && imagePixels->isAllocated()) {
		return computeImageMetadata(mediaFile, type, *imagePixels);
	}

	MediaMetadata metadata;
	metadata.fileName = mediaFile.getFileName();
	metadata.type = type;
	if (type == "video") {
		fillVideoMotionFeatures(metadata, mediaFile.getAbsolutePath());
	}
	return metadata;
}

MediaMetadata MetadataManager::computeImageMetadata(const ofFile & mediaFile, const std::string & type, const ofPixels & pixels) const {
	MediaMetadata metadata;
	metadata.fileName = mediaFile.getFileName();
	metadata.type = type;

	const int width = pixels.getWidth();
	const int height = pixels.getHeight();
	const int count = width * height;
	if (count <= 0) {
		return metadata;
	}

	double sum = 0.0;
	double sumSq = 0.0;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			const float lum = luminanceAt(pixels, x, y);
			sum += lum;
			sumSq += lum * lum;
		}
	}

	const double mean = sum / count;
	const double variance = std::max(0.0, (sumSq / count) - (mean * mean));
	metadata.meanLuminance = static_cast<float>(mean);
	metadata.luminanceVariance = static_cast<float>(variance);

	double edgeHits = 0.0;
	double edgeSamples = 0.0;
	double textureSum = 0.0;
	for (int y = 0; y < height - 1; y += 2) {
		for (int x = 0; x < width - 1; x += 2) {
			const float here = luminanceAt(pixels, x, y);
			const float right = luminanceAt(pixels, x + 1, y);
			const float down = luminanceAt(pixels, x, y + 1);
			const float gradient = std::sqrt((here - right) * (here - right) + (here - down) * (here - down));
			textureSum += gradient;
			edgeHits += gradient > 0.18f ? 1.0 : 0.0;
			edgeSamples += 1.0;
		}
	}

	if (edgeSamples > 0.0) {
		metadata.edgeDensity = static_cast<float>(edgeHits / edgeSamples);
		metadata.texture = static_cast<float>(textureSum / edgeSamples);
	}

	metadata.keypoints = countOrbKeypoints(pixelsToGrayMat(pixels));

	return metadata;
}

bool MetadataManager::readMetadata(const std::string & xmlPath, MediaMetadata & metadata, const ofFile & mediaFile, const std::string & type) const {
	ofFile xmlFile(xmlPath);
	if (!xmlFile.exists()) {
		return false;
	}

	ofxXmlSettings xml;
	if (!xml.loadFile(xmlPath)) {
		return false;
	}

	if (!xml.pushTag("MEDIA")) {
		return false;
	}

	const int featureVersion = xml.getValue("FEATURES_VERSION", 0);
	if (featureVersion < kFeatureVersion) {
		xml.popTag();
		return false;
	}

	metadata.fileName = xml.getValue("FILE", mediaFile.getFileName());
	metadata.type = xml.getValue("TYPE", type);
	metadata.meanLuminance = xml.getValue("MEAN_LUMINANCE", 0.0f);
	metadata.luminanceVariance = xml.getValue("LUMINANCE_VARIANCE", 0.0f);
	metadata.edgeDensity = xml.getValue("EDGE_DENSITY", 0.0f);
	metadata.texture = xml.getValue("TEXTURE", 0.0f);
	metadata.keypoints = xml.getValue("KEYPOINTS_ORB", 0);
	metadata.motionEnergy = xml.getValue("MOTION_ENERGY", 0.0f);
	metadata.videoRhythm = xml.getValue("VIDEO_RHYTHM", 0.0f);
	xml.popTag();

	return true;
}

void MetadataManager::writeMetadata(const std::string & xmlPath, const MediaMetadata & metadata) const {
	ofxXmlSettings xml;
	xml.clear();
	xml.addTag("MEDIA");
	xml.pushTag("MEDIA");
	xml.setValue("FEATURES_VERSION", kFeatureVersion);
	xml.setValue("FILE", metadata.fileName);
	xml.setValue("TYPE", metadata.type);
	xml.setValue("MEAN_LUMINANCE", metadata.meanLuminance);
	xml.setValue("LUMINANCE_VARIANCE", metadata.luminanceVariance);
	xml.setValue("EDGE_DENSITY", metadata.edgeDensity);
	xml.setValue("TEXTURE", metadata.texture);
	xml.setValue("KEYPOINTS_ORB", metadata.keypoints);
	xml.setValue("MOTION_ENERGY", metadata.motionEnergy);
	xml.setValue("VIDEO_RHYTHM", metadata.videoRhythm);
	xml.popTag();
	xml.saveFile(xmlPath);
}

std::string MetadataManager::getXmlPathFor(const std::string & fileName) const {
	return metadataDir + "/" + sanitizeFileName(fileName) + ".xml";
}

std::string MetadataManager::sanitizeFileName(const std::string & fileName) const {
	std::string clean;
	for (char c : fileName) {
		if (std::isalnum(static_cast<unsigned char>(c))) {
			clean += c;
		} else {
			clean += "_";
		}
	}
	return clean;
}

float MetadataManager::luminanceAt(const ofPixels & pixels, int x, int y) const {
	const ofColor color = pixels.getColor(x, y);
	return (0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b) / 255.0f;
}

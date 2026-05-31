#pragma once

#include "MediaItem.h"
#include "ofMain.h"

class ClusterManager {
public:
	std::vector<int> cluster(const std::vector<std::shared_ptr<MediaItem>> & items, int requestedClusterCount) const;
	float distance(const MediaMetadata & a, const MediaMetadata & b) const;

private:
	struct FeatureVector {
		float meanLuminance = 0.0f;
		float luminanceVariance = 0.0f;
		float edgeDensity = 0.0f;
		float texture = 0.0f;
		float keypoints = 0.0f;
		float motionEnergy = 0.0f;
	};

	std::vector<FeatureVector> buildNormalizedFeatures(const std::vector<std::shared_ptr<MediaItem>> & items) const;
	FeatureVector featureFromMetadata(const MediaMetadata & metadata) const;
	float distance(const FeatureVector & a, const FeatureVector & b) const;
	FeatureVector average(const std::vector<FeatureVector> & features, const std::vector<int> & assignments, int clusterIndex) const;
};

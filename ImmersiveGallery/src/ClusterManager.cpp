#include "ClusterManager.h"

#include <limits>

std::vector<int> ClusterManager::cluster(const std::vector<std::shared_ptr<MediaItem>> & items, int requestedClusterCount) const {
	std::vector<int> assignments(items.size(), 0);
	if (items.empty()) {
		return assignments;
	}

	const int clusterCount = ofClamp(requestedClusterCount, 1, static_cast<int>(items.size()));
	const std::vector<FeatureVector> features = buildNormalizedFeatures(items);
	std::vector<FeatureVector> centroids;
	centroids.reserve(clusterCount);

	for (int i = 0; i < clusterCount; ++i) {
		const int sourceIndex = static_cast<int>(ofMap(i, 0, std::max(1, clusterCount - 1), 0, static_cast<int>(features.size()) - 1, true));
		centroids.push_back(features[sourceIndex]);
	}

	for (int iteration = 0; iteration < 24; ++iteration) {
		bool changed = false;

		for (std::size_t i = 0; i < features.size(); ++i) {
			float bestDistance = std::numeric_limits<float>::max();
			int bestCluster = 0;

			for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
				const float currentDistance = distance(features[i], centroids[clusterIndex]);
				if (currentDistance < bestDistance) {
					bestDistance = currentDistance;
					bestCluster = clusterIndex;
				}
			}

			if (assignments[i] != bestCluster) {
				assignments[i] = bestCluster;
				changed = true;
			}
		}

		for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
			centroids[clusterIndex] = average(features, assignments, clusterIndex);
		}

		if (!changed) {
			break;
		}
	}

	return assignments;
}

float ClusterManager::distance(const MediaMetadata & a, const MediaMetadata & b) const {
	return distance(featureFromMetadata(a), featureFromMetadata(b));
}

std::vector<ClusterManager::FeatureVector> ClusterManager::buildNormalizedFeatures(const std::vector<std::shared_ptr<MediaItem>> & items) const {
	std::vector<FeatureVector> features;
	features.reserve(items.size());

	for (const auto & item : items) {
		features.push_back(featureFromMetadata(item->getMetadata()));
	}

	FeatureVector minValues = features.front();
	FeatureVector maxValues = features.front();
	for (const auto & feature : features) {
		minValues.meanLuminance = std::min(minValues.meanLuminance, feature.meanLuminance);
		minValues.luminanceVariance = std::min(minValues.luminanceVariance, feature.luminanceVariance);
		minValues.edgeDensity = std::min(minValues.edgeDensity, feature.edgeDensity);
		minValues.texture = std::min(minValues.texture, feature.texture);
		minValues.keypoints = std::min(minValues.keypoints, feature.keypoints);
		minValues.motionEnergy = std::min(minValues.motionEnergy, feature.motionEnergy);

		maxValues.meanLuminance = std::max(maxValues.meanLuminance, feature.meanLuminance);
		maxValues.luminanceVariance = std::max(maxValues.luminanceVariance, feature.luminanceVariance);
		maxValues.edgeDensity = std::max(maxValues.edgeDensity, feature.edgeDensity);
		maxValues.texture = std::max(maxValues.texture, feature.texture);
		maxValues.keypoints = std::max(maxValues.keypoints, feature.keypoints);
		maxValues.motionEnergy = std::max(maxValues.motionEnergy, feature.motionEnergy);
	}

	auto normalize = [](float value, float minValue, float maxValue) {
		const float range = maxValue - minValue;
		if (std::abs(range) < 0.0001f) {
			return 0.0f;
		}
		return (value - minValue) / range;
	};

	for (auto & feature : features) {
		feature.meanLuminance = normalize(feature.meanLuminance, minValues.meanLuminance, maxValues.meanLuminance);
		feature.luminanceVariance = normalize(feature.luminanceVariance, minValues.luminanceVariance, maxValues.luminanceVariance);
		feature.edgeDensity = normalize(feature.edgeDensity, minValues.edgeDensity, maxValues.edgeDensity);
		feature.texture = normalize(feature.texture, minValues.texture, maxValues.texture);
		feature.keypoints = normalize(feature.keypoints, minValues.keypoints, maxValues.keypoints);
		feature.motionEnergy = normalize(feature.motionEnergy, minValues.motionEnergy, maxValues.motionEnergy);
	}

	return features;
}

ClusterManager::FeatureVector ClusterManager::featureFromMetadata(const MediaMetadata & metadata) const {
	FeatureVector feature;
	feature.meanLuminance = metadata.meanLuminance;
	feature.luminanceVariance = metadata.luminanceVariance;
	feature.edgeDensity = metadata.edgeDensity;
	feature.texture = metadata.texture;
	feature.keypoints = static_cast<float>(metadata.keypoints);
	feature.motionEnergy = metadata.motionEnergy;
	return feature;
}

float ClusterManager::distance(const FeatureVector & a, const FeatureVector & b) const {
	const float luminanceWeight = 1.0f;
	const float varianceWeight = 1.0f;
	const float edgeWeight = 1.0f;
	const float textureWeight = 1.0f;
	const float keypointWeight = 0.9f;
	const float motionWeight = 1.15f;

	const float dLum = a.meanLuminance - b.meanLuminance;
	const float dVar = a.luminanceVariance - b.luminanceVariance;
	const float dEdge = a.edgeDensity - b.edgeDensity;
	const float dTexture = a.texture - b.texture;
	const float dKeypoints = a.keypoints - b.keypoints;
	const float dMotion = a.motionEnergy - b.motionEnergy;

	return std::sqrt(
		luminanceWeight * dLum * dLum +
		varianceWeight * dVar * dVar +
		edgeWeight * dEdge * dEdge +
		textureWeight * dTexture * dTexture +
		keypointWeight * dKeypoints * dKeypoints +
		motionWeight * dMotion * dMotion);
}

ClusterManager::FeatureVector ClusterManager::average(const std::vector<FeatureVector> & features, const std::vector<int> & assignments, int clusterIndex) const {
	FeatureVector result;
	int count = 0;

	for (std::size_t i = 0; i < features.size(); ++i) {
		if (assignments[i] != clusterIndex) {
			continue;
		}

		result.meanLuminance += features[i].meanLuminance;
		result.luminanceVariance += features[i].luminanceVariance;
		result.edgeDensity += features[i].edgeDensity;
		result.texture += features[i].texture;
		result.keypoints += features[i].keypoints;
		result.motionEnergy += features[i].motionEnergy;
		++count;
	}

	if (count == 0) {
		const int fallbackIndex = clusterIndex % static_cast<int>(features.size());
		return features[fallbackIndex];
	}

	result.meanLuminance /= count;
	result.luminanceVariance /= count;
	result.edgeDensity /= count;
	result.texture /= count;
	result.keypoints /= count;
	result.motionEnergy /= count;
	return result;
}

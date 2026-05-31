#pragma once

#include "ofMain.h"

struct MediaMetadata {
	std::string fileName;
	std::string type;
	float meanLuminance = 0.0f;
	float luminanceVariance = 0.0f;
	float edgeDensity = 0.0f;
	float texture = 0.0f;
	int keypoints = 0;
	float motionEnergy = 0.0f;
	float videoRhythm = 0.0f;
};

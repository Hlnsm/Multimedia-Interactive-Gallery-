#pragma once

#include "CameraController.h"
#include "MediaLibrary.h"
#include "MetadataManager.h"
#include "ofMain.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);

private:
	CameraController camera;
	MetadataManager metadata;
	MediaLibrary library;
	bool showHelp = true;
	bool showMetadata = true;
	bool clusterInputActive = false;
	bool focusMode = false;
	std::string clusterInput = "3";

	void drawSceneReference();
	void drawHud();
	void drawFocusView();
	void startClusterInput();
	void handleClusterInputKey(int key);
	void handleFocusKey(int key);
	void applyClusterInput();
};

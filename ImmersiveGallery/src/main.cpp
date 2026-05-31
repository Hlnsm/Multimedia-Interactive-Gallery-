#include "ofMain.h"
#include "ofApp.h"

int main() {
	ofGLWindowSettings settings;
	settings.setSize(1280, 720);
	settings.windowMode = OF_WINDOW;
	settings.setPosition(glm::vec2(80, 80));

	auto window = ofCreateWindow(settings);
	ofRunApp(window, std::make_shared<ofApp>());
	ofRunMainLoop();
}

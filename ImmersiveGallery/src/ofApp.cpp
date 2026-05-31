#include "ofApp.h"

void ofApp::setup() {
	ofSetWindowTitle("Galleria");
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	ofEnableDepthTest();

	metadata.setup("media", "metadata");
	library.setup(metadata.getMediaDirectory(), metadata);
	camera.setup();
}

void ofApp::update() {
	const float dt = std::min<float>(static_cast<float>(ofGetLastFrameTime()), 0.05f);

	if (!focusMode) {
		camera.update(dt);
		if (!clusterInputActive) {
			library.updateHover(glm::vec2(ofGetMouseX(), ofGetMouseY()), camera.getCamera());
		}
	}
	library.update();
}

void ofApp::draw() {
	ofBackgroundGradient(ofColor(19, 22, 28), ofColor(5, 6, 9), OF_GRADIENT_CIRCULAR);

	if (focusMode) {
		drawFocusView();
		return;
	}

	ofEnableDepthTest();
	camera.begin();
	drawSceneReference();
	library.draw(showMetadata);
	camera.end();
	ofDisableDepthTest();

	drawHud();
}

void ofApp::keyPressed(int key) {
	if (clusterInputActive) {
		handleClusterInputKey(key);
		return;
	}

	if (focusMode) {
		handleFocusKey(key);
		return;
	}

	camera.keyPressed(key);

	switch (key) {
	case 'h':
	case 'H':
		showHelp = !showHelp;
		break;
	case 'm':
	case 'M':
		showMetadata = !showMetadata;
		break;
	case 'r':
	case 'R':
		library.reload();
		break;
	case 'c':
	case 'C':
		startClusterInput();
		break;
	case 'g':
	case 'G':
		library.resetGrid();
		break;
	case 'f':
	case 'F':
		ofToggleFullscreen();
		break;
	case ' ':
		library.toggleFeatureMotion();
		break;
	default:
		break;
	}
}

void ofApp::keyReleased(int key) {
	camera.keyReleased(key);
}

void ofApp::mouseMoved(int x, int y) {
	if (!focusMode && !clusterInputActive) {
		library.updateHover(glm::vec2(x, y), camera.getCamera());
	}
}

void ofApp::mouseDragged(int x, int y, int button) {
	if (!focusMode && !clusterInputActive) {
		library.updateHover(glm::vec2(x, y), camera.getCamera());
	}
}

void ofApp::mousePressed(int x, int y, int button) {
	if (focusMode || clusterInputActive || button != OF_MOUSE_BUTTON_LEFT) {
		return;
	}

	library.updateHover(glm::vec2(x, y), camera.getCamera());
	if (library.selectHovered()) {
		focusMode = true;
	}
}

void ofApp::windowResized(int w, int h) {
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
}

void ofApp::drawSceneReference() {
	ofPushStyle();
	ofSetLineWidth(1.0f);
	const float backZ = -260.0f;
	const float width = 1800.0f;
	const float height = 1080.0f;
	const float step = 120.0f;

	ofSetColor(55, 62, 72, 100);
	for (float x = -width * 0.5f; x <= width * 0.5f; x += step) {
		ofDrawLine(glm::vec3(x, -height * 0.5f, backZ), glm::vec3(x, height * 0.5f, backZ));
	}
	for (float y = -height * 0.5f; y <= height * 0.5f; y += step) {
		ofDrawLine(glm::vec3(-width * 0.5f, y, backZ), glm::vec3(width * 0.5f, y, backZ));
	}

	ofSetColor(255, 255, 255, 28);
	ofNoFill();
	ofDrawBox(0.0f, 0.0f, backZ, width, height, 1.0f);
	ofPopStyle();
}

void ofApp::drawHud() {
	ofPushStyle();
	ofSetColor(0, 170);
	const float panelHeight = showHelp ? (clusterInputActive || library.getActiveClusterCount() > 0 ? 240.0f : 196.0f) : 74.0f;
	ofDrawRectangle(16, 16, 620, panelHeight);

	ofSetColor(245);
	std::stringstream ss;
	ss << " | movimento por features: " << (library.isFeatureMotionEnabled() ? "ativo" : "pausado");

	if (showHelp) {
		ss << "\nWASD /mover camara   Q/E: mover na vertical"
		   << "\nEspaco: pausa/continua movimento dos objetos"
		   << "\n V: voltar a galeria"
		   << "\nC: Organizar   Enter: confirmar numero   G: grelha"
		   << "\nM: mostrar metadata   F: fullscreen   H: ajuda";
		
	}

	if (clusterInputActive) {
		ss << "\n\nNumero de clusters: " << clusterInput << "  (escreve o numero, Enter aplica, Esc cancela)";
	} else if (library.getActiveClusterCount() > 0) {
		ss << "\n\nA mostrar " << library.getActiveClusterCount() << " ilhas de clusters. Carrega G para grelha.";
	}

	ofDrawBitmapString(ss.str(), 28, 38);
	ofPopStyle();
}

void ofApp::drawFocusView() {
	ofDisableDepthTest();

	ofPushStyle();
	ofSetColor(0, 185);
	ofDrawRectangle(0, 0, ofGetWidth(), 116);

	ofSetColor(245);
	std::stringstream ss;
	ss << library.getSelectedInfo()
	   << "\nSeta esquerda/direita: anterior/seguinte"
	   << "   V: galeria";
	ofDrawBitmapString(ss.str(), 30, 28);

	const float margin = 72.0f;
	const ofRectangle mediaBounds(
		margin,
		138.0f,
		ofGetWidth() - margin * 2.0f,
		ofGetHeight() - 178.0f);
	library.drawSelectedFocus(mediaBounds);
	ofPopStyle();
}

void ofApp::startClusterInput() {
	if (library.empty()) {
		return;
	}

	const int defaultCount = ofClamp(library.getActiveClusterCount() > 0 ? library.getActiveClusterCount() : 3, 1, static_cast<int>(library.size()));
	clusterInput = ofToString(defaultCount);
	clusterInputActive = true;
}

void ofApp::handleClusterInputKey(int key) {
	if (key == OF_KEY_RETURN) {
		applyClusterInput();
		return;
	}

	if (key == OF_KEY_ESC) {
		clusterInputActive = false;
		return;
	}

	if (key == OF_KEY_BACKSPACE) {
		if (!clusterInput.empty()) {
			clusterInput.pop_back();
		}
		return;
	}

	if (key >= '0' && key <= '9' && clusterInput.size() < 2) {
		if (clusterInput == "0") {
			clusterInput.clear();
		}
		clusterInput.push_back(static_cast<char>(key));
	}
}

void ofApp::handleFocusKey(int key) {
	switch (key) {
	case 'v':
	case 'V':
		focusMode = false;
		library.clearSelection();
		break;
	case OF_KEY_RIGHT:
		library.selectNext(1);
		break;
	case OF_KEY_LEFT:
		library.selectNext(-1);
		break;
	case ' ':
		library.toggleSelectedPlayback();
		break;
	case 'f':
	case 'F':
		ofToggleFullscreen();
		break;
	default:
		break;
	}
}

void ofApp::applyClusterInput() {
	const int requestedClusters = ofToInt(clusterInput);
	if (requestedClusters > 0) {
		library.applyClusters(requestedClusters);
	}
	clusterInputActive = false;
}

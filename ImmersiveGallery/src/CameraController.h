#pragma once

#include "ofMain.h"
#include <set>

class CameraController {
public:
	void setup();
	void update(float dt);
	void begin();
	void end();
	void keyPressed(int key);
	void keyReleased(int key);
	glm::vec3 getPosition() const;
	ofEasyCam & getCamera();

private:
	ofEasyCam cam;
	std::set<int> pressedKeys;
	glm::vec3 velocity = glm::vec3(0.0f);
	float moveSpeed = 520.0f;
	float damping = 0.82f;

	bool isDown(int key) const;
};

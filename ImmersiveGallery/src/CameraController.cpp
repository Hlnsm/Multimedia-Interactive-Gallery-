#include "CameraController.h"

void CameraController::setup() {
	cam.setDistance(950.0f);
	cam.setNearClip(0.1f);
	cam.setFarClip(6000.0f);
	cam.enableMouseInput();
	cam.disableInertia();
}

void CameraController::update(float dt) {
	glm::vec3 input(0.0f);
	if (isDown('a') || isDown('A') || isDown(OF_KEY_LEFT)) {
		input.x -= 1.0f;
	}
	if (isDown('d') || isDown('D') || isDown(OF_KEY_RIGHT)) {
		input.x += 1.0f;
	}
	if (isDown('q') || isDown('Q')) {
		input.y -= 1.0f;
	}
	if (isDown('e') || isDown('E')) {
		input.y += 1.0f;
	}
	if (isDown('w') || isDown('W') || isDown(OF_KEY_UP)) {
		input.z -= 1.0f;
	}
	if (isDown('s') || isDown('S') || isDown(OF_KEY_DOWN)) {
		input.z += 1.0f;
	}

	if (glm::length(input) > 0.0f) {
		input = glm::normalize(input);
	}

	const glm::vec3 targetVelocity = input * moveSpeed;
	velocity = velocity * damping + targetVelocity * (1.0f - damping);

	cam.truck(velocity.x * dt);
	cam.boom(velocity.y * dt);
	cam.dolly(velocity.z * dt);
}

void CameraController::begin() {
	cam.begin();
}

void CameraController::end() {
	cam.end();
}

void CameraController::keyPressed(int key) {
	pressedKeys.insert(key);
}

void CameraController::keyReleased(int key) {
	pressedKeys.erase(key);
}

glm::vec3 CameraController::getPosition() const {
	return cam.getPosition();
}

ofEasyCam & CameraController::getCamera() {
	return cam;
}

bool CameraController::isDown(int key) const {
	return pressedKeys.find(key) != pressedKeys.end();
}

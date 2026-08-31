#include "BouncingBallMovement.h"

#include <cmath>

#include "Active.h"
#include "Application.h"

void BouncingBallMovement::OnEnabled() {
	std::vector<int> directions;
	for (int i = 0; i < 32; i++) {
		if (DirectionAtStart & (1 << i)) {
			directions.push_back(i);
		}
	}

	// direction will just be right if no directions are set
	if (!directions.empty()) {
		movementDirection = directions[Application::Instance().RandomRange(0, static_cast<short>(directions.size() - 1)).GetIntValue()];
	}

	realSpeed = static_cast<float>(std::clamp<int>(speed, 0, 250));
	hasStopSpeed = false;
	stopSpeed = 0.0f;
	subPixelX = 0.0f;
	subPixelY = 0.0f;

	if (!MovingAtStart || stopped) {
		Stop();
	}
}

void BouncingBallMovement::Start() {
	stopped = false;
	if (hasStopSpeed) {
		realSpeed = stopSpeed;
		hasStopSpeed = false;
		stopSpeed = 0.0f;
	}
}

void BouncingBallMovement::Stop() {
	if (!hasStopSpeed) {
		stopSpeed = realSpeed;
		hasStopSpeed = true;
		realSpeed = 0.0f;
	}
	stopped = true;
}

void BouncingBallMovement::SetCurrentSpeed(int newSpeed) {
	const int clampedSpeed = newSpeed < 0 ? 0 : (newSpeed > 250 ? 250 : newSpeed);
	speed = static_cast<short>(clampedSpeed);
	realSpeed = static_cast<float>(clampedSpeed);
	hasStopSpeed = false;
	stopSpeed = 0.0f;
	stopped = false;
}

void BouncingBallMovement::Update(float deltaTime) {
	const float movementBaseTime = deltaTime * 60.0f;

	if (!stopped && deceleration > 0 && realSpeed > 0.0f) {
		realSpeed -= static_cast<float>(GetAcceleratorValue(deceleration)) * movementBaseTime / 256.0f;
		if (realSpeed < 0.0f) {
			realSpeed = 0.0f;
		}
	}

	const double deltaX = static_cast<double>(GetCosinus32(movementDirection & 31)) * static_cast<int>(realSpeed) * movementBaseTime / 2048.0;
	const double deltaY = static_cast<double>(GetSinus32(movementDirection & 31)) * static_cast<int>(realSpeed) * movementBaseTime / 2048.0;

	const double nextX = static_cast<double>(Instance->GetX().GetIntValue()) + subPixelX + deltaX;
	const double nextY = static_cast<double>(Instance->GetY().GetIntValue()) + subPixelY + deltaY;

	Instance->SetX(static_cast<int>(nextX));
	Instance->SetY(static_cast<int>(nextY));

	subPixelX = static_cast<float>(nextX - static_cast<int>(nextX));
	subPixelY = static_cast<float>(nextY - static_cast<int>(nextY));

	if (!((Active*)Instance)->AutomaticRotation) {
		((Active*)Instance)->animations.SetCurrentDirection(movementDirection);
	}
}

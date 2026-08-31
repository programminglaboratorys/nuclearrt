#pragma once

#include "Movement.h"

class BouncingBallMovement : public Movement
{
public:
	BouncingBallMovement(unsigned short player, bool movingAtStart, int directionAtStart, short speed, short randomizer, short angles, short security, short deceleration)
		: Movement(player, movingAtStart, directionAtStart), speed(speed), randomizer(randomizer), angles(angles), security(security), deceleration(deceleration) {}

	short speed;
	short randomizer;
	short angles;
	short security;
	short deceleration;

	float realSpeed = 0;
	float stopSpeed = 0;
	bool hasStopSpeed = false;
	float subPixelX = 0;
	float subPixelY = 0;

	CValue GetRealSpeed() override { return CValue(realSpeed); }
	int GetMinimumSpeed() override { return 0; }
	int GetMaximumSpeed() override { return static_cast<int>(speed); }
	int GetMovementDirection() override { return movementDirection; }
	void SetCurrentSpeed(int speed) override;
	void OnEnabled() override;
	void Start() override;
	void Stop() override;
	void Update(float deltaTime) override;
};
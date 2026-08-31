#pragma once

#include <unordered_map>
#include <memory>
#include <vector>

#include "ObjectInstance.h"
#include "Application.h"

class Movement
{
public:
	Movement() = default;
	Movement(unsigned short player, bool movingAtStart, int directionAtStart)
		: Player(player), MovingAtStart(movingAtStart), DirectionAtStart(directionAtStart) {}

	unsigned short Player;
	bool MovingAtStart;
	int DirectionAtStart;

	ObjectInstance* Instance;

	bool stopped = false; //set by events

	int movementDirection = 0;

	virtual ~Movement() = default;
	virtual void Initialize() {} // called for each movement at the start of the frame
	virtual void OnEnabled() {} // called when the movement is switched to
	virtual void OnDisabled() {} // called when the movement is switched to another movement
	virtual CValue GetRealSpeed() { return CValue(0); }
	virtual int GetMinimumSpeed() { return 0; }
	virtual int GetMaximumSpeed() { return 0; }
	virtual bool IsStopped() { return GetRealSpeed() == 0; }
	virtual void SetCurrentSpeed(int speed) {}
	virtual void SetMovementDirection(int directionMask) { 
		if (directionMask == 0) return;
		
		std::vector<int> directions;
		for (int i = 0; i < 32; i++) {
			if (directionMask & (1 << i)) {
				directions.push_back(i);
			}
		}
		//todo: only set valid directions
		movementDirection = directions[Application::Instance().RandomRange(0, static_cast<short>(directions.size() - 1)).GetIntValue()];
	}

	bool IsDirectionValid(int direction, int validDirectionsMask) {
		return (1 << direction) & validDirectionsMask;
	}
	
	void LookAtObject(ObjectInstance* targetObject, int xOffset, int yOffset);
	void LookAtPoint(int x, int y);
	virtual int GetMovementDirection() { return movementDirection; } // 0-31 with 0 being right and going counter-clockwise
	virtual void Start() {}
	virtual void Stop() {}
	virtual void Update(float deltaTime) {}

	int GetCosinus32(int direction) {
		return Cosinus32[direction & 31];
	}

	int GetSinus32(int direction) {
		return Sinus32[direction & 31];
	}

	int GetAcceleratorValue(int acceleration) {
		if (acceleration <= 0) {
			return 0;
		}
		if (acceleration > 100) {
			return acceleration << 8;
		}
		return Accelerators[acceleration];
	}

private:
	static constexpr int Cosinus32[32] = {
		256, 251, 236, 212, 181, 142, 97, 49,
		0, -49, -97, -142, -181, -212, -236, -251,
		-256, -251, -236, -212, -181, -142, -97, -49,
		0, 49, 97, 142, 181, 212, 236, 251
	};
	static constexpr int Sinus32[32] = {
		0, -49, -97, -142, -181, -212, -236, -251,
		-256, -251, -236, -212, -181, -142, -97, -49,
		0, 49, 97, 142, 181, 212, 236, 251,
		256, 251, 236, 212, 181, 142, 97, 49
	};

	static constexpr int Accelerators[101] = {
		0x0002, 0x0003, 0x0004, 0x0006, 0x0008, 0x000a, 0x000c, 0x0010, 0x0014, 0x0018,
		0x0030, 0x0038, 0x0040, 0x0048, 0x0050, 0x0058, 0x0060, 0x0068, 0x0070, 0x0078,
		0x0090, 0x00A0, 0x00B0, 0x00c0, 0x00d0, 0x00e0, 0x00f0, 0x0100, 0x0110, 0x0120,
		0x0140, 0x0150, 0x0160, 0x0170, 0x0180, 0x0190, 0x01a0, 0x01b0, 0x01c0, 0x01e0,
		0x0200, 0x0220, 0x0230, 0x0250, 0x0270, 0x0280, 0x02a0, 0x02b0, 0x02d0, 0x02e0,
		0x0300, 0x0310, 0x0330, 0x0350, 0x0360, 0x0380, 0x03a0, 0x03b0, 0x03d0, 0x03e0,
		0x0400, 0x0460, 0x04c0, 0x0520, 0x05a0, 0x0600, 0x0660, 0x06c0, 0x0720, 0x07a0,
		0x0800, 0x08c0, 0x0980, 0x0a80, 0x0b40, 0x0c00, 0x0cc0, 0x0d80, 0x0e80, 0x0f40,
		0x1000, 0x1990, 0x1332, 0x1460, 0x1664, 0x1800, 0x1999, 0x1b32, 0x1cc6, 0x1e64,
		0x2000, 0x266c, 0x2d98, 0x3404, 0x3a70, 0x40dc, 0x4748, 0x4db4, 0x5400, 0x6400,
		0x6400
	};
};
#pragma once

#ifdef NUCLEAR_BACKEND_SDL2

#include "InputBackend.h"

#include <SDL2/SDL.h>
#include <vector>

class SDL2Backend;

class SDL2InputBackend : public InputBackend {
public:
	void SetBackend(SDL2Backend* b) { backend = b; }

	void GetKeyboardState(uint8_t* outBuffer) override;
	int GetMouseX() override;
	int GetMouseY() override;
	void SetMouseX(int x) override;
	void SetMouseY(int y) override;
	int GetMouseWheelMove() override;
	uint32_t GetMouseState() override;
	void HideMouseCursor() override;
	void ShowMouseCursor() override;

	bool IsGamepadConnected(int index) override;
	uint8_t GetGamepadButtonState(int index) override;

	int FusionToSDLKey(short key);

	std::vector<SDL_GameController*> gamepads = {};

#ifdef PLATFORM_SWITCH
	int touchX = 0;
	int touchY = 0;
	bool touchDown = false;
#endif

private:
	SDL2Backend* backend = nullptr;
}; 
#endif
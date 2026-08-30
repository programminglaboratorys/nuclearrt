#pragma once

#ifdef NUCLEAR_BACKEND_SDL2

#include "PlatformBackend.h"
#include <vector>

class SDL2Backend;

class SDL2PlatformBackend : public PlatformBackend {
public:
	void SetBackend(SDL2Backend* b) { backend = b; }
	
	std::string GetName() const override { return "SDL2"; }

	void Initialize() override;

	bool ShouldQuit() override;

	unsigned int GetTicks() override;
	float GetTimeDelta() override;
	void Delay(unsigned int ms) override;

	std::string GetPlatformName() override;
	std::string GetAssetsDirectory() override;

	std::string GetAppDrive() override;
	std::string GetAppDirectory() override;
	std::string GetAppPath() override;

	bool IsWindowFocused() const { return windowFocused; }

	void SetFullscreen(bool fullscreen) override;

	void Log(std::string text) override;
private:
	SDL2Backend* backend = nullptr;

	bool windowFocused = true;
}; 
#endif
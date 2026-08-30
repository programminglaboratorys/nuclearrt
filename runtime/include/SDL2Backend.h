#pragma once

#ifdef NUCLEAR_BACKEND_SDL2

#include "Backend.h"

#include "SDL2PlatformBackend.h"
#include "SDL2GraphicsBackend.h"
#include "SDL2AudioBackend.h"
#include "SDL2InputBackend.h"

class SDL2Backend : public Backend {
public:
	SDL2Backend() {
		platform = new SDL2PlatformBackend();
		graphics = new SDL2GraphicsBackend();
		audio = new SDL2AudioBackend();
		input = new SDL2InputBackend();
		
		GetPlatform()->SetBackend(this);
		GetGraphics()->SetBackend(this);
		GetAudio()->SetBackend(this);
		GetInput()->SetBackend(this);

		platform->Initialize();
		graphics->Initialize();
		audio->Initialize();
		input->Initialize();
	}
	
	~SDL2Backend() {
		platform->Deinitialize();
		graphics->Deinitialize();
		audio->Deinitialize();
		input->Deinitialize();
		delete platform;
		delete graphics;
		delete audio;
		delete input;
	}

	SDL2GraphicsBackend* GetGraphics() const { return static_cast<SDL2GraphicsBackend*>(graphics); }
	SDL2AudioBackend* GetAudio() const { return static_cast<SDL2AudioBackend*>(audio); }
	SDL2InputBackend* GetInput() const { return static_cast<SDL2InputBackend*>(input); }
	SDL2PlatformBackend* GetPlatform() const { return static_cast<SDL2PlatformBackend*>(platform); }
}; 
#endif
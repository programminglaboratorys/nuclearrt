#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <algorithm>
#include <type_traits>

#include "AppData.h"
#include "Backend.h"
#include "CValue.h"
#include "GameState.h"
#include "Input.h"
#include "ObjectGlobalData.h"

class Frame;

class Application
{
public:
	static Application& Instance()
	{
		static Application instance;
		return instance;
	}

	AppData* GetAppData() const { return appData; }
	void SetAppData(AppData* data) { appData = data; }

	Backend* GetBackend() const { return backend; }
    void SetBackend(Backend* b) { backend = b; }

	Input* GetInput() const { return input; }
	void SetInput(Input* i) { input = i; }

	std::unique_ptr<Frame>& GetCurrentFrame() { return currentFrame; }

	void Initialize();

	void Update();

	void Draw();

	void Shutdown();

	void Run();

	void QueueStateChange(GameState newState, int frameIndex = -1);
	GameState GetCurrentState() const { return currentState; }

	CValue Random(CValue max);
	CValue Random(short max);
	CValue RandomRange(CValue min, CValue max);
	CValue RandomRange(short min, short max);
	bool RandomChance(CValue chance, CValue max);
	bool RandomChance(short chance, short max);

	Application();
	~Application();
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

private:
	AppData* appData = nullptr;
	Backend* backend = nullptr;
	Input* input = nullptr;
	
	std::unique_ptr<Frame> currentFrame;
	void LoadFrame(int frameIndex);

	GameState currentState;
	int newFrameIndex;
	void RunState();

	std::vector<ObjectGlobalData*> globalObjectData;
	void MergeGlobalObjectData(std::vector<ObjectGlobalData*> frameData);
};
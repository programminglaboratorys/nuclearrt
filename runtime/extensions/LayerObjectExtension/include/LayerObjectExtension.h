#pragma once

#include "Application.h"
#include "Extension.h"
#include "ObjectInstance.h"
	
class LayerObjectExtension : public Extension {
public:
	LayerObjectExtension(unsigned int objectInfoHandle, int type, std::string name)
		: Extension(objectInfoHandle, type, name) {}

	void Initialize() override;

	void YSort(bool ascending = true);
	void XSort(bool ascending = true);

	void AltValueSort(bool ascending, int altIndex, int defaultValue);

private:
	unsigned int currentLayer = 1; // 1-indexed
};
#pragma once
#include "EffectInstance.h"

static EffectBank instance;

class EffectBank {
public:
    static EffectBank& Instance() {
        return instance;
    }

    {{ EFFECT_DEFINITIONS }}
}; 
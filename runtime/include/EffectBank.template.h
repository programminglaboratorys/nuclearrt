#pragma once

#include "EffectInstance.h"

class EffectBank {
public:
    static EffectBank& Instance() {
        return instance;
    }

    {{ EFFECT_DEFINITIONS }}

private:
    static EffectBank instance;
};

inline EffectBank EffectBank::instance;
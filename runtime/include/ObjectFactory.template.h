#pragma once

#include <memory>
#include <unordered_map>
#include "ObjectInstance.h"

#include "QuickBackdrop.h"
#include "Backdrop.h"
#include "Active.h"
#include "StringObject.h"
#include "Score.h"
#include "Lives.h"
#include "Counter.h"
#include "Extension.h"

#include "StaticMovement.h"
#include "MouseMovement.h"
#include "EightDirectionsMovement.h"
#include "BouncingBallMovement.h"
#include "PathMovement.h"
#include "BulletMovement.h"

#include "EffectBank.h"

{{ EXTENSION_INCLUDES }}

class ObjectFactory {
public:
    static ObjectFactory& Instance() {
        return instance;
    }

    ObjectInstance* CreateInstance(unsigned int handle);

    {{ OBJECT_INFO_FUNCTIONS_DEFINITIONS }}

private:
    static ObjectFactory instance;
};

inline ObjectFactory ObjectFactory::instance;
#pragma once

#include "CValue.h"

#include <variant>
#include <string>
#include <vector>

struct EffectParameter {
    std::string Name;
    int Type;
    std::variant<int, float> Value;

    EffectParameter(std::string name, int type, std::variant<int, float> value)
        : Name(name), Type(type), Value(value) {}
};

class EffectInstance {
public:
    EffectInstance(std::string filename, std::vector<EffectParameter> parameters)
        : filename(filename), Parameters(parameters) {}

    std::string filename;
    std::vector<EffectParameter> Parameters;

    void SetParameter(std::string name, const CValue& value)
    {
        for (auto& parameter : Parameters) {
            if (parameter.Name == name) {
                if (parameter.Type == 1)
                {
                    parameter.Value = (float)value.GetDoubleValue();
                }
                else
                {
                    parameter.Value = value.GetIntValue();
                }

                return;
            }
        }
    }
};
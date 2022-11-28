#pragma once

#include "Atom/Core/Core.h"
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/noise.hpp>

namespace Atom
{
    namespace ScriptWrappers
    {
        class Math
        {
        public:
            template<typename Type>
            static Type Abs(const Type& val) { return glm::abs(val); }

            template<typename Type>
            static Type Clamp(const Type& val, const Type& min, const Type& max) { return glm::clamp(val, min, max); }

            template<typename Type>
            static Type Ceil(const Type& val) { return glm::ceil(val); }

            template<typename Type>
            static Type Floor(const Type& val) { return glm::floor(val); }

            template<typename Type>
            static Type Max(const Type& a, const Type& b) { return glm::max(a, b); }

            template<typename Type>
            static Type Min(const Type& a, const Type& b) { return glm::min(a, b); }

            template<typename Type>
            static Type Mix(const Type& a, const Type& b, f32 t) { return glm::mix(a, b, t); }

            template<typename Type>
            static Type Round(const Type& value) { return glm::round(value); }

            template<typename Type>
            static Type RoundEven(const Type& value) { return glm::roundEven(value); }

            template<typename Type>
            static Type Exp(const Type& x) { return glm::exp(x); }

            template<typename Type>
            static Type Exp2(const Type& x) { return glm::exp2(x); }

            template<typename Type>
            static Type Log(const Type& x) { return glm::log(x); }

            template<typename Type>
            static Type Log2(const Type& x) { return glm::log2(x); }

            template<typename Type>
            static Type Pow(const Type& x, const Type& y) { return glm::pow(x, y); }

            template<typename Type>
            static Type Sqrt(const Type& x) { return glm::sqrt(x); }

            static glm::vec3 Cross(const glm::vec3& x, const glm::vec3& y) { return glm::cross(x, y); }

            template<typename Type>
            static f32 Dot(const Type& x, const Type& y) { return glm::dot(x, y); }

            template<typename Type>
            static f32 Distance(const Type& x, const Type& y) { return glm::distance(x, y); }

            template<typename Type>
            static f32 Length(const Type& x) { return glm::length(x); }

            template<typename Type>
            static Type Normalize(const Type& x) { return glm::normalize(x); }

            template<typename Type>
            static Type Reflect(const Type& x, const Type& normal) { return glm::reflect(x, normal); }

            template<typename Type>
            static Type Refract(const Type& x, const Type& normal, f32 eta) { return glm::refract(x, normal, eta); }

            template<typename Type>
            static f32 PerlinNoise(const Type& x) { return glm::perlin(x); }

            template<typename Type>
            static f32 SimplexNoise(const Type& x) { return glm::simplex(x); }

            template<typename Type>
            static Type Acos(const Type& x) { return glm::acos(x); }

            template<typename Type>
            static Type Acosh(const Type& x) { return glm::acosh(x); }

            template<typename Type>
            static Type Asin(const Type& x) { return glm::asin(x); }

            template<typename Type>
            static Type Asinh(const Type& x) { return glm::asinh(x); }

            template<typename Type>
            static Type Atan(const Type& x) { return glm::atan(x); }

            template<typename Type>
            static Type Atanh(const Type& x) { return glm::atanh(x); }

            template<typename Type>
            static Type Sin(const Type& x) { return glm::sin(x); }

            template<typename Type>
            static Type Sinh(const Type& x) { return glm::sinh(x); }

            template<typename Type>
            static Type Cos(const Type& x) { return glm::cos(x); }

            template<typename Type>
            static Type Cosh(const Type& x) { return glm::cosh(x); }

            template<typename Type>
            static Type Tan(const Type& x) { return glm::tan(x); }

            template<typename Type>
            static Type Tanh(const Type& x) { return glm::tanh(x); }

            template<typename Type>
            static Type Degrees(const Type& angle) { return glm::degrees(angle); }

            template<typename Type>
            static Type Radians(const Type& angle) { return glm::radians(angle); }

            template<typename Type>
            static Type Lerp(const Type& a, const Type& b, f32 t) { return glm::lerp(a, b, t); }
        };
    }
}
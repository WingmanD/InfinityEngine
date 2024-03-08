#pragma once

#include "CoreMinimal.h"
#include "Logger.h"
#include "ReflectionTags.h"
#include "Enum.h"
#include "SimpleMath.h"
#include <exception>
#include <wrl/client.h>
#include <chrono>
#include <cstdint>
#include <format>
#include <iostream>
#include <print>
#include <tuple>

typedef DirectX::SimpleMath::Vector2 Vector2;
typedef DirectX::SimpleMath::Vector3 Vector3;
typedef DirectX::SimpleMath::Vector4 Vector4;
typedef DirectX::SimpleMath::Matrix Matrix;
typedef DirectX::SimpleMath::Quaternion Quaternion;
typedef DirectX::SimpleMath::Plane Plane;
typedef DirectX::SimpleMath::Ray Ray;
typedef DirectX::SimpleMath::Color Color;


#ifdef _MSC_VER
#define DEBUG_BREAK() __debugbreak()
#define DEBUG _DEBUG
#endif

#define NUM_ARGS(...) std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value

#define TRACE_LOG(message, ...) std::cout << __FUNCTION__ << ": " << std::format((message), __VA_ARGS__) << std::endl
#define LOG(message, ...) Logger::GetInstance().Log(std::format((message), __VA_ARGS__))

inline double GetTimeInSeconds()
{
    const double milliseconds = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    return milliseconds / 1000.0;
}

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception();
    }
}

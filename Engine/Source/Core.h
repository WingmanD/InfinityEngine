#pragma once

#include "Logger.h"
#include "ReflectionTags.h"
#include "Math/Vector3.h"
#include <exception>
#include <wrl/client.h>
#include <chrono>
#include <cstdint>
#include <format>
#include <iostream>
#include <print>
#include <tuple>

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int64_t int64;

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

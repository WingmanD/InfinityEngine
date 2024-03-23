#pragma once

#include "CoreMinimal.h"
#include "Logger.h"
#include "ReflectionTags.h"
#include "Enum.h"
#include "Math/MathFwd.h"
#include "ObjectShared.h"
#include <exception>
#include <wrl/client.h>
#include <chrono>
#include <format>
#include <iostream>
#include <print>
#include <tuple>

#define NUM_ARGS(...) std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value

#define TRACE_LOG(message, ...) std::cout << __FUNCTION__ << ": " << std::format((message), __VA_ARGS__) << std::endl
#define LOG(message, ...) Logger::GetInstance().Log(std::format((message), __VA_ARGS__))

template <typename T>
SharedObjectPtr<T> NewObject()
{
    return T::StaticType()->template NewObject<T>();
}

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

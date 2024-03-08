#pragma once

#include <concepts>
#include <cstdint>
#include <memory>

struct ObjectDeleter;
class Object;

template <typename Derived, typename Base>
concept IsA = std::derived_from<Derived, Base>;

template <typename T> requires std::is_base_of_v<Object, T>
using UniqueObjectPtr = std::unique_ptr<T, ObjectDeleter>;

template <typename T> requires std::is_base_of_v<Object, T>
using SharedObjectPtr = std::shared_ptr<T>;

template <typename T> requires std::is_base_of_v<Object, T>
using WeakObjectPtr = std::weak_ptr<T>;

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int64_t int64;
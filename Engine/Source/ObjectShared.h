#pragma once

#include <memory>
#include <type_traits>

class Object;
// struct ObjectDeleter;
//
// template <typename T> requires std::is_base_of_v<Object, T>
// using UniqueObjectPtr = std::unique_ptr<T, ObjectDeleter>;

template <typename T>
using SharedObjectPtr = std::shared_ptr<T>;

template <typename T> requires std::is_base_of_v<Object, T>
using WeakObjectPtr = std::weak_ptr<T>;

// template <typename T, typename... Args> requires std::is_base_of_v<Object, T>
// SharedObjectPtr<T> NewObject(Args&&... args)
// {
//     return T::template New<T>(std::forward<Args>(args)...);
// }

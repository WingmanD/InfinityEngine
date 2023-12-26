#pragma once

#include "TypeRegistry.h"
#include <memory>

class Object : public std::enable_shared_from_this<Object>
{
public:
    Object() = default;

    Object(const Object&) = default;
    Object& operator=(const Object&) = default;
    Object(Object&&) noexcept = default;
    Object& operator=(Object&&) noexcept = default;

    static Type* StaticType();

    virtual Type* GetType() const;

    template <typename T> requires std::is_base_of_v<Object, T>
    std::shared_ptr<T> DuplicateStatic() const
    {
        return std::static_pointer_cast<T>(Duplicate());
    }

    template <typename T> requires std::is_base_of_v<Object, T>
    std::shared_ptr<T> Duplicate() const
    {
        return std::dynamic_pointer_cast<T>(Duplicate());
    }

    virtual std::shared_ptr<Object> Duplicate() const;

    /*
     * Duplicate the object at the specified memory location.
     * NOTE: You must allocate memory for the object yourself and keep track of its lifetime.
     */
    virtual Object* DuplicateAt(void* ptr) const;

    virtual ~Object() = default;
};

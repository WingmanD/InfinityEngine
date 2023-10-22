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

    virtual std::shared_ptr<Object> Duplicate() const;

    virtual ~Object() = default;
};

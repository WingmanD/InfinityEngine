#pragma once

#include "TypeRegistry.h"
#include <memory>

class Object : public std::enable_shared_from_this<Object>
{
public:
    static Type* StaticType();

    virtual Type* GetType() const;

    virtual ~Object() = default;
};

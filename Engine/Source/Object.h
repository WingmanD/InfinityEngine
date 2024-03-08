#pragma once

#include "IValidateable.h"
#include "TypeRegistry.h"
#include "Containers/BucketArray.h"
#include <memory>

struct ObjectDeleter
{
    void operator()(Object* object) const;
};

class Object : public IValidateable, public std::enable_shared_from_this<Object>
{
public:
    friend class IValidateable;
    
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

    virtual void OnPropertyChanged(const std::wstring& propertyName);

    virtual ~Object() = default;

    void Destroy();

private:
    inline static BucketArray<Object> ClassBucketArray;

    bool _isValid = false;

    // IValidateable
private:
    void SetValidImplementation(bool value);
    bool IsValidImplementation() const;
};

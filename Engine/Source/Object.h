#pragma once

#include "IValidateable.h"
#include "ISerializeable.h"
#include "MemoryWriter.h"
#include "MemoryReader.h"
#include "Name.h"
#include "TypeRegistry.h"
#include "Containers/BucketArray.h"
#include <memory>

template <typename T>
struct ObjectDeleter
{
    void operator()(T* object) const
    {
        T::ClassBucketArray.Remove(*object);
    }
};

class Object : public IValidateable, public ISerializeable, public std::enable_shared_from_this<Object>
{
    using ClassType = Object;
    
public:
    friend class IValidateable;
    
    friend struct ObjectDeleter<ClassType>;

public:
    Object() = default;

    Object(const Object&) = default;
    Object& operator=(const Object&) = default;
    Object(Object&&) noexcept = default;
    Object& operator=(Object&&) noexcept = default;

    virtual ~Object() override = default;

    static Type* StaticType();

    virtual Type* GetType() const;

    template <typename... Args>
    static SharedObjectPtr<Object> New(Args&&... args)
    {
        return SharedObjectPtr<Object>(ClassBucketArray.Emplace(std::forward<Args>(args)...), ObjectDeleter<ClassType>());
    }

    template <typename... Args>
    static SharedObjectPtr<Object> New(BucketArray<Object>& bucketArray, Args&&... args)
    {
        return SharedObjectPtr<Object>(bucketArray.Emplace(std::forward<Args>(args)...), ObjectDeleter<ClassType>());
    }

    template <typename T> requires std::is_base_of_v<Object, T>
    SharedObjectPtr<T> DuplicateStatic() const
    {
        return std::static_pointer_cast<T>(Duplicate());
    }

    template <typename T> requires std::is_base_of_v<Object, T>
    SharedObjectPtr<T> DuplicateObject() const
    {
        return std::dynamic_pointer_cast<T>(Duplicate());
    }

    virtual SharedObjectPtr<Object> Duplicate() const;
    
    /*
     * Duplicate the object at the specified memory location.
     * NOTE: You must allocate memory for the object yourself and keep track of its lifetime.
     */
    virtual Object* DuplicateAt(void* ptr) const;
    
    virtual void Copy(const Object& other);

    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

    virtual void OnPropertyChanged(Name propertyName);

private:
    inline static BucketArray<Object> ClassBucketArray;

    bool _isValid = false;

    // IValidateable
private:
    void SetValidImplementation(bool value);
    bool IsValidImplementation() const;
};

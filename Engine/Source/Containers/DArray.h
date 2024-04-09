#pragma once

#include "CoreMinimal.h"
#include <cassert>
#include <format>
#include <functional>
#include <memory>

enum EIndexNone : uint64
{
    INDEX_NONE = std::numeric_limits<uint64>::max()
};

template <typename T, std::size_t SSO_SIZE>
class DArrayRandomAccessIterator
{
public:
    using IteratorCategory = std::random_access_iterator_tag;
    using ValueType = T;
    using SizeType = std::size_t;
    using DifferenceType = std::ptrdiff_t;
    using Reference = T&;
    using ConstReference = const T&;
    using RValueReference = T&&;
    using Pointer = T*;
    using ConstPointer = const T*;

public:
    DArrayRandomAccessIterator(SizeType index, Pointer ssoStartPtr, Pointer dataStartPtr) :
        _ssoStartPtr(ssoStartPtr), _dataStartPtr(dataStartPtr), _index(index)
    {
        UpdatePtr();
    }

    DArrayRandomAccessIterator(const DArrayRandomAccessIterator& other) = default;
    DArrayRandomAccessIterator(DArrayRandomAccessIterator&& other) noexcept = default;

    ~DArrayRandomAccessIterator() = default;

    DArrayRandomAccessIterator& operator=(const DArrayRandomAccessIterator& other) = default;
    DArrayRandomAccessIterator& operator=(DArrayRandomAccessIterator&& other) noexcept = default;

    Pointer GetPtr() const
    {
        return _currentPtr;
    }

    Pointer GetPtrConst() const
    {
        return _currentPtr;
    }

    [[nodiscard]] SizeType GetIndex() const
    {
        return _index;
    }

    auto operator<=>(const DArrayRandomAccessIterator& other) const
    {
        return _index <=> other._index;
    }

    bool operator==(const DArrayRandomAccessIterator& other) const
    {
        return _index == other._index;
    }

    DArrayRandomAccessIterator& operator++()
    {
        ++_index;
        UpdatePtr();

        return *this;
    }

    DArrayRandomAccessIterator operator++(int)
    {
        DArrayRandomAccessIterator tmp(*this);
        ++tmp._index;
        tmp.UpdatePtr();

        return tmp;
    }

    DArrayRandomAccessIterator& operator--()
    {
        --_index;
        UpdatePtr();

        return *this;
    }

    DArrayRandomAccessIterator operator--(int)
    {
        DArrayRandomAccessIterator tmp(*this);
        --tmp._index;
        tmp.UpdatePtr();

        return tmp;
    }

    DArrayRandomAccessIterator& operator+=(DifferenceType n)
    {
        _index += n;
        UpdatePtr();

        return *this;
    }

    DArrayRandomAccessIterator& operator-=(DifferenceType n)
    {
        _index -= n;
        UpdatePtr();

        return *this;
    }

    DArrayRandomAccessIterator operator+(DifferenceType n) const
    {
        DArrayRandomAccessIterator tmp(*this);
        tmp._index += n;
        tmp.UpdatePtr();

        return tmp;
    }

    DArrayRandomAccessIterator operator-(DifferenceType n) const
    {
        DArrayRandomAccessIterator tmp(*this);
        tmp._index -= n;
        tmp.UpdatePtr();

        return tmp;
    }

    DifferenceType operator-(const DArrayRandomAccessIterator& other) const
    {
        return _index - other._index;
    }

    Reference operator*()
    {
        return *_currentPtr;
    }

    ConstReference operator*() const
    {
        return *_currentPtr;
    }

    Pointer operator->()
    {
        return _currentPtr;
    }

private:
    Pointer _currentPtr;
    Pointer _ssoStartPtr;
    Pointer _dataStartPtr;
    SizeType _index;

private:
    void UpdatePtr()
    {
        if (_index < SSO_SIZE)
        {
            _currentPtr = _ssoStartPtr + _index;
        }
        else
        {
            _currentPtr = _dataStartPtr + (_index - SSO_SIZE);
        }
    }
};

template <typename T>
class DArrayRandomAccessIterator<T, 0>
{
public:
    using IteratorCategory = std::random_access_iterator_tag;
    using ValueType = T;
    using DifferenceType = std::ptrdiff_t;
    using Reference = T&;
    using ConstReference = const T&;
    using RValueReference = T&&;
    using Pointer = T*;
    using ConstPointer = const T*;

public:
    explicit DArrayRandomAccessIterator(Pointer ptr) : _ptr(ptr)
    {
    }

    DArrayRandomAccessIterator(const DArrayRandomAccessIterator& other) = default;
    DArrayRandomAccessIterator(DArrayRandomAccessIterator&& other) noexcept = default;

    ~DArrayRandomAccessIterator() = default;

    DArrayRandomAccessIterator& operator=(const DArrayRandomAccessIterator& other) = default;
    DArrayRandomAccessIterator& operator=(DArrayRandomAccessIterator&& other) noexcept = default;

    DArrayRandomAccessIterator& operator=(Pointer ptr)
    {
        _ptr = ptr;
        return *this;
    }

    Pointer GetPtr() const
    {
        return _ptr;
    }

    ConstPointer GetConstPtr() const
    {
        return _ptr;
    }

    auto operator<=>(const DArrayRandomAccessIterator& other) const
    {
        return _ptr <=> other._ptr;
    }

    bool operator==(const DArrayRandomAccessIterator&) const = default;

    DArrayRandomAccessIterator& operator++()
    {
        ++_ptr;

        return *this;
    }

    DArrayRandomAccessIterator operator++(int)
    {
        DArrayRandomAccessIterator tmp(*this);
        ++tmp._ptr;

        return tmp;
    }

    DArrayRandomAccessIterator& operator--()
    {
        --_ptr;

        return *this;
    }

    DArrayRandomAccessIterator operator--(int)
    {
        DArrayRandomAccessIterator tmp(*this);
        --tmp._ptr;

        return tmp;
    }

    DArrayRandomAccessIterator& operator+=(DifferenceType n)
    {
        _ptr += n;

        return *this;
    }

    DArrayRandomAccessIterator& operator-=(DifferenceType n)
    {
        _ptr -= n;

        return *this;
    }

    DArrayRandomAccessIterator operator+(DifferenceType n) const
    {
        return DArrayRandomAccessIterator(_ptr + n);
    }

    DArrayRandomAccessIterator operator-(DifferenceType n) const
    {
        return DArrayRandomAccessIterator(_ptr - n);
    }

    DifferenceType operator-(const DArrayRandomAccessIterator& other) const
    {
        return _ptr - other._ptr;
    }

    Reference operator*()
    {
        return *_ptr;
    }

    ConstReference operator*() const
    {
        return *_ptr;
    }

    Pointer operator->()
    {
        return _ptr;
    }

private:
    Pointer _ptr;
};

template <typename T, std::size_t SSO_SIZE = 0, typename Allocator = std::allocator<T>>
class DArray
{
public:
    using type = T;

    using ValueType = T;
    using AllocatorType = Allocator;
    using SizeType = std::size_t;
    using DifferenceType = std::ptrdiff_t;
    using Reference = T&;
    using ConstReference = const T&;
    using RValueReference = T&&;
    using Pointer = T*;

    using Iterator = DArrayRandomAccessIterator<T, SSO_SIZE>;
    using ConstIterator = DArrayRandomAccessIterator<const T, SSO_SIZE>;
    using ReverseIterator = std::reverse_iterator<Iterator>;
    using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

public:
    static DArray Empty;

public:
    DArray() = default;

    explicit DArray(SizeType count, ConstReference value = T(), const AllocatorType& allocator = AllocatorType())
        : _allocator(allocator), _count(count)
    {
        if constexpr (SSO_SIZE > 0)
        {
            if (count > SSO_SIZE)
            {
                _data = _allocator.allocate(count - SSO_SIZE);
                _capacity = count;

                for (SizeType i = 0; i < SSO_SIZE; ++i)
                {
                    GetSSOData()[i] = value;
                }

                for (SizeType i = 0; i < count - SSO_SIZE; ++i)
                {
                    _data[i] = value;
                }
            }
            else
            {
                for (SizeType i = 0; i < count; ++i)
                {
                    GetSSOData()[i] = value;
                }
            }
        }
        else
        {
            _data = _allocator.allocate(count);

            for (SizeType i = 0; i < count; ++i)
            {
                _data[i] = value;
            }
        }
    }

    template <typename InputIterator>
    DArray(const InputIterator& start, const InputIterator& end)
    {
        _allocator = AllocatorType();

        for (auto it = start; it != end; ++it)
        {
            Add(*it);
        }
    }

    template <typename ContainerType>
    explicit DArray(const ContainerType& container) : DArray(container.begin(), container.end())
    {
    }

    DArray(const std::initializer_list<T>& initializerList) : DArray(initializerList.begin(), initializerList.end())
    {
    }

    DArray(const DArray& other): _allocator(other._allocator), _count(other._count), _capacity(other._capacity)
    {
        if constexpr (SSO_SIZE > 0)
        {
            if (_capacity > SSO_SIZE)
            {
                for (auto i = 0; i < std::min(_count, SSO_SIZE); ++i)
                {
                    std::construct_at(&GetSSOData()[i], other.GetSSODataConst()[i]);
                }

                _data = _allocator.allocate(_capacity - SSO_SIZE);
                for (auto i = 0; i < _capacity - SSO_SIZE; ++i)
                {
                    std::construct_at(&_data[i], other._data[i]);
                }
            }
            else
            {
                for (auto i = 0; i < other._count; ++i)
                {
                    std::construct_at(&GetSSOData()[i], other.GetSSODataConst()[i]);
                }
            }
        }
        else
        {
            _data = _allocator.allocate(_capacity);
            for (SizeType i = 0; i < _count; ++i)
            {
                _data[i] = other._data[i];
            }
        }
    }

    DArray(DArray&& other) noexcept
    {
        if constexpr (SSO_SIZE > 0)
        {
            for (auto i = 0; i < other._count; ++i)
            {
                std::construct_at(&GetSSOData()[i], std::move(other.GetSSOData()[i]));
            }
        }

        _data = other._data;
        other._data = nullptr;

        _count = other._count;
        _capacity = other._capacity;
    }

    ~DArray()
    {
        if constexpr (SSO_SIZE > 0)
        {
            for (auto i = 0; i < std::min(_count, SSO_SIZE); ++i)
            {
                GetSSOData()[i].~T();
            }

            if (_capacity > SSO_SIZE)
            {
                for (auto i = 0; i < _count - SSO_SIZE; ++i)
                {
                    _data[i].~T();
                }

                _allocator.deallocate(_data, _capacity - SSO_SIZE);
            }
        }
        else
        {
            for (SizeType i = 0; i < _count; ++i)
            {
                _data[i].~T();
            }

            _allocator.deallocate(_data, _capacity);
        }
    }

    DArray& operator=(const DArray& other)
    {
        if (this == &other)
        {
            return *this;
        }

        Clear();

        if constexpr (SSO_SIZE > 0)
        {
            if (other._capacity > SSO_SIZE)
            {
                _data = _allocator.allocate(other._capacity - SSO_SIZE);
                _capacity = other._capacity;

                for (auto i = 0; i < std::min(other._count, SSO_SIZE); ++i)
                {
                    std::construct_at(&GetSSOData()[i], other.GetSSODataConst()[i]);
                }

                for (auto i = 0; i < other._capacity - SSO_SIZE; ++i)
                {
                    std::construct_at(&_data[i], other._data[i]);
                }
            }
            else
            {
                _capacity = SSO_SIZE;
                for (auto i = 0; i < other._count; ++i)
                {
                    std::construct_at(&GetSSOData()[i], other.GetSSODataConst()[i]);
                }
            }
        }
        else
        {
            _data = _allocator.allocate(other._capacity);
            for (SizeType i = 0; i < other._count; ++i)
            {
                _data[i] = other._data[i];
            }
        }

        return *this;
    }

    DArray& operator=(DArray&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        Clear();

        if constexpr (SSO_SIZE > 0)
        {
            for (auto i = 0; i < other._count; ++i)
            {
                std::construct_at(&GetSSOData()[i], std::move(other.GetSSOData()[i]));
            }
        }

        _data = other._data;
        other._data = nullptr;

        _count = other._count;
        _capacity = other._capacity;

        return *this;
    }

    bool operator==(const DArray& other) const
    {
        if (_count != other._count)
        {
            return false;
        }

        if constexpr (SSO_SIZE > 0)
        {
            for (auto i = 0; i < _count; ++i)
            {
                if (GetElementConst(i) != other.GetElementConst(i))
                {
                    return false;
                }
            }
        }
        else
        {
            for (SizeType i = 0; i < _count; ++i)
            {
                if (_data[i] != other._data[i])
                {
                    return false;
                }
            }
        }

        return true;
    }
    
    template <std::size_t N = SSO_SIZE, std::enable_if_t<N == 0, size_t> = 0>
    Pointer GetData()
    {
        return _data;
    }

    Reference operator[](SizeType index)
    {
        return GetElement(index);
    }

    ConstReference operator[](SizeType index) const
    {
        return GetElementConst(index);
    }

    Reference Front()
    {
        return GetElement(0);
    }

    ConstReference Front() const
    {
        return GetElementConst(0);
    }

    Reference Back()
    {
        return GetElement(_count - 1);
    }

    ConstReference Back() const
    {
        return GetElementConst(_count - 1);
    }

    Iterator begin()
    {
        if constexpr (SSO_SIZE > 0)
        {
            return Iterator(0, GetSSOData(), _data);
        }
        else
        {
            return Iterator(_data);
        }
    }

    ConstIterator begin() const
    {
        if constexpr (SSO_SIZE > 0)
        {
            return ConstIterator(0, GetSSODataConst(), _data);
        }
        else
        {
            return ConstIterator(_data);
        }
    }

    ConstIterator cbegin() const
    {
        return begin();
    }

    Iterator end()
    {
        if constexpr (SSO_SIZE > 0)
        {
            return Iterator(_count, GetSSOData(), _data);
        }
        else
        {
            return Iterator(_data + _count);
        }
    }

    ConstIterator end() const
    {
        if constexpr (SSO_SIZE > 0)
        {
            return ConstIterator(_count, GetSSODataConst(), _data);
        }
        else
        {
            return ConstIterator(_data + _count);
        }
    }

    ConstIterator cend() const
    {
        return end();
    }

    ReverseIterator rbegin() const
    {
        return std::make_reverse_iterator(end());
    }

    ConstReverseIterator crbegin() const
    {
        return std::make_reverse_iterator(cend());
    }

    ReverseIterator rend() const
    {
        return std::make_reverse_iterator(begin());
    }

    ConstReverseIterator crend() const
    {
        return std::make_reverse_iterator(cbegin());
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return _count == 0;
    }

    [[nodiscard]] SizeType Count() const
    {
        return _count;
    }

    [[nodiscard]] SizeType Capacity() const
    {
        return _capacity;
    }

    void Reserve(SizeType newCapacity)
    {
        if (newCapacity <= _capacity)
        {
            return;
        }

        const SizeType newCapacityActual = std::max(newCapacity, _capacity * 2);

        if constexpr (SSO_SIZE > 0)
        {
            Pointer newData = _allocator.allocate(newCapacityActual - SSO_SIZE);

            if (_capacity > SSO_SIZE && _data != nullptr)
            {
                const SizeType count = _count - SSO_SIZE;

                if constexpr (std::is_move_constructible_v<T>)
                {
                    for (auto i = 0; i < count; ++i)
                    {
                        std::construct_at(&newData[i], std::move(_data[i]));
                        _data[i].~T();
                    }
                }
                else
                {
                    for (auto i = 0; i < count; ++i)
                    {
                        std::construct_at(&newData[i], _data[i]);
                        _data[i].~T();
                    }
                }

                _allocator.deallocate(_data, _capacity - SSO_SIZE);
            }

            _data = newData;
        }
        else
        {
            Pointer newData = _allocator.allocate(newCapacityActual);

            if constexpr (std::is_move_constructible_v<T>)
            {
                for (auto i = 0; i < _count; ++i)
                {
                    std::construct_at(&newData[i], std::move(_data[i]));
                    _data[i].~T();
                }
            }
            else
            {
                for (auto i = 0; i < _count; ++i)
                {
                    std::construct_at(&newData[i], _data[i]);
                    _data[i].~T();
                }
            }

            _allocator.deallocate(_data, _capacity);
            _data = newData;
        }

        _capacity = newCapacityActual;
    }

    void ShrinkToFit()
    {
        if (_count == _capacity)
        {
            return;
        }

        if constexpr (SSO_SIZE > 0)
        {
            if (_capacity <= SSO_SIZE)
            {
                return;
            }

            if (_count <= SSO_SIZE)
            {
                _allocator.deallocate(_data, _capacity - SSO_SIZE);
                _data = nullptr;
                _capacity = SSO_SIZE;
            }
            else
            {
                Pointer newData = _allocator.allocate(_count);

                if constexpr (std::is_move_constructible_v<T>)
                {
                    for (auto i = 0; i < _count; ++i)
                    {
                        std::construct_at(&newData[i], std::move(_data[i]));
                    }
                }
                else
                {
                    for (auto i = 0; i < _count; ++i)
                    {
                        std::construct_at(&newData[i], _data[i]);
                    }
                }

                _allocator.deallocate(_data, _capacity - SSO_SIZE);
                _data = newData;
                _capacity = _count;
            }
        }
        else
        {
            Pointer newData = _allocator.allocate(_count);

            for (auto i = 0; i < _count; ++i)
            {
                if constexpr (std::is_move_constructible_v<T>)
                {
                    std::construct_at(&newData[i], std::move(_data[i]));
                }
                else
                {
                    std::construct_at(&newData[i], _data[i]);
                }

                _data[i].~T();
            }

            _allocator.deallocate(_data, _capacity - SSO_SIZE);
            _data = newData;
            _capacity = _count;
        }
    }

    void Clear()
    {
        if constexpr (SSO_SIZE > 0)
        {
            if (_capacity > SSO_SIZE)
            {
                for (auto i = 0; i < SSO_SIZE; ++i)
                {
                    GetSSOData()[i].~T();
                }

                for (auto i = 0; i < _capacity - SSO_SIZE; ++i)
                {
                    _data[i].~T();
                }
            }
            else
            {
                for (auto i = 0; i < _count; ++i)
                {
                    GetSSOData()[i].~T();
                }
            }
        }
        else
        {
            for (SizeType i = 0; i < _count; ++i)
            {
                _data[i].~T();
            }
        }

        _count = 0;
    }

    Iterator Insert(Iterator pos, ConstReference value)
    {
        return InsertImplementation(pos, std::forward<ConstReference>(value));
    }

    Iterator Insert(ConstIterator pos, ConstReference value)
    {
        return InsertImplementation(pos, std::forward<ConstReference>(value));
    }

    Iterator Insert(Iterator pos, RValueReference value)
    {
        return InsertImplementation(pos, std::forward<RValueReference>(value));
    }

    Iterator InsertAt(SizeType index, ConstReference value)
    {
        return Insert(begin() + index, 1, value);
    }

    Reference Add(const T& value)
    {
        Reserve(_count + 1);

        std::construct_at(&GetElement(_count), value);

        ++_count;

        return Back();
    }

    Reference Add(T&& value)
    {
        Reserve(_count + 1);

        std::construct_at(&GetElement(_count), std::move(value));

        ++_count;

        return Back();
    }

    template <typename... Args>
    Reference Emplace(Args&&... args)
    {
        Reserve(_count + 1);

        std::construct_at(&GetElement(_count), std::forward<Args>(args)...);

        ++_count;

        return Back();
    }

    void PopBack()
    {
        assert(_count > 0);

        if constexpr (SSO_SIZE > 0)
        {
            if (_count <= SSO_SIZE)
            {
                GetSSOData()[_count - 1].~T();
            }
            else
            {
                _data[_count - SSO_SIZE - 1].~T();
            }
        }
        else
        {
            _data[_count - 1].~T();
        }

        --_count;
    }

    Iterator Find(ConstReference value)
    {
        if constexpr (SSO_SIZE > 0)
        {
            for (auto it = begin(); it != end(); ++it)
            {
                if (*it == value)
                {
                    return it;
                }
            }
        }
        else
        {
            for (auto i = 0; i < _count; ++i)
            {
                if (_data[i] == value)
                {
                    return begin() + i;
                }
            }
        }

        return end();
    }

    Iterator FindIf(const std::function<bool(ConstReference)>& predicate)
    {
        if constexpr (SSO_SIZE > 0)
        {
            for (auto it = begin(); it != end(); ++it)
            {
                if (predicate(*it))
                {
                    return it;
                }
            }
        }
        else
        {
            for (auto i = 0; i < _count; ++i)
            {
                if (predicate(_data[i]))
                {
                    return begin() + i;
                }
            }
        }

        return end();
    }

    SizeType IndexOf(ConstReference value)
    {
        const SizeType index = Find(value) - begin();
        if (index < _count)
        {
            return index;
        }

        return INDEX_NONE;
    }

    bool Contains(ConstReference value)
    {
        return Find(value) != end();
    }

    bool ContainsIf(const std::function<bool(ConstReference)>& predicate)
    {
        return FindIf(predicate) != end();
    }

    void RemoveAt(SizeType index)
    {
        ShiftElementsLeft(index);
        --_count;
    }

    bool RemoveAt(Iterator pos)
    {
        if (pos == end())
        {
            return false;
        }

        RemoveAt(pos - begin());
        return true;
    }

    void RemoveAtSwap(Iterator pos)
    {
        RemoveAtSwap(pos - begin());
    }

    void RemoveAtSwap(SizeType index)
    {
        if (_count > 1)
        {
            Reference element = GetElement(index);
            element.~T();

            if constexpr (std::is_move_constructible_v<T>)
            {
                std::construct_at(&element, std::move(Back()));
            }
            else
            {
                std::construct_at(&GetElement(index), Back());
            }
        }

        PopBack();
    }

    bool Remove(ConstReference value)
    {
        auto it = Find(value);
        if (it == end())
        {
            return false;
        }

        RemoveAt(it);
        return true;
    }

    bool RemoveSwap(ConstReference value)
    {
        auto it = Find(value);
        if (it == end())
        {
            return false;
        }

        RemoveAtSwap(it);
        return true;
    }

    bool RemoveIf(const std::function<bool(ConstReference)>& predicate)
    {
        auto it = FindIf(predicate);
        if (it == end())
        {
            return false;
        }

        RemoveAt(it);
        return true;
    }

    bool RemoveIfSwap(const std::function<bool(ConstReference)>& predicate)
    {
        auto it = FindIf(predicate);
        if (it == end())
        {
            return false;
        }

        RemoveAtSwap(it);
        return true;
    }

    SizeType RemoveAll(ConstReference value)
    {
        SizeType removedCount = 0;

        if constexpr (SSO_SIZE > 0)
        {
            for (int64 i = _count; i >= 0; i--)
            {
                if (GetElementConst(i) == value)
                {
                    RemoveAt(i);
                    ++removedCount;
                }
            }
        }
        else
        {
            for (int64 i = _count; i >= 0; i--)
            {
                if (_data[i] == value)
                {
                    RemoveAt(i);
                    ++removedCount;
                }
            }
        }

        return removedCount;
    }

    SizeType RemoveAllSwap(ConstReference value)
    {
        SizeType removedCount = 0;

        if constexpr (SSO_SIZE > 0)
        {
            for (int64 i = _count; i >= 0; i--)
            {
                if (GetElementConst(i) == value)
                {
                    RemoveAtSwap(i);
                    ++removedCount;
                }
            }
        }
        else
        {
            for (int64 i = _count; i >= 0; i--)
            {
                if (_data[i] == value)
                {
                    RemoveAtSwap(i);
                    ++removedCount;
                }
            }
        }

        return removedCount;
    }

    SizeType RemoveAllIf(const std::function<bool(ConstReference)>& predicate)
    {
        SizeType removedCount = 0;

        if constexpr (SSO_SIZE > 0)
        {
            for (int64 i = _count; i >= 0; i--)
            {
                if (predicate(GetElementConst(i)))
                {
                    RemoveAt(i);
                    ++removedCount;
                }
            }
        }
        else
        {
            for (int64 i = _count; i >= 0; i--)
            {
                if (predicate(_data[i]))
                {
                    RemoveAt(i);
                    ++removedCount;
                }
            }
        }

        return removedCount;
    }

    SizeType RemoveAllIfSwap(const std::function<bool(ConstReference)>& predicate)
    {
        SizeType removedCount = 0;

        if constexpr (SSO_SIZE > 0)
        {
            for (int64 i = _count; i >= 0; i--)
            {
                if (predicate(GetElementConst(i)))
                {
                    RemoveAtSwap(i);
                    ++removedCount;
                }
            }
        }
        else
        {
            for (int64 i = _count; i >= 0; i--)
            {
                if (predicate(_data[i]))
                {
                    RemoveAtSwap(i);
                    ++removedCount;
                }
            }
        }

        return removedCount;
    }

    void Resize(SizeType newSize)
    {
        Resize(newSize, T());
    }

    void Resize(SizeType newSize, const ValueType& value)
    {
        Reserve(newSize);

        for (auto i = _count; i < newSize - _count; ++i)
        {
            std::construct_at(&GetElement(i), value);
        }
    }

private:
    AllocatorType _allocator;

    using SSOBufferType = std::conditional_t<SSO_SIZE == 0, std::monostate, std::byte[sizeof(T) * SSO_SIZE]>;

    alignas(T) SSOBufferType _ssoBuffer{};
    Pointer _data = nullptr;

    SizeType _count = 0;
    SizeType _capacity = SSO_SIZE;

private:
    Reference GetElement(SizeType index)
    {
        return const_cast<Reference>(GetElementConst(index));
    }

    ConstReference GetElementConst(SizeType index) const
    {
        if constexpr (SSO_SIZE > 0)
        {
            if (index < SSO_SIZE)
            {
                return GetSSODataConst()[index];
            }

            return _data[index - SSO_SIZE];
        }
        else
        {
            return _data[index];
        }
    }

    T* GetSSOData()
    {
        return reinterpret_cast<T*>(_ssoBuffer);
    }

    const T* GetSSODataConst() const
    {
        return reinterpret_cast<const T*>(_ssoBuffer);
    }

    void ShiftElementsLeft(SizeType index)
    {
        if constexpr (SSO_SIZE > 0)
        {
            const int64 numData = _count - SSO_SIZE;
            SizeType dataStartIndex = index - SSO_SIZE;

            if (index < SSO_SIZE)
            {
                dataStartIndex = 0;

                for (SizeType i = index; i < SSO_SIZE - 1; ++i)
                {
                    const SizeType next = i + 1;
                    GetSSOData()[i].~T();
                    if constexpr (std::is_move_constructible_v<T>)
                    {
                        std::construct_at(&GetSSOData()[i], std::move(GetSSOData()[next]));
                    }
                    else
                    {
                        std::construct_at(&GetSSOData()[i], GetSSOData()[next]);
                    }
                }

                const SizeType lastSSO = SSO_SIZE - 1;
                GetSSOData()[lastSSO].~T();

                if (numData > 0)
                {
                    if constexpr (std::is_move_constructible_v<T>)
                    {
                        std::construct_at(&GetSSOData()[lastSSO], std::move(_data[0]));
                    }
                    else
                    {
                        std::construct_at(&GetSSOData()[lastSSO], _data[0]);
                    }
                }
            }

            for (int64 i = dataStartIndex; i < numData - 1; ++i)
            {
                _data[i].~T();
                if constexpr (std::is_move_constructible_v<T>)
                {
                    std::construct_at(&_data[i], std::move(_data[i + 1]));
                }
                else
                {
                    std::construct_at(&_data[i], _data[i + 1]);
                }
            }
            _data[numData - 1].~T();
        }
        else
        {
            for (SizeType i = index; i < _count - 1; ++i)
            {
                _data[i].~T();
                if constexpr (std::is_move_constructible_v<T>)
                {
                    std::construct_at(&_data[i], std::move(_data[i + 1]));
                }
                else
                {
                    std::construct_at(&_data[i], _data[i + 1]);
                }
            }

            _data[_count - 1].~T();
        }
    }

    void ShiftElementsRight(SizeType index)
    {
        if constexpr (SSO_SIZE > 0)
        {
            const int64 numData = _count - SSO_SIZE;
            const int64 dataStartIndex = index - SSO_SIZE;

            if (index > SSO_SIZE)
            {
                for (int64 i = numData - 1; i > dataStartIndex; --i)
                {
                    _data[i].~T();
                    if constexpr (std::is_move_constructible_v<T>)
                    {
                        std::construct_at(&_data[i], std::move(_data[i - 1]));
                    }
                    else
                    {
                        std::construct_at(&_data[i], _data[i - 1]);
                    }
                }

                _data[dataStartIndex].~T();
            }
            else
            {
                if (numData > 0)
                {
                    if constexpr (std::is_move_constructible_v<T>)
                    {
                        std::construct_at(&_data[0], std::move(GetSSOData()[SSO_SIZE - 1]));
                    }
                    else
                    {
                        std::construct_at(&_data[0], GetSSOData()[SSO_SIZE - 1]);
                    }
                }


                if constexpr (std::is_move_constructible_v<T>)
                {
                    std::construct_at(&GetSSOData()[_count], std::move(GetSSOData()[_count - 1]));
                }
                else
                {
                    std::construct_at(&GetSSOData()[_count], GetSSOData()[_count - 1]);
                }

                for (SizeType i = _count - 1; i >= index + 1; --i)
                {
                    GetSSOData()[i].~T();

                    const SizeType previous = i - 1;
                    if constexpr (std::is_move_constructible_v<T>)
                    {
                        std::construct_at(&GetSSOData()[i], std::move(GetSSOData()[previous]));
                    }
                    else
                    {
                        std::construct_at(&GetSSOData()[i], GetSSOData()[previous]);
                    }
                }

                GetSSOData()[index].~T();
            }
        }
        else
        {
            if constexpr (std::is_move_constructible_v<T>)
            {
                std::construct_at(&_data[_count], std::move(_data[_count - 1]));
            }
            else
            {
                std::construct_at(&_data[_count], _data[_count - 1]);
            }

            for (SizeType i = _count - 1; i >= index + 1; --i)
            {
                const SizeType previous = i - 1;
                _data[i].~T();
                if constexpr (std::is_move_constructible_v<T>)
                {
                    std::construct_at(&_data[i], std::move(_data[previous]));
                }
                else
                {
                    std::construct_at(&_data[i], _data[previous]);
                }
            }

            _data[index].~T();
        }
    }

    template <typename IteratorType>
    Iterator InsertImplementation(IteratorType pos, RValueReference value)
    {
        SizeType index;
        if constexpr (SSO_SIZE > 0)
        {
            index = pos.GetIndex();
        }
        else
        {
            index = pos.GetPtr() - _data;
        }

        Reserve(_count + 1);
        ShiftElementsRight(index);

        Iterator newPos = begin() + index;
        std::construct_at(newPos.GetPtr(), std::forward<RValueReference>(value));

        ++_count;
        return newPos;
    }
};

﻿#pragma once

#include "DropdownMenu.h"
#include "DropdownTextChoice.h"
#include "TypePicker.reflection.h"

REFLECTED()
class DropdownTypeChoice : public DropdownTextChoice
{
    GENERATED()

public:
    DropdownTypeChoice() = default;

    DropdownTypeChoice(const DropdownTypeChoice& other);
    DropdownTypeChoice& operator=(const DropdownTypeChoice& other);

    bool InitializeFromType(Type* type);

    Type* GetSelectedType() const;

public:
    bool Initialize() override;

private:
    Type* _type = nullptr;
};

REFLECTED()
class TypePicker : public DropdownMenu
{
    GENERATED()

public:
    TypePicker() = default;

    static SharedObjectPtr<TypePicker> CreateForType(Type* baseType, const std::function<bool(const Type*)>& callback = nullptr);

    void InitializeFromType(Type* baseType, const std::function<bool(const Type*)>& callback = nullptr);

    Type* GetBaseType() const;

    Type* GetSelectedType() const;
    void SetSelectedType(const Type* type);

private:
    Type* _baseType = nullptr;
};

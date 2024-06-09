#pragma once

#include "DropdownMenu.h"
#include "DropdownTextChoice.h"
#include "EnumDropdown.reflection.h"

REFLECTED()
class DropdownEnumChoice : public DropdownTextChoice
{
    GENERATED()

public:
    DropdownEnumChoice() = default;

    DropdownEnumChoice(const DropdownEnumChoice& other);
    DropdownEnumChoice& operator=(const DropdownEnumChoice& other);

    bool InitializeFromEnum(const Enum* enumType, uint32 value);

    const std::string& GetSelectedEnumName() const;
    uint32 GetSelectedEnumValue() const;

    // DropdownTextChoice
public:
    bool Initialize() override;

private:
    const Enum* _enumType = nullptr;
    uint32 _value = 0;

private:
    bool InitializeFromEnumInternal(const Enum* enumType, uint32 value);
};

REFLECTED()
class EnumDropdown : public DropdownMenu
{
    GENERATED()

public:
    EnumDropdown() = default;

    static SharedObjectPtr<EnumDropdown> CreateForEnum(const Enum* enumType);

    void InitializeFromEnum(const Enum* enumType);

    const Enum* GetEnumType() const;

    const std::string& GetSelectedEnumName() const;
    uint32 GetSelectedEnumValue() const;

    void SetSelectedEnumValue(uint32 value);

private:
    const Enum* _enumType = nullptr;
};

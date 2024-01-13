#pragma once

#include "TextBox.h"
#include "Delegate.h"
#include "EditableTextBox.reflection.h"

class Caret;

REFLECTED()

class EditableTextBox : public TextBox
{
    GENERATED()

public:
    Delegate<const std::wstring&> OnValueChanged;

public:
    EditableTextBox() = default;

    EditableTextBox(const EditableTextBox& other) = default;
    EditableTextBox& operator=(const EditableTextBox& other) = default;

    void SetMinLength(int32 length);
    int32 GetMinLength() const;

    void SetMaxLength(int32 length);
    int32 GetMaxLength() const;

    void SetCursorPosition(int32 position);
    int32 GetCursorPosition() const;

    // Widget
public:
    virtual bool Initialize() override;

    // TextBox
protected:
    void OnTextChanged() override;

    // Widget
protected:
    void UpdateDesiredSizeInternal() override;
    void RebuildLayoutInternal() override;
    
    virtual void OnFocusChanged(bool focused) override;

private:
    PROPERTY(Edit, DisplayName = "Min Lenght")
    int32 _minLength = 5;

    PROPERTY(Edit, DisplayName = "Max Lenght")
    int32 _maxLength = 100;

    int32 _cursorPosition = 0;

    DelegateHandle _onKeyDownHandle;

    std::weak_ptr<Caret> _caret;

private:
    void OnCursorPositionChanged() const;
};

#pragma once

#include "TextBox.h"
#include "EditableTextBox.reflection.h"
#include "Delegate.h"

class Caret;

REFLECTED()
class EditableTextBox : public TextBox
{
    GENERATED()

public:
    EditableTextBox() = default;

    EditableTextBox(const EditableTextBox& other) = default;
    EditableTextBox& operator=(const EditableTextBox& other) = default;

    void SetMinLenght(int32 lenght);
    int32 GetMinLenght() const;

    void SetMaxLenght(int32 lenght);
    int32 GetMaxLenght() const;

    void SetCursorPosition(int32 position);
    int32 GetCursorPosition() const;

    // Widget
public:
    virtual bool Initialize() override;

    // TextBox
protected:
    virtual void OnTextChanged() override;
    
    // Widget
protected:
    virtual void OnFocusChanged(bool focused) override;

private:
    PROPERTY(EditableInEditor, DisplayName = "Min Lenght")
    int32 _minLenght = 5;
    
    PROPERTY(EditableInEditor, DisplayName = "Max Lenght")
    int32 _maxLenght = 100;
    
    int32 _cursorPosition = 0;

    DelegateHandle _onKeyDownHandle;

    std::weak_ptr<Caret> _caret;

private:
    void OnCursorPositionChanged() const;
};

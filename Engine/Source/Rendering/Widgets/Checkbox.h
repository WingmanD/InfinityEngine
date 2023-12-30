#pragma once

#include "Widget.h"
#include "Rendering/Delegate.h"
#include "Checkbox.reflection.h"

REFLECTED()
class Checkbox : public Widget
{
    GENERATED()

public:
    Delegate<bool> OnCheckedChanged;

public:
    Checkbox() = default;
    Checkbox(const Checkbox& other) = default;
    Checkbox& operator=(const Checkbox& other) = default;

    void SetChecked(bool value);
    bool IsChecked() const;

    // Widget
public:
    virtual bool Initialize() override;

protected:
    virtual void OnReleasedInternal() override;
    
private:
    bool _isChecked = false;

private:
    void OnCheckedChangedInternal();
};

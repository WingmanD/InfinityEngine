#pragma once

#include "Widget.h"
#include "MulticastDelegate.h"
#include "Checkbox.reflection.h"

REFLECTED()
class Checkbox : public Widget
{
    GENERATED()

public:
    MulticastDelegate<bool> OnCheckedChanged;

public:
    Checkbox();
    Checkbox(const Checkbox& other) = default;
    Checkbox& operator=(const Checkbox& other) = default;

    void SetChecked(bool value);
    bool IsChecked() const;

    // Widget
public:
    virtual bool Initialize() override;

protected:
    void UpdateDesiredSizeInternal() override;
    virtual bool OnReleasedInternal() override;
    
private:
    bool _isChecked = false;

private:
    void OnCheckedChangedInternal();
};

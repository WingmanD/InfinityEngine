#include "Checkbox.h"
#include "MaterialParameterTypes.h"
#include "Rendering/Material.h"

Checkbox::Checkbox()
{
    EnableInputCompatibility(EWidgetInputCompatibility::LeftClick);
}

void Checkbox::SetChecked(bool value)
{
    if (_isChecked == value)
    {
        return;
    }
    
    _isChecked = value;

    OnCheckedChangedInternal();
}

bool Checkbox::IsChecked() const
{
    return _isChecked;
}

bool Checkbox::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }
    
    if (GetDesiredSize() == Vector2::Zero)
    {
        SetDesiredSize(Vector2(10.0f));
    }

    if (GetPadding() == Vector4::Zero)
    {
        SetPadding(Vector4(2.0f));
    }
    
    SetFillMode(EWidgetFillMode::RetainAspectRatio);

    SetCollisionEnabled(true);
    
    return true;
}

void Checkbox::UpdateDesiredSizeInternal()
{
    // Do nothing
}

bool Checkbox::OnReleasedInternal()
{
    SetChecked(!_isChecked);

    return true;
}

void Checkbox::OnCheckedChangedInternal()
{
    WidgetPerPassConstants* parameter = GetMaterial()->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    assert(parameter != nullptr);

    if (_isChecked)
    {
        parameter->Flags |= WidgetPerPassConstants::EWidgetFlags::Pressed;
        OnCheckedChanged.Broadcast(true);
    }
    else
    {
        parameter->Flags &= ~WidgetPerPassConstants::EWidgetFlags::Pressed;
        OnCheckedChanged.Broadcast(false);
    }
}

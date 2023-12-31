#include "Checkbox.h"
#include "MaterialParameterTypes.h"
#include "Rendering/Material.h"

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

    SetFillMode(EWidgetFillMode::RetainAspectRatio);
    
    if (GetDesiredSize() == Vector2::One)
    {
        SetDesiredSize(Vector2(0.04f));
    }

    if (GetPadding() == Vector4::Zero)
    {
        SetPadding(Vector4(0.01f));
    }
    
    return true;
}

void Checkbox::OnReleasedInternal()
{
    SetChecked(!_isChecked);
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

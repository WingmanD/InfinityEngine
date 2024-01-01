#include "Caret.h"
#include "MaterialParameterTypes.h"
#include "Rendering/Material.h"

bool Caret::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    SetSize({0.05f, 0.8f});
    SetCollisionEnabled(false);
    SetVisibility(false);

    {
        WidgetPerPassConstants* param = GetMaterial()->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
        param->BaseColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    }
    
    return true;
}

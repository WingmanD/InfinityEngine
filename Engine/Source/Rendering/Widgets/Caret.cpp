#include "Caret.h"
#include "MaterialParameterTypes.h"
#include "Rendering/Material.h"

bool Caret::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    SetDesiredSize({4.0f, 30.0f});
    SetVisibility(false);

    WidgetPerPassConstants* param = GetMaterial()->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    param->BaseColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    
    return true;
}

void Caret::UpdateDesiredSizeInternal()
{
    // Do nothing, size is overridden
}

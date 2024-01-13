#include "Caret.h"
#include "MaterialParameterTypes.h"
#include "Rendering/Material.h"

bool Caret::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    SetDesiredSize({2.0f, 10.0f});
    SetVisibility(false);

    WidgetPerPassConstants* param = GetMaterial()->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    param->BaseColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    
    return true;
}

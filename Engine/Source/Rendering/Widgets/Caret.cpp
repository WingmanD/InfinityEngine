#include "Caret.h"

bool Caret::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    SetSize({0.01f, 0.05f});
    SetCollisionEnabled(false);
    SetVisibility(false);
    
    return true;
}

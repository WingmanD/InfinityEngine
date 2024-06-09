#pragma once

#include "Component.h"
#include "CLevelEdit.reflection.h"

REFLECTED()
class CLevelEdit : public Component
{
    GENERATED()
    
public:
    uint64 LevelElementID = 0;
};

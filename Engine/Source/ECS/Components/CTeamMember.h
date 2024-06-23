#pragma once

#include "Component.h"
#include "CTeamMember.reflection.h"

REFLECTED()
class CTeamMember : public Component
{
    GENERATED()
    
public:
    PROPERTY(Edit, Serialize)
    uint32 TeamID = 0;
};

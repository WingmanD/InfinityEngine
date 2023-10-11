#pragma once

#include "Core.h"
#include "Asset.h"
#include "Texture.reflection.h"

class RenderingSubsystem;

REFLECTED()
class Texture : public Asset
{
    TEXTURE_GENERATED()
    
public:
    Texture() = default;
    Texture(uint32 width, uint32 height);

    uint32 GetWidth() const;
    uint32 GetHeight() const;
    
private:
    uint32 _width = 0;
    uint32 _height = 0;
};

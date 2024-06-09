#pragma once

#include "Texture.h"
#include "RenderTarget.reflection.h"

class RenderingSubsystem;

REFLECTED()
class RenderTarget : public Texture
{
    GENERATED()
    
public:
    RenderTarget() = default;
    RenderTarget(const RenderTarget&);

    virtual void Resize(uint32 width, uint32 height);

protected:
    RenderTarget(uint32 width, uint32 height);
};

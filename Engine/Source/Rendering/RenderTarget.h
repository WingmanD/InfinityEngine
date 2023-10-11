#pragma once

#include "Texture.h"
#include "Core.h"

class RenderingSubsystem;

class RenderTarget : public Texture
{
public:
    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;

    virtual void Resize(uint32 width, uint32 height) = 0;

protected:
    RenderTarget(uint32 width, uint32 height);
};

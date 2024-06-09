#include "RenderTarget.h"

RenderTarget::RenderTarget(const RenderTarget&) : Texture(0, 0)
{
}

void RenderTarget::Resize(uint32 width, uint32 height)
{
}

RenderTarget::RenderTarget(uint32 width, uint32 height) : Texture(width, height)
{
}

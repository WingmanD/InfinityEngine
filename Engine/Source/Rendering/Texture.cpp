#include "Texture.h"

Texture::Texture(uint32 width, uint32 height) : _width(width), _height(height)
{
}

uint32 Texture::GetWidth() const
{
    return _width;
}

uint32 Texture::GetHeight() const
{
    return _height;
}

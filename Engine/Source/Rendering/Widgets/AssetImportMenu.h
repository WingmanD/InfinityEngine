#pragma once

#include "Widget.h"
#include "AssetImportMenu.reflection.h"

REFLECTED()
class AssetImportMenu : public Widget
{
    GENERATED()
    
public:
    AssetImportMenu() = default;

    virtual bool Initialize() override;
};

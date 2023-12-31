#pragma once

#include "Widget.h"
#include "Caret.reflection.h"

REFLECTED()
class Caret : public Widget
{
    GENERATED()
    
public:
    Caret() = default;
    
    Caret(const Caret& other) = default;
    Caret& operator=(const Caret& other) = default;

    // Widget
public:
    virtual bool Initialize() override;
};

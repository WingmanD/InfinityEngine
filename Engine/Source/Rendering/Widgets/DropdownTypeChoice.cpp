#include "DropdownTypeChoice.h"
#include "MaterialParameterTypes.h"
#include "TextBox.h"
#include "Rendering/Material.h"

DropdownTypeChoice::DropdownTypeChoice(const DropdownTypeChoice& other) : Widget(other)
{
    _type = other._type;
}

DropdownTypeChoice& DropdownTypeChoice::operator=(const DropdownTypeChoice& other)
{
    if (this == &other)
    {
        return *this;
    }

    return *this = DropdownTypeChoice(other);
}

void DropdownTypeChoice::InitializeFromType(Type* type)
{
    if (type == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    Initialize();
    InitializeFromTypeInternal(type);
}

Type* DropdownTypeChoice::GetSelectedType() const
{
    return _type;
}

bool DropdownTypeChoice::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    SetFillMode(EWidgetFillMode::FillX);
    SetPadding({0.005f, 0.005f, 0.005f, 0.005f});

    const std::shared_ptr<TextBox> textWidget = std::make_shared<TextBox>();
    textWidget->Initialize();

    AddChild(textWidget);
    textWidget->SetCollisionEnabled(false);

    _textWidget = textWidget;

    if (_type != nullptr)
    {
        InitializeFromTypeInternal(_type);
    }

    return true;
}

void DropdownTypeChoice::InitializeFromTypeInternal(Type* type)
{
    _type = type;

    const std::shared_ptr<TextBox> textWidget = _textWidget.lock();
    textWidget->SetText(Util::ToWString(type->GetName()));
    textWidget->SetTextColor(DirectX::Colors::Red.v);

    SetCollisionEnabled(true);
}

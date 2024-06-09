#include "AssetImportMenu.h"
#include "Button.h"
#include "TextBox.h"
#include "FlowBox.h"
#include "TypePicker.h"
#include "Rendering/Window.h"

bool AssetImportMenu::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    const std::shared_ptr<FlowBox> verticalBox = AddChild<FlowBox>();
    if (verticalBox == nullptr)
    {
        return false;
    }
    verticalBox->SetDirection(EFlowBoxDirection::Vertical);

    {
        const std::shared_ptr<FlowBox> horizontalBox = verticalBox->AddChild<FlowBox>();
        if (horizontalBox == nullptr)
        {
            return false;
        }
        horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);
        horizontalBox->SetFillMode(EWidgetFillMode::FillX);

        const auto textBox = horizontalBox->AddChild<TextBox>();
        if (textBox == nullptr)
        {
            return false;
        }
        textBox->SetText(L"Import Asset");
        textBox->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);
        textBox->SetPadding({0.0f, 50.0f, 0.0f, 0.0f});

        const std::shared_ptr<Button> closeButton = horizontalBox->AddChild<Button>();
        if (closeButton == nullptr)
        {
            return false;
        }
        closeButton->SetText(L"x");
        closeButton->SetPadding({5.0f, 5.0f, 0.0f, 0.0f});
        closeButton->SetFillMode(EWidgetFillMode::FillY);
        std::ignore = closeButton->OnReleased.Add([this]()
        {
            DestroyWidget();
        });
    }

    const std::shared_ptr<FlowBox> horizontalBox = verticalBox->AddChild<FlowBox>();
    if (horizontalBox == nullptr)
    {
        return false;
    }
    horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);
    horizontalBox->SetFillMode(EWidgetFillMode::FillX);

    const std::shared_ptr<TextBox> textBox = horizontalBox->AddChild<TextBox>();
    if (textBox == nullptr)
    {
        return false;
    }
    textBox->SetText(L"Type:");
    textBox->SetPadding({0.0f, 50.0f, 0.0f, 0.0f});

    const std::shared_ptr<TypePicker> typePicker = TypePicker::CreateForType(
        Asset::StaticType(), [](const Type* type)
        {
            return type->GetCDO<Asset>()->GetImporterType() != nullptr;
        });

    if (typePicker == nullptr)
    {
        return false;
    }
    horizontalBox->AddChild(typePicker);

    const std::shared_ptr<Button> button = verticalBox->AddChild<Button>();
    if (button == nullptr)
    {
        return false;
    }

    button->SetText(L"Next");
    button->SetFillMode(EWidgetFillMode::FillX);
    std::ignore = button->OnReleased.Add([this, typePicker]()
    {
        const Type* type = typePicker->GetSelectedType();
        if (type == nullptr)
        {
            return;
        }

        const Asset* cdo = type->GetCDO<Asset>();
        const auto widget = cdo->CreateImportWidget();
        if (widget == nullptr)
        {
            return;
        }

        GetParentWindow()->AddPopup(widget);

        DestroyWidget();
    });

    return true;
}

#include "AssetCreatorMenu.h"
#include "Button.h"
#include "EditableTextBox.h"
#include "FlowBox.h"
#include "TypePicker.h"
#include "TextBox.h"
#include "Engine/Subsystems/AssetManager.h"

bool AssetCreatorMenu::Initialize()
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

    const auto closeButton = verticalBox->AddChild<Button>();
    if (closeButton == nullptr)
    {
        return false;
    }
    closeButton->SetText(L"x");
    closeButton->OnReleased.Add([this]()
    {
        Destroy();
    });
    // todo align right

    const std::shared_ptr<TextBox> title = verticalBox->AddChild<TextBox>();
    if (title == nullptr)
    {
        return false;
    }
    title->SetText(L"Create New Asset");

    const std::shared_ptr<FlowBox> nameHorizontalBox = verticalBox->AddChild<FlowBox>();
    if (nameHorizontalBox == nullptr)
    {
        return false;
    }
    nameHorizontalBox->SetDirection(EFlowBoxDirection::Horizontal);

    const std::shared_ptr<TextBox> nameTextBox = nameHorizontalBox->AddChild<TextBox>();
    if (nameTextBox == nullptr)
    {
        return false;
    }
    nameTextBox->SetText(L"Name: ");

    const std::shared_ptr<EditableTextBox> nameInput = nameHorizontalBox->AddChild<EditableTextBox>();
    if (nameInput == nullptr)
    {
        return false;
    }
    nameInput->SetCollisionEnabled(true);
    _nameInput = nameInput;

    const std::shared_ptr<FlowBox> typeHorizontalBox = verticalBox->AddChild<FlowBox>();
    if (typeHorizontalBox == nullptr)
    {
        return false;
    }
    typeHorizontalBox->SetDirection(EFlowBoxDirection::Horizontal);

    const std::shared_ptr<TextBox> typeNameBox = typeHorizontalBox->AddChild<TextBox>();
    if (typeNameBox == nullptr)
    {
        return false;
    }
    typeNameBox->SetText(L"Type: ");

    const std::shared_ptr<TypePicker> typePicker = typeHorizontalBox->AddChild<TypePicker>();
    if (typePicker == nullptr)
    {
        return false;
    }
    _typePicker = typePicker;

    typePicker->InitializeFromType(Object::StaticType());

    const std::shared_ptr<Button> createButton = verticalBox->AddChild<Button>();
    if (createButton == nullptr)
    {
        return false;
    }
    createButton->SetText(L"Create");
    createButton->SetEnabled(false);
    createButton->SetFillMode(EWidgetFillMode::FillX);

    createButton->OnReleased.Add([this]()
    {
        const std::wstring name = _nameInput.lock()->GetText();
        const Type* type = _typePicker.lock()->GetSelectedType();

        if (name.empty() || type == nullptr)
        {
            return;
        }

        AssetManager::Get().NewAsset(type, name);

        Destroy();
    });

    nameInput->OnValueChanged.Add([createButton](const std::wstring& text)
    {
        if (text.empty())
        {
            createButton->SetEnabled(false);
            return;
        }

        if (AssetManager::Get().FindAssetByName(text) != nullptr)
        {
            createButton->SetEnabled(false);
            return;
        }

        createButton->SetEnabled(true);
    });

    return true;
}

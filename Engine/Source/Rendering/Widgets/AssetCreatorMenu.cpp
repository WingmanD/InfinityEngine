#include "AssetCreatorMenu.h"
#include "Button.h"
#include "EditableTextBox.h"
#include "FlowBox.h"
#include "MaterialParameterTypes.h"
#include "TableWidget.h"
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

    {
        const std::shared_ptr<FlowBox> horizontalBox = verticalBox->AddChild<FlowBox>();
        if (horizontalBox == nullptr)
        {
            return false;
        }
        horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);
        horizontalBox->SetFillMode(EWidgetFillMode::FillX);

        const std::shared_ptr<TextBox> title = horizontalBox->AddChild<TextBox>();
        if (title == nullptr)
        {
            return false;
        }
        title->SetText(L"Create New Asset");
        title->SetPadding({0.0f, 50.0f, 0.0f, 0.0f});
        title->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

        const std::shared_ptr<Button> closeButton = horizontalBox->AddChild<Button>();
        if (closeButton == nullptr)
        {
            return false;
        }
        closeButton->SetText(L"x");
        closeButton->SetPadding({5.0f, 5.0f, 0.0f, 0.0f});
        closeButton->SetFillMode(EWidgetFillMode::FillY);
        closeButton->OnReleased.Add([this]()
        {
            DestroyWidget();
        });
    }

    const std::shared_ptr<TableWidget> table = verticalBox->AddChild<TableWidget>();
    if (table == nullptr)
    {
        return false;
    }

    const auto nameRow = std::make_shared<TableRowWidget>();
    if (!nameRow->Initialize())
    {
        return false;
    }
    table->AddRow(nameRow);

    const std::shared_ptr<TextBox> nameTextBox = nameRow->AddChild<TextBox>();
    if (nameTextBox == nullptr)
    {
        return false;
    }
    nameTextBox->SetText(L"Name: ");

    const std::shared_ptr<EditableTextBox> nameInput = nameRow->AddChild<EditableTextBox>();
    if (nameInput == nullptr)
    {
        return false;
    }
    nameInput->SetFillMode(EWidgetFillMode::FillX);
    _nameInput = nameInput;

    const auto typeRow = std::make_shared<TableRowWidget>();
    if (!typeRow->Initialize())
    {
        return false;
    }
    table->AddRow(typeRow);

    const std::shared_ptr<TextBox> typeNameBox = typeRow->AddChild<TextBox>();
    if (typeNameBox == nullptr)
    {
        return false;
    }
    typeNameBox->SetText(L"Type: ");

    const std::shared_ptr<TypePicker> typePicker = typeRow->AddChild<TypePicker>();
    if (typePicker == nullptr)
    {
        return false;
    }
    _typePicker = typePicker;

    typePicker->InitializeFromType(Asset::StaticType());

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

        AssetManager::Get().NewAsset(*type, Name(name));

        DestroyWidget();
    });

    nameInput->OnValueChanged.Add([createButton](const std::wstring& text)
    {
        if (text.empty())
        {
            createButton->SetEnabled(false);
            return;
        }

        if (AssetManager::Get().FindAssetByName(Name(text)) != nullptr)
        {
            createButton->SetEnabled(false);
            return;
        }

        createButton->SetEnabled(true);
    });

    return true;
}

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

    const SharedObjectPtr<FlowBox> verticalBox = AddChild<FlowBox>();
    if (verticalBox == nullptr)
    {
        return false;
    }

    {
        const SharedObjectPtr<FlowBox> horizontalBox = verticalBox->AddChild<FlowBox>();
        if (horizontalBox == nullptr)
        {
            return false;
        }
        horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);
        horizontalBox->SetFillMode(EWidgetFillMode::FillX);

        const SharedObjectPtr<TextBox> title = horizontalBox->AddChild<TextBox>();
        if (title == nullptr)
        {
            return false;
        }
        title->SetText(L"Create New Asset");
        title->SetPadding({0.0f, 50.0f, 0.0f, 0.0f});
        title->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

        const SharedObjectPtr<Button> closeButton = horizontalBox->AddChild<Button>();
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

    const SharedObjectPtr<TableWidget> table = verticalBox->AddChild<TableWidget>();
    if (table == nullptr)
    {
        return false;
    }

    const auto nameRow = NewObject<TableRowWidget>();
    if (!nameRow->Initialize())
    {
        return false;
    }
    table->AddRow(nameRow);

    const SharedObjectPtr<TextBox> nameTextBox = nameRow->AddChild<TextBox>();
    if (nameTextBox == nullptr)
    {
        return false;
    }
    nameTextBox->SetText(L"Name: ");

    const SharedObjectPtr<EditableTextBox> nameInput = nameRow->AddChild<EditableTextBox>();
    if (nameInput == nullptr)
    {
        return false;
    }
    nameInput->SetFillMode(EWidgetFillMode::FillX);
    _nameInput = nameInput;

    const auto typeRow = NewObject<TableRowWidget>();
    if (!typeRow->Initialize())
    {
        return false;
    }
    table->AddRow(typeRow);

    const SharedObjectPtr<TextBox> typeNameBox = typeRow->AddChild<TextBox>();
    if (typeNameBox == nullptr)
    {
        return false;
    }
    typeNameBox->SetText(L"Type: ");

    const SharedObjectPtr<TypePicker> typePicker = typeRow->AddChild<TypePicker>();
    if (typePicker == nullptr)
    {
        return false;
    }
    _typePicker = typePicker;

    typePicker->InitializeFromType(Asset::StaticType());

    const SharedObjectPtr<Button> createButton = verticalBox->AddChild<Button>();
    if (createButton == nullptr)
    {
        return false;
    }
    createButton->SetText(L"Create");
    createButton->SetEnabled(false);
    createButton->SetFillMode(EWidgetFillMode::FillX);

    std::ignore = createButton->OnReleased.Add([this]()
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

    std::ignore = nameInput->OnValueChanged.Add([createButton](const std::wstring& text)
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

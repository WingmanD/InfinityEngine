#include "TableWidget.h"
#include "FlowBox.h"

bool TableRowWidget::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    SetFillMode(EWidgetFillMode::FillX);

    return true;
}

void TableRowWidget::RebuildLayoutInternal()
{
    const std::shared_ptr<TableWidget> table = std::dynamic_pointer_cast<TableWidget>(
        GetParentWidget()->GetParentWidget());
    if (table == nullptr)
    {
        DEBUG_BREAK();
        return;
    }
    const std::vector<float>& columnRatios = table->GetColumnRatios();

    const Vector2 screenSize = GetScreenSize();

    if (GetChildren().size() > columnRatios.size())
    {
        DEBUG_BREAK();
        return;
    }

    int32 index = 0;
    for (const std::shared_ptr<Widget>& widget : GetChildren())
    {
        const Vector2& childDesiredSize = widget->GetDesiredSize();
        const Vector2& childPaddedDesiredSize = widget->GetPaddedDesiredSize();

        Vector2 newChildSize;

        if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillY | EWidgetFillMode::RetainAspectRatio))
        {
            newChildSize = Vector2(
                childDesiredSize.x * (GetDesiredSize().y / childPaddedDesiredSize.y) / screenSize.x,
                childDesiredSize.y / childPaddedDesiredSize.y);
        }
        else if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillY))
        {
            newChildSize = Vector2(childDesiredSize.x / screenSize.x, 1.0f);
        }
        else
        {
            newChildSize = childDesiredSize / screenSize;
        }

        if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillX))
        {
            newChildSize.x = columnRatios[index];
        }

        const Vector2 newChildPaddedSize = newChildSize * widget->GetPaddedDesiredSize() / childDesiredSize;

        widget->SetSize(newChildSize);

        float offset = 0.0f;
        if (index > 0)
        {
            for (auto i = 0; i < index; ++i)
            {
                offset += columnRatios[i];
            }
        }

        Vector2 childPosition = {offset + newChildPaddedSize.x * 0.5f, 0.0f};
        widget->SetPosition(childPosition);

        ++index;
    }
}

void TableRowWidget::UpdateDesiredSizeInternal()
{
    Vector2 newDesiredSize;
    std::vector<std::shared_ptr<Widget>> widgetsWithFill;

    for (const std::shared_ptr<Widget>& widget : GetChildren())
    {
        const Vector2& paddedDesiredSize = widget->GetPaddedDesiredSize();
        if ((widget->GetFillMode() & EWidgetFillMode::RetainAspectRatio) != EWidgetFillMode::None)
        {
            widgetsWithFill.push_back(widget);
        }
        else
        {
            newDesiredSize.x += paddedDesiredSize.x;
        }

        newDesiredSize.y = std::max(newDesiredSize.y, paddedDesiredSize.y);
    }

    for (const std::shared_ptr<Widget>& widget : widgetsWithFill)
    {
        const Vector2& paddedDesiredSize = widget->GetPaddedDesiredSize();
        newDesiredSize.x += paddedDesiredSize.x * newDesiredSize.y / paddedDesiredSize.y;
    }

    SetDesiredSize(newDesiredSize);
}

void TableRowWidget::OnChildAdded(const std::shared_ptr<Widget>& child)
{
    child->SetAnchor(EWidgetAnchor::CenterLeft);
}

void TableWidget::AddRow(const std::shared_ptr<TableRowWidget>& row)
{
    if (row == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    _verticalBox.lock()->AddChild(row);
}

void TableWidget::RemoveRow(const std::shared_ptr<TableRowWidget>& row)
{
    if (row == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    const std::shared_ptr<FlowBox> verticalBox = _verticalBox.lock();
    verticalBox->RemoveChild(row);

    size_t numColumns = 0;
    for (const std::shared_ptr<Widget>& widget : verticalBox->GetChildren())
    {
        numColumns = std::max(numColumns, widget->GetChildren().size());
    }

    _columnRatios.resize(numColumns);
}

const std::vector<float>& TableWidget::GetColumnRatios() const
{
    return _columnRatios;
}

bool TableWidget::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    const std::shared_ptr<FlowBox> flowBox = AddChild<FlowBox>();
    if (!flowBox->Initialize())
    {
        return false;
    }
    _verticalBox = flowBox;

    flowBox->SetDirection(EFlowBoxDirection::Vertical);
    flowBox->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

    return true;
}

void TableWidget::UpdateDesiredSizeInternal()
{
    Widget::UpdateDesiredSizeInternal();

    const std::shared_ptr<FlowBox> verticalBox = _verticalBox.lock();

    size_t numColumns = 0;
    for (const std::shared_ptr<Widget>& widget : verticalBox->GetChildren())
    {
        numColumns = std::max(numColumns, widget->GetChildren().size());
    }
    _columnRatios.resize(numColumns);

    static std::vector<float> maxWidths(numColumns);
    for (const std::shared_ptr<Widget>& row : verticalBox->GetChildren())
    {
        const std::vector<std::shared_ptr<Widget>>& children = row->GetChildren();
        for (auto i = 0; i < children.size(); ++i)
        {
            maxWidths[i] = std::max(maxWidths[i], children[i]->GetPaddedDesiredSize().x);
        }
    }

    float sum = 0.0f;
    for (const float width : maxWidths)
    {
        sum += width;
    }

    SetDesiredSize({sum, GetDesiredSize().y});

    for (auto i = 0; i < _columnRatios.size(); ++i)
    {
        _columnRatios[i] = maxWidths[i] / sum;
    }
}

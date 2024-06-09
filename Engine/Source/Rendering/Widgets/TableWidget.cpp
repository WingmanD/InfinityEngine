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
    const SharedObjectPtr<TableWidget> table = std::dynamic_pointer_cast<TableWidget>(
        GetParentWidget()->GetParentWidget());
    if (table == nullptr)
    {
        DEBUG_BREAK();
        return;
    }
    const std::vector<float>& columnRatios = table->GetColumnRatios();

    const Vector2 screenSize = GetScreenSize();

    if (GetChildren().Count() > columnRatios.size())
    {
        DEBUG_BREAK();
        return;
    }

    int32 index = 0;
    for (const SharedObjectPtr<Widget>& widget : GetChildren())
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
    DArray<SharedObjectPtr<Widget>> widgetsWithFill;

    for (const SharedObjectPtr<Widget>& widget : GetChildren())
    {
        const Vector2& paddedDesiredSize = widget->GetPaddedDesiredSize();
        if ((widget->GetFillMode() & EWidgetFillMode::RetainAspectRatio) != EWidgetFillMode::None)
        {
            widgetsWithFill.Add(widget);
        }
        else
        {
            newDesiredSize.x += paddedDesiredSize.x;
        }

        newDesiredSize.y = std::max(newDesiredSize.y, paddedDesiredSize.y);
    }

    for (const SharedObjectPtr<Widget>& widget : widgetsWithFill)
    {
        const Vector2& paddedDesiredSize = widget->GetPaddedDesiredSize();
        newDesiredSize.x += paddedDesiredSize.x * newDesiredSize.y / paddedDesiredSize.y;
    }

    SetDesiredSize(newDesiredSize);
}

void TableRowWidget::OnChildAdded(const SharedObjectPtr<Widget>& child)
{
    child->SetAnchor(EWidgetAnchor::CenterLeft);
}

void TableWidget::AddRow(const SharedObjectPtr<TableRowWidget>& row)
{
    if (row == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    _verticalBox.lock()->AddChild(row);
}

void TableWidget::RemoveRow(const SharedObjectPtr<TableRowWidget>& row)
{
    if (row == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    const SharedObjectPtr<FlowBox> verticalBox = _verticalBox.lock();
    verticalBox->RemoveChild(row);

    size_t numColumns = 0;
    for (const SharedObjectPtr<Widget>& widget : verticalBox->GetChildren())
    {
        numColumns = std::max(numColumns, widget->GetChildren().Count());
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

    const SharedObjectPtr<FlowBox> flowBox = AddChild<FlowBox>();
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

    const SharedObjectPtr<FlowBox> verticalBox = _verticalBox.lock();

    size_t numColumns = 0;
    for (const SharedObjectPtr<Widget>& widget : verticalBox->GetChildren())
    {
        numColumns = std::max(numColumns, widget->GetChildren().Count());
    }
    _columnRatios.resize(numColumns);

    std::vector<float> maxWidths(numColumns);
    for (const SharedObjectPtr<Widget>& row : verticalBox->GetChildren())
    {
        const DArray<SharedObjectPtr<Widget>>& children = row->GetChildren();
        for (auto i = 0; i < children.Count(); ++i)
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

﻿#pragma once

#include "Widget.h"
#include "TableWidget.reflection.h"

class FlowBox;

REFLECTED()
class TableRowWidget : public Widget
{
    GENERATED()

public:
    TableRowWidget() = default;

    // Widget
public:
    bool Initialize() override;

    // Widget
protected:
    void RebuildLayoutInternal() override;
    void UpdateDesiredSizeInternal() override;

    void OnChildAdded(const SharedObjectPtr<Widget>& child) override;
};

REFLECTED()
class TableWidget : public Widget
{
    GENERATED()

public:
    TableWidget() = default;
    
    void AddRow(const SharedObjectPtr<TableRowWidget>& row);
    void RemoveRow(const SharedObjectPtr<TableRowWidget>& row);

    const std::vector<float>& GetColumnRatios() const;

    // Widget
public:
    bool Initialize() override;

    // Widget
protected:
    void UpdateDesiredSizeInternal() override;

private:
    std::weak_ptr<FlowBox> _verticalBox;
    std::vector<float> _columnRatios;
};

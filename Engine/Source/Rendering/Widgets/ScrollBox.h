#pragma once

#include "Widget.h"
#include "ScrollBox.reflection.h"

enum class EScrollBoxDirection : uint8
{
    Vertical,
    Horizontal
};

REFLECTED()
class ScrollBox : public Widget
{
    GENERATED()
    
public:
    ScrollBox();
    
    void SetDirection(EScrollBoxDirection direction);
    EScrollBoxDirection GetDirection() const;

    void SetMaxDesiredSize(const Vector2& size);
    const Vector2& GetMaxDesiredSize() const;

protected:
    virtual void OnChildAdded(const SharedObjectPtr<Widget>& child) override;
    virtual void OnChildRemoved(const SharedObjectPtr<Widget>& child) override;

    virtual bool OnScrolledInternal(int32 value) override;

    virtual void UpdateDesiredSizeInternal() override;
    virtual void RebuildLayoutInternal() override;
    
private:
    PROPERTY(Edit, DisplayName = "Direction")
    EScrollBoxDirection _direction = EScrollBoxDirection::Vertical;

    PROPERTY(Edit, DisplayName = "Max Size")
    Vector2 _maxDesiredSize = Vector2(1.0f, 1.0f);

    float _progress = 0.0f;

    void SetProgress(float value);
};

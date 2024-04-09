#pragma once

#include "CoreMinimal.h"
#include "NonCopyable.h"
#include <memory>

class Window;
class Widget;

class WidgetRenderingProxy : public NonCopyable<WidgetRenderingProxy>
{
public:
    WidgetRenderingProxy() = default;
    virtual ~WidgetRenderingProxy() = default;

    virtual bool Initialize();
    void SetWidget(Widget* owningWidget);

    virtual void OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow);
    virtual void OnTransformChanged();
    virtual void OnWidgetRectChanged();

protected:
    Widget& GetOwningWidget() const;

    template <typename T> requires IsA<T, Widget>
    T& GetOwningWidget() const
    {
        return dynamic_cast<T&>(GetOwningWidget());
    }

private:
    Widget* _owningWidget = nullptr;
};

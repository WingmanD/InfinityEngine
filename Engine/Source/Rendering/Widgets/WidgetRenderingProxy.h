#pragma once

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

protected:
    Widget& GetOwningWidget() const;

private:
    Widget* _owningWidget = nullptr;
};

#include "WidgetRenderingProxy.h"

bool WidgetRenderingProxy::Initialize()
{
    return true;
}

void WidgetRenderingProxy::SetWidget(Widget* owningWidget)
{
    _owningWidget = owningWidget;
}

void WidgetRenderingProxy::OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow)
{
}

void WidgetRenderingProxy::OnTransformChanged()
{
}

void WidgetRenderingProxy::OnWidgetRectChanged()
{
}

Widget& WidgetRenderingProxy::GetOwningWidget() const
{
    return *_owningWidget;
}

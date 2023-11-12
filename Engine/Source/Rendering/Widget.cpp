#include "Widget.h"
#include "Material.h"
#include "StaticMesh.h"
#include "UIStatics.h"
#include "MaterialParameterTypes.h"
#include "Window.h"
#include "DX12/DX12Shader.h"
#include "DX12/DX12StaticMeshRenderingData.h"
#include "Engine/Subsystems/AssetManager.h"
#include <array>
#include <cassert>

std::array<const Vector2, 9> Widget::_anchorPositionMap = {
    Vector2(-0.5f, 0.5f), // TopLeft
    Vector2(0.0f, 0.5f), // TopCenter
    Vector2(0.5f, 0.5f), // TopRight
    Vector2(-0.5f, 0.0f), // CenterLeft
    Vector2(0.0f, 0.0f), // Center
    Vector2(0.5f, 0.0f), // CenterRight
    Vector2(-0.5f, -0.5f), // BottomLeft
    Vector2(0.0f, -0.5f), // BottomCenter
    Vector2(0.5f, -0.5f)  // BottomRight
};

Widget::Widget()
{
}

bool Widget::Initialize()
{
    _quadMesh = UIStatics::GetUIQuadMesh();
    assert(_quadMesh != nullptr);
    // todo asset manager should have a function for finding and loading asset
    _quadMesh->Load();

    if (_material == nullptr)
    {
        _material = _quadMesh->GetMaterial();
    }
    else
    {
        _material->Load();
        _quadMesh->SetMaterial(_material);
    }

    // todo instance mesh

    // todo add to grid

    return true;
}

void Widget::TestDraw(ID3D12GraphicsCommandList* commandList)
{
    if (_material == nullptr)
    {
        return;
    }

    WidgetPerPassConstants* parameter = _material->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    assert(parameter != nullptr);

    parameter->Transform = _transform.GetMatrix();
    parameter->Flags = WidgetPerPassConstants::EWidgetFlags::Enabled;

    // todo widget rect needs to be in screen space, not CS
    //commandList->RSSetScissorRects(1, &_widgetRect);

    if (_quadMesh->GetRenderingData()->IsUploaded())
    {
        static_cast<DX12StaticMeshRenderingData*>(_quadMesh->GetRenderingData())->SetupDrawing(commandList);
    }

    for (const std::shared_ptr<Widget>& widget : _children)
    {
        widget->TestDraw(commandList);
    }
}

void Widget::SetPosition(const Vector2& position)
{
    _transform.SetPosition(GetAnchorPosition(GetAnchor()) + position);
}

const Vector2& Widget::GetPosition() const
{
    return _transform.GetPosition() - GetAnchorPosition(GetAnchor());
}

void Widget::SetRotation(float degrees)
{
    _transform.SetRotation(degrees);
}

float Widget::GetRotation() const
{
    return _transform.GetRotation();
}

void Widget::SetSize(const Vector2& size)
{
    const std::shared_ptr<Window> window = GetParentWindow();
    if (window == nullptr)
    {
        return;
    }

    const Vector2 sizeTransformed = Vector2(size.x / window->GetAspectRatio(), size.y);
    _transform.SetScale(sizeTransformed);

    OnResized();
}

const Vector2& Widget::GetSize() const
{
    const Vector2 size = _transform.GetScale();
    
    const std::shared_ptr<Window> window = GetParentWindow();
    if (window == nullptr)
    {
        return size;
    }

    return Vector2(size.x * window->GetAspectRatio(), size.y);
}

void Widget::SetTransform(const Transform2D& transform)
{
    _transform = transform;
}

const Transform2D& Widget::GetTransform() const
{
    return _transform;
}

void Widget::SetMaterial(const std::shared_ptr<Material>& material)
{
    _quadMesh->SetMaterial(material);
}

std::shared_ptr<Material> Widget::GetMaterial() const
{
    return _material;
}

void Widget::AddChild(const std::shared_ptr<Widget>& widget)
{
    if (widget == nullptr)
    {
        return;
    }

    _children.push_back(widget);
    widget->_parentWidget = std::static_pointer_cast<Widget>(shared_from_this());
}

void Widget::RemoveChild(const std::shared_ptr<Widget>& widget)
{
    widget->_parentWidget.reset();
    std::erase(_children, widget);
}

std::shared_ptr<Widget> Widget::GetParentWidget() const
{
    return _parentWidget.lock();
}

RECT Widget::GetRect() const
{
    return _widgetRect;
}

void Widget::SetWindow(const std::shared_ptr<Window>& window)
{
    _parentWindow = window;
    OnWindowChanged(window);
}

std::shared_ptr<Window> Widget::GetParentWindow() const
{
    return _parentWindow.lock();
}

void Widget::Destroy()
{
    if (const std::shared_ptr<Widget> parent = GetParentWidget())
    {
        parent->RemoveChild(std::static_pointer_cast<Widget>(shared_from_this()));
    }
}

void Widget::SetAnchor(EWidgetAnchor anchor)
{
    const Vector2 oldPosition = GetPosition();

    _anchor = anchor;

    SetPosition(oldPosition);
}

void Widget::OnParentResized()
{
    const Vector2 size = GetSize();
    SetSize(size);
    for (std::shared_ptr<Widget>& widget : _children)
    {
        widget->OnParentResized();
    }
}

void Widget::OnResized()
{
    const Vector2& position = _transform.GetPosition();
    const Vector2& size = _transform.GetScale();

    _widgetRect = RECT
    {
        static_cast<LONG>(position.x),
        static_cast<LONG>(position.y),
        static_cast<LONG>(position.x + size.x),
        static_cast<LONG>(position.y + size.y)
    };

    assert(_material != nullptr);
}

const Vector2& Widget::GetAnchorPosition(EWidgetAnchor anchor)
{
    return _anchorPositionMap[static_cast<uint32>(anchor)];
}

EWidgetAnchor Widget::GetAnchor() const
{
    return _anchor;
}

void Widget::OnWindowChanged(const std::shared_ptr<Window>& window)
{
    if (window == nullptr)
    {
        Destroy();
        return;
    }

    if (_parentWindow.lock() == window)
    {
        return;
    }

    _parentWindow = window;
}

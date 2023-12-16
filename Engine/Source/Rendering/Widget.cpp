﻿#include "Widget.h"
#include "Material.h"
#include "StaticMesh.h"
#include "UIStatics.h"
#include "MaterialParameterTypes.h"
#include "Window.h"
#include "DX12/DX12Shader.h"
#include "DX12/DX12StaticMeshRenderingData.h"
#include "Engine/Subsystems/AssetManager.h"
#include <cassert>

#include "SpriteBatch.h"

std::array<const Vector2, 9> Widget::_anchorPositionMap = {
    Vector2(-1.0f, 1.0f), // TopLeft
    Vector2(0.0f, 1.0f), // TopCenter
    Vector2(1.0f, 1.0f), // TopRight
    Vector2(-1.0f, 0.0f), // CenterLeft
    Vector2(0.0f, 0.0f), // Center
    Vector2(1.0f, 0.0f), // CenterRight
    Vector2(-1.0f, -1.0f), // BottomLeft
    Vector2(0.0f, -1.0f), // BottomCenter
    Vector2(1.0f, -1.0f)  // BottomRight
};

Widget::Widget()
{
    SetVisibility(true);
    SetCollisionEnabled(true);
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

    if (IsVisible())
    {
        WidgetPerPassConstants* parameter = _material->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
        assert(parameter != nullptr);

        parameter->Transform = _quadTransform.GetMatrix() * _transform.GetMatrix();
        if (const std::shared_ptr<Widget>& parentWidget = GetParentWidget())
        {
            parameter->Transform = parentWidget->GetTransform().GetMatrix() * parameter->Transform;
        }

        parameter->Flags = WidgetPerPassConstants::EWidgetFlags::Enabled;

        // todo widget rect needs to be in screen space, not CS
        //commandList->RSSetScissorRects(1, &_widgetRect);

        if (_quadMesh->GetRenderingData()->IsUploaded())
        {
            static_cast<DX12StaticMeshRenderingData*>(_quadMesh->GetRenderingData())->SetupDrawing(commandList);
        }
    }

    for (const std::shared_ptr<Widget>& widget : _children)
    {
        widget->TestDraw(commandList);
    }
}
void Widget::SetVisibility(bool value, bool recursive /*= false*/)
{
    if (value == IsVisible())
    {
        return;
    }

    _state = value ? _state | EWidgetState::Visible : _state & ~EWidgetState::Visible;

    if (recursive)
    {
        for (const std::shared_ptr<Widget>& widget : _children)
        {
            widget->SetVisibility(value, recursive);
        }
    }

    // todo disable mesh instance
}

bool Widget::IsVisible() const
{
    return (_state & EWidgetState::Visible) != EWidgetState::None && (_state & EWidgetState::Collapsed) == EWidgetState::None;
}

void Widget::SetCollisionEnabled(bool value, bool recursive /*= false*/)
{
    _state = value ? _state | EWidgetState::CollisionEnabled : _state & ~EWidgetState::CollisionEnabled;

    // todo disable collision in hittest grid
}

bool Widget::IsCollisionEnabled() const
{
    return (_state & EWidgetState::CollisionEnabled) != EWidgetState::None;
}

void Widget::SetCollapsed(bool value)
{
    if (value == IsCollapsed())
    {
        return;
    }

    _state = value ? _state | EWidgetState::Collapsed : _state & ~EWidgetState::Collapsed;

    if (value)
    {
        _storedCollapsedSize = _size;
        SetSize({0.0f, 0.0f});
    }
    else
    {
        SetSize(_storedCollapsedSize);
    }

    // todo disable mesh instance and collision
}

bool Widget::IsCollapsed() const
{
    return (_state & EWidgetState::Collapsed) != EWidgetState::None;
}

void Widget::SetPosition(const Vector2& position)
{
    _transform.SetPosition(GetAnchorPosition(GetAnchor()) + position);
}

Vector2 Widget::GetPosition() const
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

void Widget::SetScale(const Vector2& scale)
{
    _transform.SetScale(scale);
}

Vector2 Widget::GetScale() const
{
    return _transform.GetScale();
}

void Widget::SetSize(const Vector2& size)
{
    _size = size;

    if (const std::shared_ptr<Widget> widget = GetParentWidget())
    {
        _quadTransform.SetScale(size * widget->GetSize());
    }
    else
    {
        _quadTransform.SetScale(size);
    }

    OnResized();
}

Vector2 Widget::GetSize() const
{
    return _size;
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
    // todo unlink old material and shared param
    _quadMesh->SetMaterial(material);
    _material = material;

    if (_material != nullptr)
    {
        _material->GetParameterMap().SetSharedParameter("GWindowGlobals", GetParentWindow()->GetWindowGlobals(), true);
    }
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
    widget->SetWindow(GetParentWindow());
    widget->OnParentResized();

    // todo what about state flags and propagation?
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
    // todo desired size
    const Vector2 size = GetSize();
    SetSize(size);

    for (std::shared_ptr<Widget>& widget : _children)
    {
        widget->OnParentResized();
    }
}

void Widget::OnResized()
{
    const std::shared_ptr<Window> window = GetParentWindow();
    if (window == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    // todo calculate vertices in screen space, this is in local space
    const Vector2 windowSize = Vector2(static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));

    const Vector2 position = (_transform.GetPosition() - GetAnchorPosition(GetAnchor())) * windowSize;
    const Vector2& size = _transform.GetScale() * windowSize;

    // calculate AABB for 4 vertices
    _widgetRect = RECT
    {
        static_cast<LONG>(position.x),
        static_cast<LONG>(position.y),
        static_cast<LONG>(position.x + size.x),
        static_cast<LONG>(position.y + size.y)
    };

    assert(_material != nullptr);
    // todo update material parameters - should be on GPU at all times, updated only when necessary
}

Vector2 Widget::GetAnchorPosition(EWidgetAnchor anchor) const
{
    const Vector2 anchorPosition = _anchorPositionMap[static_cast<uint32>(anchor)];
    if (const std::shared_ptr<Widget>& parentWidget = GetParentWidget())
    {
        return parentWidget->GetSize() * anchorPosition;
    }

    return anchorPosition;
}

EWidgetAnchor Widget::GetAnchor() const
{
    return _anchor;
}

void Widget::OnWindowChanged(const std::shared_ptr<Window>& window)
{
    if (_material != nullptr)
    {
        _material->GetParameterMap().SetSharedParameter("GWindowGlobals", window->GetWindowGlobals(), true);
    }
}

#include "Widget.h"

#include "Material.h"
#include "StaticMesh.h"
#include "UIStatics.h"

Widget::Widget()
{
}

bool Widget::Initialize()
{
    _quadMesh = UIStatics::GetUIQuadMesh();
    assert(_quadMesh != nullptr);

    _quadMesh->SetMaterial(_material);

    // todo add to grid

    return true;
}

void Widget::SetPosition(const Vector2& position)
{
    _position = position;
}

const Vector2& Widget::GetPosition() const
{
    return _position;
}

void Widget::SetSize(const Vector2& size)
{
    if (_size == size)
    {
        return;
    }

    _size = size;

    OnResized();
}

const Vector2& Widget::GetSize() const
{
    return _size;
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
    widget->_parent = std::static_pointer_cast<Widget>(shared_from_this());
}

void Widget::RemoveChild(const std::shared_ptr<Widget>& widget)
{
    widget->_parent.reset();
    std::erase(_children, widget);
}

std::shared_ptr<Widget> Widget::GetParent() const
{
    return _parent.lock();
}

RECT Widget::GetRect() const
{
    return _widgetRect;
}

void Widget::OnResized()
{
    _widgetRect = RECT
    {
        static_cast<LONG>(_position.GetX()),
        static_cast<LONG>(_position.GetY()),
        static_cast<LONG>(_position.GetX() + _size.GetY()),
        static_cast<LONG>(_position.GetX() + _size.GetY())
    };

    assert(_material != nullptr);
}

#include "DX12TextWidgetRenderingProxy.h"
#include "DX12RenderingSubsystem.h"
#include "DX12Window.h"
#include "RenderTargetState.h"
#include "ResourceUploadBatch.h"
#include "SpriteBatch.h"
#include "Rendering/Widgets/TextWidget.h"
#include "Rendering/Widgets/UIStatics.h"
#include "Rendering/Widgets/Widget.h"

bool DX12TextWidgetRenderingProxy::Initialize()
{
    const DX12RenderingSubsystem& renderingSubsystem = static_cast<DX12RenderingSubsystem&>(RenderingSubsystem::Get());

    const DirectX::RenderTargetState renderTargetState(renderingSubsystem.GetFrameBufferFormat(), renderingSubsystem.GetDepthStencilFormat());
    DirectX::SpriteBatchPipelineStateDescription pipelineStateDescription(renderTargetState);

    DirectX::ResourceUploadBatch resourceUploadBatch(renderingSubsystem.GetDevice());
    resourceUploadBatch.Begin();
    _spriteBatch = std::make_unique<DirectX::SpriteBatch>(renderingSubsystem.GetDevice(), resourceUploadBatch, pipelineStateDescription);
    resourceUploadBatch.End(renderingSubsystem.GetCommandQueue()).wait();

    return true;
}

void DX12TextWidgetRenderingProxy::OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow)
{
    if (_viewportChangedHandle.IsValid())
    {
        if (DX12Window* window = static_cast<DX12Window*>(oldWindow.get()))
        {
            window->OnViewportChanged.Unsubscribe(_viewportChangedHandle);
        }
    }

    if (DX12Window* window = static_cast<DX12Window*>(newWindow.get()))
    {
        _spriteBatch->SetViewport(window->GetViewport());

        window->OnViewportChanged.Subscribe([this](const D3D12_VIEWPORT& viewport)
        {
            _spriteBatch->SetViewport(viewport);
        });
    }
}

void DX12TextWidgetRenderingProxy::OnTransformChanged()
{
    const TextWidget& widget = static_cast<TextWidget&>(GetOwningWidget());

    const std::shared_ptr<Font> font = widget.GetFont();
    if (font == nullptr)
    {
        return;
    }

    const DirectX::SpriteFont* spriteFont = font->GetSpriteFont(widget.GetFontType());
    if (spriteFont == nullptr)
    {
        return;
    }

    Vector2 offset = Vector2::Zero;
    switch (widget.GetFormatting())
    {
        case ETextFormatting::Center:
        {
            offset = spriteFont->MeasureString(widget.GetText().c_str());
            offset /= 2.0f;
            offset *= -1.0f;
            break;
        }
        case ETextFormatting::Left:
        {
            // todo
            break;
        }
        case ETextFormatting::Right:
        {
            // todo
            offset = spriteFont->MeasureString(widget.GetText().c_str());
            offset.x *= -1.0f;
            break;
        }
    }

    const std::shared_ptr<Window> parentWindow = widget.GetParentWindow();

    const Transform2D transformWS = widget.GetTransformWS();

    const Vector2 position = UIStatics::ToScreenSpace(transformWS.GetPosition(), parentWindow) + offset;
    const Vector2 scale = Vector2(widget.GetSizeWS().x);    // todo fix this
    const float rotation = transformWS.GetRotation();

    _transform.SetPosition(position);
    _transform.SetScale(scale);
    _transform.SetRotation(rotation);
}

void DX12TextWidgetRenderingProxy::SetupDrawingInternal(ID3D12GraphicsCommandList* commandList) const
{
    DX12WidgetRenderingProxy::SetupDrawingInternal(commandList);
    
    commandList->RSSetScissorRects(1, &GetOwningWidget().GetRect());

    const TextWidget& widget = static_cast<TextWidget&>(GetOwningWidget());

    const std::shared_ptr<Font> font = widget.GetFont();
    if (font == nullptr)
    {
        return;
    }

    const DirectX::SpriteFont* spriteFont = font->GetSpriteFont(widget.GetFontType());
    if (spriteFont == nullptr)
    {
        return;
    }

    _spriteBatch->Begin(commandList);

    spriteFont->DrawString(_spriteBatch.get(), widget.GetText().c_str(), _transform.GetPosition(), widget.GetTextColor(), _transform.GetRotation(), Vector2::Zero, _transform.GetScale());

    _spriteBatch->End();
}

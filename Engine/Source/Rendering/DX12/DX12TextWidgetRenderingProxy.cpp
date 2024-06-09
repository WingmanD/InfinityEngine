#include "DX12TextWidgetRenderingProxy.h"
#include "DX12RenderingSubsystem.h"
#include "DX12Window.h"
#include "RenderTargetState.h"
#include "ResourceUploadBatch.h"
#include "SpriteBatch.h"
#include "Rendering/Widgets/TextBox.h"
#include "Rendering/Widgets/Widget.h"

DX12CommandList DX12TextWidgetRenderingProxy::SetupDrawingInternal(DX12CommandList& commandList) const
{
    const TextBox& widget = static_cast<TextBox&>(GetOwningWidget());

    DX12CommandList currentCommandList = commandList;
    if (widget.IsBackgroundVisible())
    {
        currentCommandList = DX12WidgetRenderingProxy::SetupDrawingInternal(currentCommandList);
    }

    currentCommandList.CommandList->RSSetScissorRects(1, &GetOwningWidget().GetRect());

    const std::shared_ptr<Font> font = widget.GetFont();
    if (font == nullptr)
    {
        return currentCommandList;
    }

    const DirectX::SpriteFont* spriteFont = font->GetSpriteFont(widget.GetFontType());
    if (spriteFont == nullptr)
    {
        return currentCommandList;
    }

    _spriteBatch->Begin(currentCommandList.CommandList.Get());

    const Transform2D& transform = widget.GetTextTransform();

    spriteFont->DrawString(_spriteBatch.get(), widget.GetText().c_str(), transform.GetPosition(), widget.GetTextColor(), transform.GetRotation(), widget.GetTextOrigin(), transform.GetScale());

    _spriteBatch->End();

    return currentCommandList;
}

bool DX12TextWidgetRenderingProxy::Initialize()
{
    const DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();

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
            window->OnViewportChanged.Remove(_viewportChangedHandle);
        }
    }

    if (DX12Window* window = static_cast<DX12Window*>(newWindow.get()))
    {
        _spriteBatch->SetViewport(window->GetViewport());

        DelegateHandle handle = window->OnViewportChanged.Add([this](const D3D12_VIEWPORT& viewport)
        {
            _spriteBatch->SetViewport(viewport);
        });
        std::ignore = GetOwningWidget().OnDestroyed.Add([newWindow, handle]()
        {
            if (newWindow != nullptr)
            {
               static_cast<DX12Window*>(newWindow.get())->OnViewportChanged.Remove(handle);
            }
        });
    }
}

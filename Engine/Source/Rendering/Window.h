#pragma once

#include "Core.h"
#include "MulticastDelegate.h"
#include "MaterialParameterTypes.h"
#include "NonCopyable.h"
#include "Containers/Spatialization/HitTestGrid.h"
#include "Widgets/Widget.h"
#include <memory>

struct CD3DX12_VIEWPORT;
class RenderingSubsystem;

class Window : public NonCopyable<Window>, public std::enable_shared_from_this<Window>
{
public:
    enum class WindowState
    {
        BeingResized,
        Windowed,
        Minimized,
        Maximized,
        Fullscreen,
        FullscreenBorderless
    };

    struct Layer : public NonCopyable<Layer>
    {
        SharedObjectPtr<Widget> RootWidget;
        HitTestGrid<std::weak_ptr<Widget>> HitTestGrid{
            0.1f * 1080.0f, 1920.0f / 1080.0f * 2.0f, 2.0f, Vector2(1920.0f / 1080.0f, 1.0f)
        };
    };

    Window(uint32 width, uint32 height, std::wstring title);
    virtual ~Window() = default;

    virtual bool Initialize();
    void Tick(double deltaTime);

    uint32 GetWidth() const;
    uint32 GetHeight() const;
    const Vector2& GetSize() const;
    float GetAspectRatio() const;
    Vector2 GetPosition() const;

    void SetState(WindowState state);
    WindowState GetState() const;

    void SetTitle(const std::wstring& title);
    const std::wstring& GetTitle() const;

    SharedObjectPtr<WindowGlobals>& GetWindowGlobals();

    HitTestGrid<std::weak_ptr<Widget>>* GetHitTestGridFor(const SharedObjectPtr<Widget>& widget);

    void RequestResize(uint32 width, uint32 height);

    SharedObjectPtr<Layer> GetTopLayer() const;
    const std::vector<SharedObjectPtr<Layer>>& GetLayers() const;

    SharedObjectPtr<Layer> AddLayer();
    bool AddPopup(const SharedObjectPtr<Widget>& popup);
    bool AddBorrowedPopup(const SharedObjectPtr<Widget>& popup);

    template <typename T>
    SharedObjectPtr<Widget> AddPopup()
    {
        const SharedObjectPtr<Layer> newLayer = AddLayer();
        SharedObjectPtr<Widget> popup = newLayer->RootWidget->AddChild<T>();
        if (popup == nullptr)
        {
            return nullptr;
        }

        std::ignore = popup->OnDestroyed.Add([this, weakLayer = std::weak_ptr(newLayer)]()
        {
            auto it = std::ranges::find_if(_layers, [weakLayer](const SharedObjectPtr<Layer>& layer)
            {
                return layer == weakLayer.lock();
            });

            if (it != _layers.end())
            {
                _layers.erase(it);
            }
        });

        return popup;
    }

    Widget* GetWidgetAt(const Vector2& positionWS) const;
    Widget* GetWidgetUnderCursor();

    bool IsFocused() const;
    void SetFocusedWidget(const SharedObjectPtr<Widget>& widget);
    SharedObjectPtr<Widget> GetFocusedWidget() const;

    HWND GetHandle() const;

    bool IsValid() const;

    void Destroy();

protected:
    struct PendingResize
    {
        bool IsValid = false;
        uint32 Width = 0u;
        uint32 Height = 0u;
    };

    PendingResize& GetPendingResize();

protected:
    void SetIsFocused(bool value);

    virtual void OnStateChanged();
    virtual void OnTitleChanged();
    virtual void OnDestroyed();

    void OnResized();

private:
    PendingResize _pendingResize{};

    std::vector<SharedObjectPtr<Layer>> _layers;

    std::weak_ptr<Widget> _pressedWidget;
    std::weak_ptr<Widget> _hoveredWidget;
    std::weak_ptr<Widget> _focusedWidget;

    DelegateHandle _onLMBDownHandle{};
    DelegateHandle _onLMBUpHandle{};
    DelegateHandle _onMMBDownHandle{};
    DelegateHandle _onMMBUpHandle{};
    DelegateHandle _onScrollHandle{};
    DelegateHandle _onMouseMovedHandle{};
    DelegateHandle _onEscPressedHandle{};

    HWND _hwnd = nullptr;

    uint32 _width = 1920u;
    uint32 _height = 1080u;
    Vector2 _size = Vector2(1920.0f, 1080.0f);
    float _aspectRatio = 16.0f / 9.0f;
    std::wstring _title = L"InfinityEngine";

    bool _isFocused = false;
    WindowState _state = WindowState::Windowed;

    SharedObjectPtr<WindowGlobals> _windowGlobals = nullptr;

private:
    LRESULT ProcessWindowMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ProcessWindowMessagesStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

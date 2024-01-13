#pragma once

#include "Core.h"
#include "Delegate.h"
#include "MaterialParameterTypes.h"
#include "NonCopyable.h"
#include "Spatialization/HitTestGrid.h"
#include "Widgets/Widget.h"

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

    Window(uint32 width, uint32 height, std::wstring title);
    virtual ~Window() = default;

    virtual bool Initialize();
    void Tick(double deltaTime);

    uint32 GetWidth() const;
    uint32 GetHeight() const;
    const Vector2& GetSize() const;
    float GetAspectRatio() const;

    void SetState(WindowState state);
    WindowState GetState() const;

    void SetTitle(const std::wstring& title);
    const std::wstring& GetTitle() const;

    std::shared_ptr<WindowGlobals>& GetWindowGlobals();

    HitTestGrid<Widget*>& GetHitTestGrid();

    void RequestResize(uint32 width, uint32 height);

    Widget* GetRootWidget() const;
    Widget* GetWidgetAt(const Vector2& positionWS);
    Widget* GetWidgetUnderCursor();

    bool IsFocused() const;

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

    std::shared_ptr<Widget> _rootWidget = nullptr;
    HitTestGrid<Widget*> _hitTestGrid{0.1f * 1080.0f, 1920.0f / 1080.0f * 2.0f, 2.0f, Vector2(1920.0f / 1080.0f, 1.0f)};
    
    std::weak_ptr<Widget> _pressedWidget;
    std::weak_ptr<Widget> _hoveredWidget;
    std::weak_ptr<Widget> _focusedWidget;

    DelegateHandle _onLMBDownHandle{};
    DelegateHandle _onLMBUpHandle{};
    DelegateHandle _onMouseMovedHandle{};

    HWND _hwnd = nullptr;

    uint32 _width = 1920u;
    uint32 _height = 1080u;
    Vector2 _size = Vector2(1920.0f, 1080.0f);
    float _aspectRatio = 16.0f / 9.0f;
    std::wstring _title = L"InfinityEngine";

    bool _isFocused = false;
    WindowState _state = WindowState::Windowed;

    std::shared_ptr<WindowGlobals> _windowGlobals = nullptr;

private:
    LRESULT ProcessWindowMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ProcessWindowMessagesStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#pragma once

#include "Core.h"
#include "MaterialParameterTypes.h"
#include "NonCopyable.h"
#include "Widget.h"

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

    uint32 GetWidth() const;
    uint32 GetHeight() const;
    float GetAspectRatio() const;

    void SetState(WindowState state);
    WindowState GetState() const;

    void SetTitle(const std::wstring& title);
    const std::wstring& GetTitle() const;
    
    std::shared_ptr<WindowGlobals>& GetWindowGlobals();

    void RequestResize(uint32 width, uint32 height);

    Widget* GetRootWidget() const;

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

    HWND _hwnd = nullptr;

    uint32 _width = 1920u;
    uint32 _height = 1080u;
    float _aspectRatio = 16.0f / 9.0f;
    std::wstring _title = L"InfinityEngine";

    bool _isFocused = false;
    WindowState _state = WindowState::Windowed;

    std::shared_ptr<WindowGlobals> _windowGlobals = nullptr;

private:
    LRESULT ProcessWindowMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ProcessWindowMessagesStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

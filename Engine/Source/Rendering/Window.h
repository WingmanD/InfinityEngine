#pragma once

#include "Core.h"

class RenderingSubsystem;

class Window
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

    Window(RenderingSubsystem* renderingSubsystem, uint32 width, uint32 height, std::wstring title);
    virtual bool Initialize();

    uint32 GetWidth() const;
    uint32 GetHeight() const;

    void SetState(WindowState state);
    WindowState GetState() const;

    void SetTitle(const std::wstring& title);
    const std::wstring& GetTitle() const;

    void RequestResize(uint32 width, uint32 height);

    bool IsFocused() const;

    HWND GetHandle() const;

    bool IsValid() const;
    
    void Destroy();

    RenderingSubsystem* GetRenderingSubsystem() const;

protected:
    struct PendingResize
    {
        bool IsValid = false;
        uint32 Width;
        uint32 Height;
    };

    PendingResize& GetPendingResize();

protected:
    void SetIsFocused(bool value);

    virtual void OnStateChanged();
    virtual void OnTitleChanged();
    virtual void OnDestroyed();

    void SetWidth(float value);
    void SetHeight(float value);
    
private:
    PendingResize _pendingResize;
    
    HWND _hwnd = nullptr;

    uint32 _width = 1920u;
    uint32 _height = 1080u;
    std::wstring _title = L"SwarmEngine";

    bool _isFocused = false;
    WindowState _state = WindowState::Windowed;

    RenderingSubsystem* _renderingSubsystem = nullptr;

private:
    LRESULT ProcessWindowMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ProcessWindowMessagesStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

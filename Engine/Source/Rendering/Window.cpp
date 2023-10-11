#include "Window.h"

#include "Engine/Engine.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

Window::Window(RenderingSubsystem* renderingSubsystem, uint32 width, uint32 height, std::wstring title) :
    _width(width),
    _height(height),
    _title(std::move(title)),
    _renderingSubsystem(renderingSubsystem)
{
}

bool Window::Initialize()
{
    const HINSTANCE hInstance = Engine::Get().GetHandle();

    // todo registering class should be done only once
    WNDCLASSEX windowClass = {0};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = ProcessWindowMessagesStatic;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(hInstance, IDC_ARROW);
    windowClass.lpszClassName = L"SwarmEngineWindow";

    RegisterClassEx(&windowClass);

    RECT rectangle = {0, 0, static_cast<LONG>(_width), static_cast<LONG>(_height)};
    AdjustWindowRect(&rectangle, WS_OVERLAPPEDWINDOW, false);
    const int32 width = rectangle.right - rectangle.left;
    const int32 height = rectangle.bottom - rectangle.top;

    _hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        windowClass.lpszClassName,
        _title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        nullptr,
        nullptr,
        hInstance,
        this);

    if (_hwnd == nullptr)
    {
        MessageBox(nullptr, L"CreateWindow Failed.", nullptr, 0);
        return false;
    }

    SetWindowLongPtr(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    ShowWindow(_hwnd, SW_SHOW);
    UpdateWindow(_hwnd);

    return true;
}

uint32 Window::GetWidth() const
{
    return _width;
}

uint32 Window::GetHeight() const
{
    return _height;
}

void Window::Destroy()
{
    SetWindowLongPtr(GetHandle(), GWLP_USERDATA, NULL);
    if (DestroyWindow(GetHandle()))
    {
        _hwnd = nullptr;
        OnDestroyed();
    }
}

RenderingSubsystem* Window::GetRenderingSubsystem() const
{
    return _renderingSubsystem;
}

void Window::SetState(WindowState state)
{
    if (_state == state)
    {
        return;
    }

    _state = state;
    OnStateChanged();
}

Window::WindowState Window::GetState() const
{
    return _state;
}

void Window::SetTitle(const std::wstring& title)
{
    _title = title;

    OnTitleChanged();
}

const std::wstring& Window::GetTitle() const
{
    return _title;
}

void Window::RequestResize(uint32 width, uint32 height)
{
    _pendingResize.Width = width;
    _pendingResize.Height = height;
    _pendingResize.IsValid = true;
}

void Window::OnDestroyed()
{
    GetRenderingSubsystem()->OnWindowDestroyed(this);
}

void Window::SetWidth(float value)
{
    _width = value;
}

void Window::SetHeight(float value)
{
    _height = value;
}

bool Window::IsFocused() const
{
    return _isFocused;
}

HWND Window::GetHandle() const
{
    return _hwnd;
}

bool Window::IsValid() const
{
    return _hwnd != nullptr;
}

Window::PendingResize& Window::GetPendingResize()
{
    return _pendingResize;
}

void Window::SetIsFocused(bool value)
{
    _isFocused = value;
}

void Window::OnStateChanged()
{
    if (_state == WindowState::BeingResized)
    {
        SetIsFocused(false);
    }

    switch (_state)
    {
    case WindowState::BeingResized:
        {
            SetIsFocused(false);
            break;
        }
    }
}

void Window::OnTitleChanged()
{
    if (!IsValid())
    {
        TRACE_LOG("Window is not valid!");
        return;
    }

    SetWindowText(GetHandle(), _title.c_str());
}

LRESULT Window::ProcessWindowMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        {
            const auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
            return 0;
        }
    case WM_ACTIVATE:
        {
            SetIsFocused(LOWORD(wParam) != WA_INACTIVE);

            return 0;
        }

    case WM_ENTERSIZEMOVE:
        {
            SetState(WindowState::BeingResized);

            return 0;
        }
    case WM_EXITSIZEMOVE:
        {
            SetState(WindowState::Windowed);

            return 0;
        }
    case WM_SIZE:
        {
            const WindowState previousState = GetState();

            const int32 width = LOWORD(lParam);
            const int32 height = HIWORD(lParam);

            if (wParam == SIZE_MINIMIZED)
            {
                SetState(WindowState::Minimized);
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                SetState(WindowState::Maximized);

                RequestResize(width, height);
            }
            else if (wParam == SIZE_RESTORED)
            {
                if (previousState == WindowState::Minimized || previousState == WindowState::Maximized)
                {
                    SetState(WindowState::Windowed);
                }

                RequestResize(width, height);

                return 0;
            }

            return 0;
        }
    case WM_CLOSE:
        [[fallthrough]];
    case WM_DESTROY:
        {
            Destroy();
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT Window::ProcessWindowMessagesStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (window != nullptr)
    {
        return window->ProcessWindowMessages(hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

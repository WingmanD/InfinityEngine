#include "Window.h"

#include "StaticMesh.h"
#include "Engine/Engine.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

Window::Window(uint32 width, uint32 height, std::wstring title) :
    _width(width),
    _height(height),
    _aspectRatio(static_cast<float>(width) / static_cast<float>(height)),
    _title(std::move(title))
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
    windowClass.lpszClassName = L"InfinityEngineWindow";

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

    _rootWidget = std::make_unique<Widget>();
    if (!_rootWidget->Initialize())
    {
        return false;
    }

    _rootWidget->SetWindow(shared_from_this());

    // todo temporary
    _rootWidget->SetAnchor(EWidgetAnchor::TopLeft);
    _rootWidget->SetPosition({0.045f, -0.035f});
    _rootWidget->SetSize({0.15f, 0.05f});

    // todo fix this
    // _rootWidget->SetRotation(45.0f);
    // _rootWidget->SetSize({0.15f, 0.05f});
    
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

float Window::GetAspectRatio() const
{
    return _aspectRatio;
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

Widget* Window::GetRootWidget() const
{
    return _rootWidget.get();
}

void Window::OnDestroyed()
{
    RenderingSubsystem::Get().OnWindowDestroyed(this);
}

void Window::OnResized()
{
    _width = _pendingResize.Width;
    _height = _pendingResize.Height;
    _aspectRatio = static_cast<float>(_width) / static_cast<float>(_height);

    if (_rootWidget != nullptr)
    {
        _rootWidget->OnParentResized();
    }
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
        default:
            break;
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

        case WM_KEYUP:
        {
            if (wParam == VK_F1)
            {
                AssetManager& assetManager = Engine::Get().GetAssetManager();
                assetManager.ImportFromDialog(StaticMesh::StaticType());
            }

            return 0;
        }
        default:
            break;
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

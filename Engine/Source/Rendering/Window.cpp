#include "Window.h"

#include <memory>

#include "Font.h"
#include "StaticMesh.h"
#include "Engine/Engine.h"
#include "Engine/Subsystems/InputSubsystem.h"
#include "Engine/Subsystems/RenderingSubsystem.h"
#include "Widgets/TextWidget.h"
#include "Widgets/UIStatics.h"
#include "Widgets/FlowBox.h"

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
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
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
        LOG(L"Failed to create new window!");
        return false;
    }

    SetWindowLongPtr(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    ShowWindow(_hwnd, SW_SHOW);
    UpdateWindow(_hwnd);

    _windowGlobals = std::make_unique<WindowGlobals>();
    if (!_windowGlobals->Initialize())
    {
        LOG(L"Failed to initialize global window material parameters!");
        return false;
    }

    _rootWidget = std::make_shared<Widget>();
    if (!_rootWidget->Initialize())
    {
        return false;
    }

    _rootWidget->SetCollisionEnabled(false);
    _rootWidget->SetVisibility(false);
    _rootWidget->SetWindow(shared_from_this());
    _rootWidget->SetIgnoreChildDesiredSize(true);
    _rootWidget->SetDesiredSize({_aspectRatio * 2.0f, 2.0f});
    _rootWidget->SetSize({_aspectRatio * 2.0f, 2.0f});

    {
        const std::shared_ptr<FlowBox> menuWidget = std::make_shared<FlowBox>();
        menuWidget->Initialize();
        menuWidget->SetDirection(EFlowBoxDirection::Vertical);
        _rootWidget->AddChild(menuWidget);
        menuWidget->SetDesiredSize({0.33f, 0.66f});
        menuWidget->SetCollisionEnabled(false);
        {
            WidgetPerPassConstants* param = menuWidget->GetMaterial()->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
            param->BaseColor = Color(0.4f, 0.4f, 0.4f, 1.0f);
        }
        {
            const std::shared_ptr<Widget> button = std::make_shared<Widget>();
            button->Initialize();
            //button->SetCollisionEnabled(false);
            menuWidget->AddChild(button);
        
            const std::shared_ptr<TextWidget> textWidget = std::make_shared<TextWidget>();
            textWidget->Initialize();
            button->AddChild(textWidget);
            textWidget->SetCollisionEnabled(false);
            textWidget->SetFont(AssetManager::Get().FindAssetByName<Font>(L"Arial"));
            textWidget->SetText(L"Main Menu");
            textWidget->SetTextColor({0.9f, 0.9f, 0.9f, 1.0f});
            textWidget->SetPadding({0.01f, 0.01f, 0.005f, 0.005f});
        }
        {
            const std::shared_ptr<Widget> button = std::make_shared<Widget>();
            button->Initialize();
            menuWidget->AddChild(button);
        
            const std::shared_ptr<TextWidget> textWidget = std::make_shared<TextWidget>();
            textWidget->Initialize();
            button->AddChild(textWidget);
            textWidget->SetCollisionEnabled(false);
            textWidget->SetFont(AssetManager::Get().FindAssetByName<Font>(L"Arial"));
            textWidget->SetText(L"Hello World!");
            textWidget->SetTextColor({0.9f, 0.9f, 0.9f, 1.0f});
            textWidget->SetPadding({0.01f, 0.01f, 0.005f, 0.005f});
        }

        {
            const std::shared_ptr<Widget> button = std::make_shared<Widget>();
            button->Initialize();
            menuWidget->AddChild(button);
        
            const std::shared_ptr<TextWidget> textWidget = std::make_shared<TextWidget>();
            textWidget->Initialize();
            button->AddChild(textWidget);
            textWidget->SetCollisionEnabled(false);
            textWidget->SetFont(AssetManager::Get().FindAssetByName<Font>(L"Arial"));
            textWidget->SetText(L"Second Button");
            textWidget->SetTextColor({0.9f, 0.9f, 0.9f, 1.0f});
            textWidget->SetPadding({0.01f, 0.01f, 0.005f, 0.005f});
        }
    }

    {
        const std::shared_ptr<Widget> newWidget = std::make_shared<Widget>();
        newWidget->Initialize();
        _rootWidget->AddChild(newWidget);
        newWidget->SetSize({0.2f, 0.1f});
        newWidget->SetAnchor(EWidgetAnchor::TopLeft);
        newWidget->SetPosition({0.1f, -0.05f});
    
        const std::shared_ptr<TextWidget> textWidget = std::make_shared<TextWidget>();
        textWidget->Initialize();
        newWidget->AddChild(textWidget);
        textWidget->SetCollisionEnabled(false);
        textWidget->SetFont(AssetManager::Get().FindAssetByName<Font>(L"Arial"));
        textWidget->SetText(L"Hello World!");
        textWidget->SetTextColor({0.9f, 0.9f, 0.9f, 1.0f});
        textWidget->SetPadding({0.01f, 0.01f, 0.005f, 0.005f});
    }
    
    {
        const std::shared_ptr<Widget> newWidget = std::make_shared<Widget>();
        newWidget->Initialize();
        _rootWidget->AddChild(newWidget);
        newWidget->SetSize({0.2f, 0.1f});
        newWidget->SetAnchor(EWidgetAnchor::TopRight);
        newWidget->SetPosition({-0.1f, -0.05f});
    }

    InputSubsystem& inputSubsystem = InputSubsystem::Get();
    inputSubsystem.SetFocusedWindow(shared_from_this(), {});

    _onLMBDownHandle = inputSubsystem.OnMouseLeftButtonDown.Subscribe([this]()
    {
        const Vector2 mousePositionWS = UIStatics::ToWidgetSpace(InputSubsystem::Get().GetMousePosition(), shared_from_this());

        Widget** hitWidgetPtr = _hitTestGrid.FindAtByPredicate(mousePositionWS,
                                                               [](const Vector2& positionWS, const Widget* widget)
                                                               {
                                                                   return widget->GetBoundingBox().Contains(positionWS);
                                                               });

        if (hitWidgetPtr != nullptr)
        {
            if (Widget* hitWidget = *hitWidgetPtr)
            {
                _interactedWidget = hitWidget->SharedFromThis();
                hitWidget->OnPressed({});
            }
        }
    });

    _onLMBUpHandle = inputSubsystem.OnMouseLeftButtonUp.Subscribe([this]()
    {
        if (const std::shared_ptr<Widget> interactedWidget = _interactedWidget.lock())
        {
            interactedWidget->OnReleased({});
            _interactedWidget.reset();

            return;
        }

        const Vector2 mousePositionWS = UIStatics::ToWidgetSpace(InputSubsystem::Get().GetMousePosition(), shared_from_this());

        Widget** hitWidgetPtr = _hitTestGrid.FindAtByPredicate(mousePositionWS,
                                                               [](const Vector2& positionWS, const Widget* widget)
                                                               {
                                                                   return widget->GetBoundingBox().Contains(positionWS);
                                                               });

        if (hitWidgetPtr != nullptr)
        {
            if (Widget* hitWidget = *hitWidgetPtr)
            {
                hitWidget->OnReleased({});
            }
        }
    });

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

std::shared_ptr<WindowGlobals>& Window::GetWindowGlobals()
{
    return _windowGlobals;
}

HitTestGrid<Widget*>& Window::GetHitTestGrid()
{
    return _hitTestGrid;
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
    InputSubsystem::Get().OnMouseLeftButtonDown.Unsubscribe(_onLMBDownHandle);
    InputSubsystem::Get().OnMouseLeftButtonUp.Unsubscribe(_onLMBUpHandle);
}

void Window::OnResized()
{
    _width = _pendingResize.Width;
    _height = _pendingResize.Height;
    _aspectRatio = static_cast<float>(_width) / static_cast<float>(_height);

    _windowGlobals->ResolutionX = static_cast<uint16>(_width);
    _windowGlobals->ResolutionY = static_cast<uint16>(_height);
    _windowGlobals->AspectRatio = _aspectRatio;
    _windowGlobals->MarkAsDirty();

    _hitTestGrid = HitTestGrid<Widget*>(0.1f * _height / 1080.0f, _aspectRatio * 2.0f, 2.0f, Vector2(_aspectRatio, 1.0f));

    if (_rootWidget != nullptr)
    {
        _rootWidget->SetSize({_aspectRatio * 2.0f, 2.0f});
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

    if (_isFocused)
    {
        InputSubsystem::Get().SetFocusedWindow(shared_from_this(), {});
    }
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
        default:
        {
            return InputSubsystem::Get().ProcessWindowMessages(hwnd, msg, wParam, lParam, shared_from_this(), {});
        }
    }
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

#include "Window.h"
#include "Font.h"
#include "StaticMesh.h"
#include "Engine/Engine.h"
#include "Engine/Subsystems/InputSubsystem.h"
#include "Engine/Subsystems/RenderingSubsystem.h"
#include "Widgets/CanvasPanel.h"
#include "Widgets/UIStatics.h"
#include <memory>

class TabSwitcher;

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

    InputSubsystem& inputSubsystem = InputSubsystem::Get();
    inputSubsystem.SetFocusedWindow(shared_from_this(), {});

    _onLMBDownHandle = inputSubsystem.OnMouseLeftButtonDown.Add([this, &inputSubsystem]()
    {
        if (inputSubsystem.IsMouseCaptured())
        {
            return;
        }
        
        if (_pressedWidget.lock() != nullptr)
        {
            return;
        }

        Widget* hitWidget = GetWidgetUnderCursor();
        if (const SharedObjectPtr<Widget> focusedWidget = GetFocusedWidget())
        {
            if (focusedWidget.get() != hitWidget)
            {
                focusedWidget->SetFocused(false);
            }
        }

        if (hitWidget != nullptr)
        {
            _pressedWidget = hitWidget->SharedFromThis();
            hitWidget->SetFocused(true);
            hitWidget->CallPressed({});
        }
    });

    _onLMBUpHandle = inputSubsystem.OnMouseLeftButtonUp.Add([this, &inputSubsystem]()
    {
        if (const SharedObjectPtr<Widget> interactedWidget = _pressedWidget.lock())
        {
            interactedWidget->CallReleased({});
            _pressedWidget.reset();

            return;
        }

        if (Widget* hitWidget = GetWidgetUnderCursor())
        {
            hitWidget->CallReleased({});
        }
    });

    _onMMBDownHandle = inputSubsystem.OnMouseMiddleButtonDown.Add([this, &inputSubsystem]()
    {
        if (inputSubsystem.IsMouseCaptured())
        {
            return;
        }
        
        Widget* hitWidget = GetWidgetUnderCursor();
        if (hitWidget != nullptr)
        {
            hitWidget->CallMiddleClickPressed({});
        }
    });

    _onMMBUpHandle = inputSubsystem.OnMouseMiddleButtonUp.Add([this, &inputSubsystem]()
    {
        if (inputSubsystem.IsMouseCaptured())
        {
            return;
        }
        
        if (Widget* hitWidget = GetWidgetUnderCursor())
        {
            hitWidget->CallMiddleClickReleased({});
        }
    });

    _onScrollHandle = inputSubsystem.OnMouseWheelScroll.Add([this, &inputSubsystem](int32 value)
    {
        if (inputSubsystem.IsMouseCaptured())
        {
            return;
        }
        
        if (Widget* hitWidget = GetWidgetUnderCursor())
        {
            hitWidget->CallScrolled(value, {});
        }
    });

    _onMouseMovedHandle = inputSubsystem.OnMouseMoved.Add([this, &inputSubsystem](const Vector2 mousePosition)
    {
        if (inputSubsystem.IsMouseCaptured())
        {
            return;
        }
        
        const SharedObjectPtr<Widget> previousHoveredWidget = _hoveredWidget.lock();

        Widget* hitWidget = GetWidgetUnderCursor();
        if (previousHoveredWidget != nullptr && hitWidget != previousHoveredWidget.get() || hitWidget == nullptr &&
            previousHoveredWidget != nullptr)
        {
            previousHoveredWidget->CallHoverEnded({});
            _hoveredWidget.reset();
        }

        if (hitWidget != nullptr && hitWidget != _hoveredWidget.lock().get())
        {
            _hoveredWidget = hitWidget->SharedFromThis();
            hitWidget->CallHoverStarted({});
        }
    });

    _onEscPressedHandle = inputSubsystem.GetKey(EKey::Escape).OnKeyUp.Add([this]()
    {
        SetFocusedWidget(nullptr);
    });

    return true;
}

void Window::Tick(double deltaTime)
{
    for (const SharedObjectPtr<Layer>& layer : _layers)
    {
        layer->RootWidget->RebuildLayout();
    }
}

uint32 Window::GetWidth() const
{
    return _width;
}

uint32 Window::GetHeight() const
{
    return _height;
}

const Vector2& Window::GetSize() const
{
    return _size;
}

float Window::GetAspectRatio() const
{
    return _aspectRatio;
}

Vector2 Window::GetPosition() const
{
    RECT rect;
    GetWindowRect(_hwnd, &rect);

    return {static_cast<float>(rect.left), static_cast<float>(rect.top)};
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

SharedObjectPtr<WindowGlobals>& Window::GetWindowGlobals()
{
    return _windowGlobals;
}

HitTestGrid<std::weak_ptr<Widget>>* Window::GetHitTestGridFor(
    const SharedObjectPtr<Widget>& widget)
{
    // todo this is inefficient
    auto rootWidget = widget->GetRootWidget();

    const auto it = std::ranges::find_if(_layers, [rootWidget](const SharedObjectPtr<Layer>& layer)
    {
        return layer->RootWidget == rootWidget;
    });

    if (it == _layers.end())
    {
        return nullptr;
    }

    const SharedObjectPtr<Layer> layer = *it;

    return &layer->HitTestGrid;
}


void Window::RequestResize(uint32 width, uint32 height)
{
    _pendingResize.Width = width;
    _pendingResize.Height = height;
    _pendingResize.IsValid = true;
}

SharedObjectPtr<Window::Layer> Window::GetTopLayer() const
{
    return _layers.empty() ? nullptr : _layers.back();
}

const std::vector<SharedObjectPtr<Window::Layer>>& Window::GetLayers() const
{
    return _layers;
}

SharedObjectPtr<Window::Layer> Window::AddLayer()
{
    const SharedObjectPtr<CanvasPanel> rootWidget = NewObject<CanvasPanel>();
    if (!rootWidget->Initialize())
    {
        return nullptr;
    }

    rootWidget->SetCollisionEnabled(false);
    rootWidget->SetVisibility(false);
    rootWidget->SetWindow(shared_from_this());
    rootWidget->SetDesiredSize({_aspectRatio * 2.0f, 2.0f});
    rootWidget->SetSize({_aspectRatio * 2.0f, 2.0f}); // todo reimport QuadMesh with 2x size to avoid this 2x scaling

    std::shared_ptr<Layer> newLayer = std::make_shared<Layer>();
    newLayer->RootWidget = rootWidget;
    newLayer->HitTestGrid = HitTestGrid<std::weak_ptr<Widget>>(0.1f * static_cast<float>(_height) / 1080.0f, _aspectRatio * 2.0f,
                                                 2.0f,
                                                 Vector2(_aspectRatio, 1.0f));
    _layers.push_back(newLayer);

    std::ignore = rootWidget->OnDestroyed.Add([this, weakLayer = std::weak_ptr(newLayer)]()
    {
        auto it = std::ranges::find_if(_layers, [weakLayer](const SharedObjectPtr<Layer>& layer)
        {
            return layer == weakLayer.lock();
        });
    });

    return newLayer;
}

bool Window::AddPopup(const SharedObjectPtr<Widget>& popup)
{
    if (popup == nullptr)
    {
        DEBUG_BREAK();
        return false;
    }

    const SharedObjectPtr<Layer> layer = AddLayer();
    if (layer == nullptr)
    {
        return false;
    }

    layer->RootWidget->AddChild(popup);

    auto removeLayer = [this, weakLayer = std::weak_ptr(layer)]()
    {
        const auto it = std::ranges::find_if(_layers, [weakLayer](const SharedObjectPtr<Layer>& layer)
        {
            return layer == weakLayer.lock();
        });

        if (it != _layers.end())
        {
            _layers.erase(it);
        }
    };

    // todo unsubscribe from events
    std::ignore = popup->OnCollapsed.Add(removeLayer);
    Widget* rawPopup = popup.get();
    std::ignore = popup->OnFocusChanged.Add([removeLayer, this, rawPopup](bool value)
    {
        if (!value)
        {
            const Widget* current = GetFocusedWidget().get();
            while (current != nullptr)
            {
                // If we focus a child of the current popup, we don't want to close the popup
                if (current == rawPopup)
                {
                    return;
                }
                
                current = current->GetParentWidget().get();
            }
            
            removeLayer();
        }
    });

    std::ignore = popup->OnDestroyed.Add(removeLayer);

    popup->SetFocused(true);

    return true;
}

bool Window::AddBorrowedPopup(const SharedObjectPtr<Widget>& popup)
{
    // todo refactor this, same as AddPopup
    if (popup == nullptr)
    {
        DEBUG_BREAK();
        return false;
    }

    const SharedObjectPtr<Layer> layer = AddLayer();
    if (layer == nullptr)
    {
        return false;
    }

    layer->RootWidget->AddChild(popup);

    auto removeLayer = [this, weakLayer = std::weak_ptr(layer), popup]()
    {
        const auto it = std::ranges::find_if(_layers, [weakLayer](const SharedObjectPtr<Layer>& layer)
        {
            return layer == weakLayer.lock();
        });

        if (it != _layers.end())
        {
            popup->RemoveFromParent();
            _layers.erase(it);
        }
    };

    // todo unsubscribe from events
    std::ignore = popup->OnCollapsed.Add(removeLayer);
    Widget* rawPopup = popup.get();
    std::ignore = popup->OnFocusChanged.Add([removeLayer, this, rawPopup](bool value)
    {
        if (!value)
        {
            const Widget* current = GetFocusedWidget().get();
            while (current != nullptr)
            {
                // If we focus a child of the current popup, we don't want to close the popup
                if (current == rawPopup)
                {
                    return;
                }
                
                current = current->GetParentWidget().get();
            }
            
            removeLayer();
        }
    });

    std::ignore = popup->OnDestroyed.Add(removeLayer);

    popup->SetFocused(true);

    return true;
}

Widget* Window::GetWidgetAt(const Vector2& positionWS) const
{
    if (positionWS.x < -static_cast<float>(_width) / 2.0f || positionWS.x > static_cast<float>(_width) / 2.0f ||
        positionWS.y < -static_cast<float>(_height) / 2.0f || positionWS.y > static_cast<float>(_height) / 2.0f)
    {
        return nullptr;
    }
    
    std::weak_ptr<Widget>* hitWidgetPtr = GetTopLayer()->HitTestGrid.FindAtByPredicate(positionWS,
    [](const Vector2& position, const std::weak_ptr<Widget> widget)
    {
        if (SharedObjectPtr<Widget> shared = widget.lock())
        {
            return shared->GetBoundingBox().Contains(position);
        }

        return false;
    });
    if (hitWidgetPtr != nullptr)
    {
        return hitWidgetPtr->lock().get();
    }

    return nullptr;
}

Widget* Window::GetWidgetUnderCursor()
{
    return GetWidgetAt(UIStatics::ToWidgetSpace(InputSubsystem::Get().GetMousePosition(), shared_from_this()));
}

void Window::OnDestroyed()
{
    RenderingSubsystem::Get().OnWindowDestroyed(this);
    InputSubsystem::Get().OnMouseLeftButtonDown.Remove(_onLMBDownHandle);
    InputSubsystem::Get().OnMouseLeftButtonUp.Remove(_onLMBUpHandle);
}

void Window::OnResized()
{
    _width = _pendingResize.Width;
    _height = _pendingResize.Height;
    _size = Vector2(static_cast<float>(_width), static_cast<float>(_height));
    _aspectRatio = static_cast<float>(_width) / static_cast<float>(_height);

    _windowGlobals->ResolutionX = static_cast<uint16>(_width);
    _windowGlobals->ResolutionY = static_cast<uint16>(_height);
    _windowGlobals->AspectRatio = _aspectRatio;
    _windowGlobals->MarkAsDirty();

    for (const SharedObjectPtr<Layer>& layer : _layers)
    {
        if (layer != nullptr)
        {
            layer->RootWidget->SetSize({_aspectRatio * 2.0f, 2.0f});

            layer->HitTestGrid = HitTestGrid<std::weak_ptr<Widget>>(0.1f * _height / 1080.0f, _aspectRatio * 2.0f, 2.0f,
                                                      Vector2(_aspectRatio, 1.0f));
            layer->RootWidget->ForceRebuildLayout(true);
        }
    }
}

bool Window::IsFocused() const
{
    return _isFocused;
}

void Window::SetFocusedWidget(const SharedObjectPtr<Widget>& widget)
{
    const SharedObjectPtr<Widget> focusedWidget = _focusedWidget.lock();
    if (focusedWidget != nullptr)
    {
        focusedWidget->SetFocused(false);
    }
    
    _focusedWidget = widget;
}

SharedObjectPtr<Widget> Window::GetFocusedWidget() const
{
    return _focusedWidget.lock();
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

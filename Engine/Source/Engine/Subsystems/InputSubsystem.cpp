﻿#include "InputSubsystem.h"
#include "Engine/Engine.h"
#include "Rendering/Window.h"

InputSubsystem& InputSubsystem::Get()
{
    return Engine::Get().GetInputSubsystem();
}

bool InputSubsystem::IsKeyDown(EKey key) const
{
    if (key == EKey::None)
    {
        DEBUG_BREAK();
        return false;
    }

    return _keyStates.at(key).IsDown;
}

KeyState& InputSubsystem::GetKey(EKey key)
{
    if (key == EKey::None)
    {
        DEBUG_BREAK();
    }

    return _keyStates.at(key);
}

DirectX::Mouse& InputSubsystem::GetMouse() const
{
    return *_mouse.get();
}

LRESULT InputSubsystem::ProcessWindowMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, const std::shared_ptr<Window>& window, PassKey<Window>)
{
    {
        using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;
        _mouse->ProcessMessage(msg, wParam, lParam);
        _mouseButtonStateTracker.Update(_mouse->GetState());
    
        if (_mouseButtonStateTracker.leftButton == ButtonState::PRESSED)
        {
            _pendingDelegates.push_back(&OnMouseLeftButtonDown);
        }
        else if (_mouseButtonStateTracker.leftButton == ButtonState::RELEASED)
        {
            _pendingDelegates.push_back(&OnMouseLeftButtonUp);
        }
    
        if (_mouseButtonStateTracker.rightButton == ButtonState::PRESSED)
        {
            _pendingDelegates.push_back(&OnMouseRightButtonDown);
        }
        else if (_mouseButtonStateTracker.rightButton == ButtonState::RELEASED)
        {
            _pendingDelegates.push_back(&OnMouseRightButtonUp);
        }
    
        if (_mouseButtonStateTracker.middleButton == ButtonState::PRESSED)
        {
            _pendingDelegates.push_back(&OnMouseMiddleButtonDown);
        }
        else if (_mouseButtonStateTracker.middleButton == ButtonState::RELEASED)
        {
            _pendingDelegates.push_back(&OnMouseMiddleButtonUp);
        }
    
        if (_mouseButtonStateTracker.xButton1 == ButtonState::PRESSED)
        {
            _pendingDelegates.push_back(&OnMouseXButton1Down);
        }
        else if (_mouseButtonStateTracker.xButton1 == ButtonState::RELEASED)
        {
            _pendingDelegates.push_back(&OnMouseXButton1Up);
        }
    
        if (_mouseButtonStateTracker.xButton2 == ButtonState::PRESSED)
        {
            _pendingDelegates.push_back(&OnMouseXButton2Down);
        }
        else if (_mouseButtonStateTracker.xButton2 == ButtonState::RELEASED)
        {
            _pendingDelegates.push_back(&OnMouseXButton2Up);
        }
    
        if (_mouse->GetState().scrollWheelValue != 0)
        {
            _pendingDelegates.push_back(&OnMouseWheelScroll);
        }
    }

    switch (msg)
    {
        case WM_KEYDOWN:
        {
            const EKey key = ConvertKeyCode(wParam);
            KeyState& keyState = _keyStates[key];
            keyState.IsDown = true;
            _pendingDelegates.push_back(&keyState.OnKeyDown);
            
            return 0;
        }
        case WM_KEYUP:
        {
            const EKey key = ConvertKeyCode(wParam);
            _keyStates[key].IsDown = false;
            _pendingDelegates.push_back(&_keyStates[key].OnKeyUp);

            return 0;
        }
        default:
            break;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void InputSubsystem::SetFocusedWindow(const std::shared_ptr<Window>& window, PassKey<Window>)
{
    _focusedWindow = window;

    _mouse->SetWindow(window->GetHandle());
}

std::shared_ptr<Window> InputSubsystem::GetFocusedWindow() const
{
    return _focusedWindow.lock();
}

bool InputSubsystem::IsAlphaNumeric(EKey key)
{
    if (key >= EKey::A && key <= EKey::Nine)
    {
        return true;
    }

    if (key >= EKey::NumPad0 && key <= EKey::NumPad9)
    {
        return true;
    }

    return false;
}

bool InputSubsystem::Initialize()
{
    if (!EngineSubsystem::Initialize())
    {
        return false;
    }

    _mouse = std::make_unique<DirectX::Mouse>();

    _keyStates[EKey::Escape].IsDown = false;
    _keyStates[EKey::F1].IsDown = false;
    _keyStates[EKey::F2].IsDown = false;
    _keyStates[EKey::F3].IsDown = false;
    _keyStates[EKey::F4].IsDown = false;
    _keyStates[EKey::F5].IsDown = false;
    _keyStates[EKey::F6].IsDown = false;
    _keyStates[EKey::F7].IsDown = false;
    _keyStates[EKey::F8].IsDown = false;
    _keyStates[EKey::F9].IsDown = false;
    _keyStates[EKey::F10].IsDown = false;
    _keyStates[EKey::F11].IsDown = false;
    _keyStates[EKey::F12].IsDown = false;
    _keyStates[EKey::CTRL].IsDown = false;
    _keyStates[EKey::Shift].IsDown = false;
    _keyStates[EKey::ALT].IsDown = false;
    _keyStates[EKey::Space].IsDown = false;
    _keyStates[EKey::Enter].IsDown = false;
    _keyStates[EKey::Backspace].IsDown = false;
    _keyStates[EKey::Tab].IsDown = false;
    _keyStates[EKey::CapsLock].IsDown = false;
    _keyStates[EKey::Insert].IsDown = false;
    _keyStates[EKey::Delete].IsDown = false;
    _keyStates[EKey::Home].IsDown = false;
    _keyStates[EKey::End].IsDown = false;
    _keyStates[EKey::PageUp].IsDown = false;
    _keyStates[EKey::PageDown].IsDown = false;
    _keyStates[EKey::NumLock].IsDown = false;
    _keyStates[EKey::NumPad0].IsDown = false;
    _keyStates[EKey::NumPad1].IsDown = false;
    _keyStates[EKey::NumPad2].IsDown = false;
    _keyStates[EKey::NumPad3].IsDown = false;
    _keyStates[EKey::NumPad4].IsDown = false;
    _keyStates[EKey::NumPad5].IsDown = false;
    _keyStates[EKey::NumPad6].IsDown = false;
    _keyStates[EKey::NumPad7].IsDown = false;
    _keyStates[EKey::NumPad8].IsDown = false;
    _keyStates[EKey::NumPad9].IsDown = false;
    _keyStates[EKey::NumPadDivide].IsDown = false;
    _keyStates[EKey::NumPadMultiply].IsDown = false;
    _keyStates[EKey::NumPadSubtract].IsDown = false;
    _keyStates[EKey::NumPadAdd].IsDown = false;
    _keyStates[EKey::NumPadDecimal].IsDown = false;
    _keyStates[EKey::NumPadEnter].IsDown = false;
    _keyStates[EKey::PrintScreen].IsDown = false;
    _keyStates[EKey::ScrollLock].IsDown = false;
    _keyStates[EKey::Pause].IsDown = false;
    _keyStates[EKey::LeftShift].IsDown = false;
    _keyStates[EKey::RightShift].IsDown = false;
    _keyStates[EKey::LeftCTRL].IsDown = false;
    _keyStates[EKey::RightCTRL].IsDown = false;
    _keyStates[EKey::LeftALT].IsDown = false;
    _keyStates[EKey::RightALT].IsDown = false;
    _keyStates[EKey::LeftWindows].IsDown = false;
    _keyStates[EKey::RightWindows].IsDown = false;
    _keyStates[EKey::Sleep].IsDown = false;
    _keyStates[EKey::VolumeMute].IsDown = false;
    _keyStates[EKey::VolumeDown].IsDown = false;
    _keyStates[EKey::VolumeUp].IsDown = false;
    _keyStates[EKey::MediaNextTrack].IsDown = false;
    _keyStates[EKey::MediaPreviousTrack].IsDown = false;
    _keyStates[EKey::MediaStop].IsDown = false;
    _keyStates[EKey::MediaPlayPause].IsDown = false;
    _keyStates[EKey::Left].IsDown = false;
    _keyStates[EKey::Right].IsDown = false;
    _keyStates[EKey::Up].IsDown = false;
    _keyStates[EKey::Down].IsDown = false;
    _keyStates[EKey::A].IsDown = false;
    _keyStates[EKey::B].IsDown = false;
    _keyStates[EKey::C].IsDown = false;
    _keyStates[EKey::D].IsDown = false;
    _keyStates[EKey::E].IsDown = false;
    _keyStates[EKey::F].IsDown = false;
    _keyStates[EKey::G].IsDown = false;
    _keyStates[EKey::H].IsDown = false;
    _keyStates[EKey::I].IsDown = false;
    _keyStates[EKey::J].IsDown = false;
    _keyStates[EKey::K].IsDown = false;
    _keyStates[EKey::L].IsDown = false;
    _keyStates[EKey::M].IsDown = false;
    _keyStates[EKey::N].IsDown = false;
    _keyStates[EKey::O].IsDown = false;
    _keyStates[EKey::P].IsDown = false;
    _keyStates[EKey::Q].IsDown = false;
    _keyStates[EKey::R].IsDown = false;
    _keyStates[EKey::S].IsDown = false;
    _keyStates[EKey::T].IsDown = false;
    _keyStates[EKey::U].IsDown = false;
    _keyStates[EKey::V].IsDown = false;
    _keyStates[EKey::W].IsDown = false;
    _keyStates[EKey::X].IsDown = false;
    _keyStates[EKey::Y].IsDown = false;
    _keyStates[EKey::Z].IsDown = false;
    _keyStates[EKey::Zero].IsDown = false;
    _keyStates[EKey::One].IsDown = false;
    _keyStates[EKey::Two].IsDown = false;
    _keyStates[EKey::Three].IsDown = false;
    _keyStates[EKey::Four].IsDown = false;
    _keyStates[EKey::Five].IsDown = false;
    _keyStates[EKey::Six].IsDown = false;
    _keyStates[EKey::Seven].IsDown = false;
    _keyStates[EKey::Eight].IsDown = false;
    _keyStates[EKey::Nine].IsDown = false;
    _keyStates[EKey::Comma].IsDown = false;
    _keyStates[EKey::Period].IsDown = false;
    _keyStates[EKey::Slash].IsDown = false;
    _keyStates[EKey::Semicolon].IsDown = false;
    _keyStates[EKey::Apostrophe].IsDown = false;
    _keyStates[EKey::LeftBracket].IsDown = false;
    _keyStates[EKey::RightBracket].IsDown = false;
    _keyStates[EKey::Backslash].IsDown = false;
    _keyStates[EKey::Minus].IsDown = false;
    _keyStates[EKey::Equals].IsDown = false;

    return true;
}

void InputSubsystem::Tick(double deltaTime)
{
    for (Delegate<>*& delegate : _pendingDelegates)
    {
        delegate->Broadcast();
    }

    _pendingDelegates.clear();
}

EKey InputSubsystem::ConvertKeyCode(WPARAM virtualKeyCode) const
{
    return static_cast<EKey>(virtualKeyCode);
}
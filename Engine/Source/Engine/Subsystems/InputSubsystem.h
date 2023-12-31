#pragma once

#include <map>

#include "Core.h"
#include "EngineSubsystem.h"
#include "Mouse.h"
#include "PassKey.h"
#include "Rendering/Delegate.h"

class Window;

enum class EKey : uint8
{
    None               = 0,
    Escape             = VK_ESCAPE,
    F1                 = VK_F1,
    F2                 = VK_F2,
    F3                 = VK_F3,
    F4                 = VK_F4,
    F5                 = VK_F5,
    F6                 = VK_F6,
    F7                 = VK_F7,
    F8                 = VK_F8,
    F9                 = VK_F9,
    F10                = VK_F10,
    F11                = VK_F11,
    F12                = VK_F12,
    CTRL               = VK_CONTROL,
    Shift              = VK_SHIFT,
    ALT                = VK_MENU,
    Space              = VK_SPACE,
    Enter              = VK_RETURN,
    Backspace          = VK_BACK,
    Tab                = VK_TAB,
    CapsLock           = VK_CAPITAL,
    Insert             = VK_INSERT,
    Delete             = VK_DELETE,
    Home               = VK_HOME,
    End                = VK_END,
    PageUp             = VK_PRIOR,
    PageDown           = VK_NEXT,
    NumLock            = VK_NUMLOCK,
    NumPad0            = VK_NUMPAD0,
    NumPad1            = VK_NUMPAD1,
    NumPad2            = VK_NUMPAD2,
    NumPad3            = VK_NUMPAD3,
    NumPad4            = VK_NUMPAD4,
    NumPad5            = VK_NUMPAD5,
    NumPad6            = VK_NUMPAD6,
    NumPad7            = VK_NUMPAD7,
    NumPad8            = VK_NUMPAD8,
    NumPad9            = VK_NUMPAD9,
    NumPadDivide       = VK_DIVIDE,
    NumPadMultiply     = VK_MULTIPLY,
    NumPadSubtract     = VK_SUBTRACT,
    NumPadAdd          = VK_ADD,
    NumPadDecimal      = VK_DECIMAL,
    NumPadEnter        = VK_RETURN,
    PrintScreen        = VK_SNAPSHOT,
    ScrollLock         = VK_SCROLL,
    Pause              = VK_PAUSE,
    LeftShift          = VK_LSHIFT,
    RightShift         = VK_RSHIFT,
    LeftCTRL           = VK_LCONTROL,
    RightCTRL          = VK_RCONTROL,
    LeftALT            = VK_LMENU,
    RightALT           = VK_RMENU,
    LeftWindows        = VK_LWIN,
    RightWindows       = VK_RWIN,
    Sleep              = VK_SLEEP,
    VolumeMute         = VK_VOLUME_MUTE,
    VolumeDown         = VK_VOLUME_DOWN,
    VolumeUp           = VK_VOLUME_UP,
    MediaNextTrack     = VK_MEDIA_NEXT_TRACK,
    MediaPreviousTrack = VK_MEDIA_PREV_TRACK,
    MediaStop          = VK_MEDIA_STOP,
    MediaPlayPause     = VK_MEDIA_PLAY_PAUSE,
    Left               = VK_LEFT,
    Right              = VK_RIGHT,
    Up                 = VK_UP,
    Down               = VK_DOWN,
    A                  = 'A',
    B                  = 'B',
    C                  = 'C',
    D                  = 'D',
    E                  = 'E',
    F                  = 'F',
    G                  = 'G',
    H                  = 'H',
    I                  = 'I',
    J                  = 'J',
    K                  = 'K',
    L                  = 'L',
    M                  = 'M',
    N                  = 'N',
    O                  = 'O',
    P                  = 'P',
    Q                  = 'Q',
    R                  = 'R',
    S                  = 'S',
    T                  = 'T',
    U                  = 'U',
    V                  = 'V',
    W                  = 'W',
    X                  = 'X',
    Y                  = 'Y',
    Z                  = 'Z',
    Zero               = '0',
    One                = '1',
    Two                = '2',
    Three              = '3',
    Four               = '4',
    Five               = '5',
    Six                = '6',
    Seven              = '7',
    Eight              = '8',
    Nine               = '9',
    Comma              = VK_OEM_COMMA,
    Period             = VK_OEM_PERIOD,
    Slash              = VK_OEM_2,
    Semicolon          = VK_OEM_1,
    Apostrophe         = VK_OEM_7,
    LeftBracket        = VK_OEM_4,
    RightBracket       = VK_OEM_6,
    Backslash          = VK_OEM_5,
    Minus              = VK_OEM_MINUS,
    Equals             = VK_OEM_PLUS
};

struct KeyState
{
    bool IsDown = false;

    Delegate<> OnKeyDown;
    Delegate<> OnKeyUp;
};

enum class ECursorIcon : uint8
{
    Arrow,
    Crosshair,
    Hand,
    IBeam,
    SizeAll,
    SizeNESW,
    SizeNS,
    SizeNWSE,
    SizeWE,
    Wait,
    Help,
    NotAllowed
};

class InputSubsystem : public EngineSubsystem
{
public:
    Delegate<const Vector2> OnMouseMoved;
    Delegate<> OnMouseLeftButtonDown;
    Delegate<> OnMouseLeftButtonUp;
    Delegate<> OnMouseRightButtonDown;
    Delegate<> OnMouseRightButtonUp;
    Delegate<> OnMouseMiddleButtonDown;
    Delegate<> OnMouseMiddleButtonUp;
    Delegate<> OnMouseXButton1Down;
    Delegate<> OnMouseXButton1Up;
    Delegate<> OnMouseXButton2Down;
    Delegate<> OnMouseXButton2Up;
    Delegate<> OnMouseWheelScroll;

    Delegate<EKey> OnAnyKeyDown;
    Delegate<EKey> OnAnyKeyUp;

public:
    InputSubsystem() = default;

    static InputSubsystem& Get();

    bool IsKeyDown(EKey key) const;

    KeyState& GetKey(EKey key);

    bool IsCapsLockToggled() const;
    
    DirectX::Mouse& GetMouse() const;

    Vector2 GetMousePosition() const;
    const DirectX::Mouse::State& GetMouseState() const;

    void SetCursorIcon(ECursorIcon icon);
    ECursorIcon GetCursorIcon() const;

    LRESULT ProcessWindowMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, const std::shared_ptr<Window>& window, PassKey<Window>);

    void SetFocusedWindow(const std::shared_ptr<Window>& window, PassKey<Window>);
    std::shared_ptr<Window> GetFocusedWindow() const;

    static bool IsAlphaNumeric(EKey key);

public:
    virtual bool Initialize() override;
    virtual void Tick(double deltaTime) override;

private:
    std::unique_ptr<DirectX::Mouse> _mouse;
    DirectX::Mouse::State _mouseState;
    DirectX::Mouse::ButtonStateTracker _mouseButtonStateTracker;
    ECursorIcon _cursorIcon = ECursorIcon::Arrow;

    std::unordered_map<EKey, KeyState> _keyStates;

    std::weak_ptr<Window> _focusedWindow;

    std::vector<Delegate<>*> _pendingDelegates;
    
    std::vector<EKey> _pendingOnAnyKeyDown;
    std::vector<EKey> _pendingOnAnyKeyUp;

    bool _isCapsLockToggled = false;

private:
    EKey ConvertKeyCode(WPARAM virtualKeyCode) const;
    void ApplyCursorIcon();
};

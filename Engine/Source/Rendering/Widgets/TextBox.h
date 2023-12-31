#pragma once

#include "Widget.h"
#include "Rendering/Font.h"
#include "TextBox.reflection.h"

// REFLECTED() todo implement reflection for enums
enum class ETextFormatting : uint8
{
    Center,
    Left,
    Right
};

REFLECTED()
class TextBox : public Widget
{
    GENERATED()

public:
    TextBox() = default;

    TextBox(const TextBox& other);
    TextBox& operator=(const TextBox& other);

    void SetText(const std::wstring& text);
    const std::wstring& GetText() const;

    void SetFont(const std::shared_ptr<Font>& font);
    std::shared_ptr<Font> GetFont() const;

    void SetFontType(Font::EType fontType);
    Font::EType GetFontType() const;

    void SetFontSize(float fontSize);
    float GetFontSize() const;

    void SetTextColor(const Color& color);
    const Color& GetTextColor() const;

    void SetFormatting(ETextFormatting formatting);
    ETextFormatting GetFormatting() const;

    void SetBackgroundVisibility(bool value);
    bool IsBackgroundVisible() const;

protected:
    virtual void OnTextChanged();
    
    // Widget
public:
    virtual bool Initialize() override;

    // Widget
protected:
    virtual bool InitializeRenderingProxy() override;

    virtual void OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow) override;
    
    virtual void OnHoverStartedInternal() override;
    virtual void OnHoverEndedInternal() override;

private:
    PROPERTY(EditInEditor, DisplayName = "Text")
    std::wstring _text;

    PROPERTY(EditInEditor, Load, DisplayName = "Font")
    std::shared_ptr<Font> _font;

    PROPERTY(EditInEditor, DisplayName = "Font Type")
    Font::EType _fontType = Font::EType::Regular;

    PROPERTY(EditInEditor, DisplayName = "Font Size")
    float _fontSize = 1.0f;

    PROPERTY(EditInEditor, DisplayName = "Text Color")
    Color _textColor = {0.0f, 0.0f, 0.0f, 1.0f};

    PROPERTY(EditInEditor, DisplayName = "Formatting")
    ETextFormatting _formatting = ETextFormatting::Center;

    PROPERTY(EditInEditor, DisplayName = "Background")
    bool _isBackgroundVisible = false;
};

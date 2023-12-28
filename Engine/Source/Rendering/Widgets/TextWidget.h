#pragma once

#include "Widget.h"
#include "Rendering/Font.h"
#include "TextWidget.reflection.h"

// REFLECTED() todo implement reflection for enums
enum class ETextFormatting : uint8
{
    Center,
    Left,
    Right
};

REFLECTED()
class TextWidget : public Widget
{
    GENERATED()

public:
    TextWidget() = default;

    TextWidget(const TextWidget& other);
    TextWidget& operator=(const TextWidget& other);

    void SetText(const std::wstring& text);
    const std::wstring& GetText() const;

    void SetFont(const std::shared_ptr<Font>& font);
    std::shared_ptr<Font> GetFont() const;

    void SetFontType(Font::EType fontType);
    Font::EType GetFontType() const;

    void SetTextColor(const Color& color);
    const Color& GetTextColor() const;

    void SetFormatting(ETextFormatting formatting);
    ETextFormatting GetFormatting() const;

    void SetBackgroundVisibility(bool value);
    bool IsBackgroundVisible() const;

protected:
    virtual bool InitializeRenderingProxy() override;

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

private:
    void OnTextChanged();
};

#pragma once

#include "Asset.h"
#include "Importer.h"
#include "SpriteFont.h"
#include "Font.reflection.h"

REFLECTED()
class FontImporter : public Importer
{
    GENERATED()

public:
    PROPERTY(Edit)
    std::string FontName;
};

// todo this class is not RHI-agnostic
REFLECTED()
class Font : public Asset
{
    GENERATED()

public:
    enum class EType : uint8
    {
        Regular,
        Bold,
        Italic,
        Strikethrough,
        Underline
    };

public:
    explicit Font();
    explicit Font(Name name);

    Font(const Font& other);

    DirectX::SpriteFont* GetSpriteFont(EType fontType) const;

    Vector2 MeasureString(const std::wstring& text, EType fontType) const;

    // Asset
public:
    virtual bool Initialize() override;

    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

    std::vector<std::shared_ptr<Asset>> Import(const std::shared_ptr<Importer>& importer) const override;

private:
    std::filesystem::path _bitmapPathBase;

    std::unique_ptr<DirectX::SpriteFont> _fontRegular;
    std::unique_ptr<DirectX::SpriteFont> _fontBold;
    std::unique_ptr<DirectX::SpriteFont> _fontItalic;
    std::unique_ptr<DirectX::SpriteFont> _fontStrikethrough;
    std::unique_ptr<DirectX::SpriteFont> _fontUnderline;
};

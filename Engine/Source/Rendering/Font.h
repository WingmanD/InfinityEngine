#pragma once

#include "Asset.h"
#include "SpriteFont.h"
#include "NonCopyable.h"
#include "Font.reflection.h"

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
    explicit Font() = default;
    explicit Font(std::wstring name);

    Font(const Font& other);
    Font& operator=(const Font& other);

    // todo rework importing, Type needs to know how to import itself
    //static std::shared_ptr<Font> Import(AssetManager& assetManager, const std::filesystem::path& ttf2Path);
    static std::shared_ptr<Font> Import(AssetManager& assetManager, const std::filesystem::path& path, const std::string& name);

    DirectX::SpriteFont* GetSpriteFont(EType fontType) const;

    // Asset
public:
    virtual bool Initialize() override;

    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

private:
    std::filesystem::path _bitmapPathBase;

    std::unique_ptr<DirectX::SpriteFont> _fontRegular;
    std::unique_ptr<DirectX::SpriteFont> _fontBold;
    std::unique_ptr<DirectX::SpriteFont> _fontItalic;
    std::unique_ptr<DirectX::SpriteFont> _fontStrikethrough;
    std::unique_ptr<DirectX::SpriteFont> _fontUnderline;
};

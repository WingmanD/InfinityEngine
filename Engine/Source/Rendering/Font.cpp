#include "Font.h"

#include "ResourceUploadBatch.h"
#include "DX12/DX12RenderingSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/Subsystems/AssetManager.h"

Font::Font(std::wstring name) : Asset(std::move(name))
{
}

Font::Font(const Font& other)
{
    // todo deep copy
}

Font& Font::operator=(const Font& other)
{
    return *this;
}

std::shared_ptr<Font> Font::Import(AssetManager& assetManager, const std::filesystem::path& path, const std::string& name)
{
    const std::filesystem::path exePath = assetManager.GetProjectRootPath() / "ThirdParty/DirectXTK12Fonts/MakeSpriteFont";

    std::string legalName = name;
    std::ranges::replace(legalName, ' ', '_');

    const std::filesystem::path fontPath = assetManager.GetProjectRootPath() / "Engine/Content/Fonts" / legalName / "";

    create_directories(fontPath);

    std::string command = std::format("{} {} {}{}.spritefont /FontSize:32", exePath.string(), name, fontPath.string(), legalName);
    int result = system(command.c_str());
    if (result != 0)
    {
        LOG(L"Failed to import regular font {}!", Util::ToWString(name));
        return nullptr;
    }

    command = std::format("{} {} {}{}{}.spritefont /FontSize:32 /FontStyle:{}", exePath.string(), name, fontPath.string(), legalName, "Bold", "Bold");
    result = system(command.c_str());
    if (result != 0)
    {
        LOG(L"Failed to import bold font {}!", Util::ToWString(name));
        return nullptr;
    }

    command = std::format("{} {} {}{}{}.spritefont /FontSize:32 /FontStyle:{}", exePath.string(), name, fontPath.string(), legalName, "Italic", "Italic");
    result = system(command.c_str());
    if (result != 0)
    {
        LOG(L"Failed to import italic font {}!", Util::ToWString(name));
        return nullptr;
    }

    command = std::format("{} {} {}{}{}.spritefont /FontSize:32 /FontStyle:{}", exePath.string(), name, fontPath.string(), legalName, "Strikeout", "Strikeout");
    result = system(command.c_str());
    if (result != 0)
    {
        LOG(L"Failed to import strikeout font {}!", Util::ToWString(name));
        return nullptr;
    }

    command = std::format("{} {} {}{}{}.spritefont /FontSize:32 /FontStyle:{}", exePath.string(), name, fontPath.string(), legalName, "Underline", "Underline");
    result = system(command.c_str());
    if (result != 0)
    {
        LOG(L"Failed to import underline font {}!", Util::ToWString(name));
        return nullptr;
    }

    std::shared_ptr<Font> font = assetManager.NewAsset<Font>(Util::ToWString(name));
    font->_bitmapPathBase = fontPath / legalName;
    if (!font->Initialize())
    {
        assetManager.DeleteAsset(font);
        return nullptr;
    }

    font->SetName(Util::ToWString(name));

    return font;
}

DirectX::SpriteFont* Font::GetSpriteFont(EType fontType) const
{
    switch (fontType)
    {
        case EType::Regular:
        {
            return _fontRegular.get();
        }
        case EType::Bold:
        {
            return _fontBold.get();
        }
        case EType::Italic:
        {
            return _fontItalic.get();
        }
        case EType::Strikethrough:
        {
            return _fontStrikethrough.get();
        }
        case EType::Underline:
        {
            return _fontUnderline.get();
        }
        default:
        {
            LOG(L"Invalid font type!");
            return nullptr;
        }
    }
}

Vector2 Font::MeasureString(const std::wstring& text, EType fontType) const
{
    const DirectX::SpriteFont* spriteFont = GetSpriteFont(fontType);
    if (spriteFont == nullptr)
    {
        return Vector2::Zero;
    }

    return spriteFont->MeasureString(text.c_str(), false);
}

bool Font::Initialize()
{
    if (!Asset::Initialize())
    {
        return false;
    }

    std::filesystem::path fontPath = _bitmapPathBase;

    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    ID3D12Device* device = renderingSubsystem.GetDevice();

    const std::shared_ptr<DescriptorHeap> descriptorHeap = renderingSubsystem.GetCBVHeap();

    DirectX::ResourceUploadBatch uploadBatch(device);
    uploadBatch.Begin(D3D12_COMMAND_LIST_TYPE_COPY);

    fontPath += ".spritefont";
    auto heapHandle = descriptorHeap->RequestHeapResourceHandle();
    _fontRegular = std::make_unique<DirectX::SpriteFont>(device, uploadBatch, fontPath.wstring().c_str(), heapHandle, descriptorHeap->GetGPUHeapResourceHandle(heapHandle));
    if (_fontRegular == nullptr)
    {
        LOG(L"Failed to load regular font {}!", GetName());
        return false;
    }

    fontPath = _bitmapPathBase;
    fontPath += "Bold.spritefont";
    heapHandle = descriptorHeap->RequestHeapResourceHandle();
    _fontBold = std::make_unique<DirectX::SpriteFont>(device, uploadBatch, fontPath.wstring().c_str(), descriptorHeap->RequestHeapResourceHandle(), descriptorHeap->GetGPUHeapResourceHandle(heapHandle));
    if (_fontBold == nullptr)
    {
        LOG(L"Failed to load bold font {}!", GetName());
        return false;
    }

    fontPath = _bitmapPathBase;
    fontPath += "Italic.spritefont";
    heapHandle = descriptorHeap->RequestHeapResourceHandle();
    _fontItalic = std::make_unique<DirectX::SpriteFont>(device, uploadBatch, fontPath.wstring().c_str(), descriptorHeap->RequestHeapResourceHandle(), descriptorHeap->GetGPUHeapResourceHandle(heapHandle));
    if (_fontItalic == nullptr)
    {
        LOG(L"Failed to load italic font {}!", GetName());
        return false;
    }

    fontPath = _bitmapPathBase;
    fontPath += "Strikeout.spritefont";
    heapHandle = descriptorHeap->RequestHeapResourceHandle();
    _fontStrikethrough = std::make_unique<DirectX::SpriteFont>(device, uploadBatch, fontPath.wstring().c_str(), descriptorHeap->RequestHeapResourceHandle(), descriptorHeap->GetGPUHeapResourceHandle(heapHandle));
    if (_fontStrikethrough == nullptr)
    {
        LOG(L"Failed to load strikethrough font {}!", GetName());
        return false;
    }

    fontPath = _bitmapPathBase;
    fontPath += "Underline.spritefont";
    heapHandle = descriptorHeap->RequestHeapResourceHandle();
    _fontUnderline = std::make_unique<DirectX::SpriteFont>(device, uploadBatch, fontPath.wstring().c_str(), descriptorHeap->RequestHeapResourceHandle(), descriptorHeap->GetGPUHeapResourceHandle(heapHandle));
    if (_fontUnderline == nullptr)
    {
        LOG(L"Failed to load underline font {}!", GetName());
        return false;
    }

    // todo all this should be done asynchronously
    const std::future<void> uploadResourcesFinished = uploadBatch.End(
        renderingSubsystem.GetCopyCommandQueue());

    uploadResourcesFinished.wait();

    return true;
}

bool Font::Serialize(MemoryWriter& writer) const
{
    if (!Asset::Serialize(writer))
    {
        return false;
    }

    writer << _bitmapPathBase;

    return true;
}

bool Font::Deserialize(MemoryReader& reader)
{
    if (!Asset::Deserialize(reader))
    {
        return false;
    }

    reader >> _bitmapPathBase;
    return true;
}

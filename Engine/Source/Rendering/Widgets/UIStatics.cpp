#include "UIStatics.h"

#include "Engine/Engine.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Font.h"
#include "Rendering/Window.h"

#ifdef _WIN32
#include <Windows.h>
#include <shobjidl.h>
#endif

std::shared_ptr<StaticMesh> UIStatics::GetUIQuadMesh()
{
    static std::weak_ptr mesh = AssetManager::Get().FindAssetByName<StaticMesh>(Name(L"UIQuad"));

    return mesh.lock();
}

std::shared_ptr<Font> UIStatics::GetDefaultFont()
{
    static std::weak_ptr font = AssetManager::Get().FindAssetByName<Font>(Name(L"Arial"));

    return font.lock();
}

Vector2 UIStatics::ToScreenSpace(const Vector2& positionWS, const std::shared_ptr<Window>& parentWindow)
{
    const Vector2 normalizedPosition = positionWS * Vector2(0.5f / parentWindow->GetAspectRatio(), -0.5f) +
        Vector2(0.5f, 0.5f);

    return normalizedPosition * Vector2(static_cast<float>(parentWindow->GetWidth()),
                                        static_cast<float>(parentWindow->GetHeight()));
}

Vector2 UIStatics::ToWidgetSpace(const Vector2& positionSS, const std::shared_ptr<Window>& parentWindow)
{
    const Vector2 normalizedPosition = positionSS / Vector2(static_cast<float>(parentWindow->GetWidth()),
                                                            static_cast<float>(parentWindow->GetHeight())) - Vector2(
        0.5f, 0.5f);

    return normalizedPosition * Vector2(parentWindow->GetAspectRatio() * 2.0f, -2.0f);
}

void UIStatics::OpenFileDialog(const std::filesystem::path& initialPath,
                               const std::function<void(const std::filesystem::path&)>& onClosed)
{
    Engine::Get().GetThreadPool().EnqueueTask([initialPath, onClosed]()
    {
        std::filesystem::path path;

#ifdef _WIN32
        HRESULT hr = CoInitializeEx(nullptr,
                                    COINIT_APARTMENTTHREADED |
                                    COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr))
        {
            IFileOpenDialog* pFileOpen;

            hr = CoCreateInstance(CLSID_FileOpenDialog,
                                  nullptr,
                                  CLSCTX_ALL,
                                  IID_IFileOpenDialog,
                                  reinterpret_cast<void**>(&pFileOpen));

            if (SUCCEEDED(hr))
            {
                {
                    IShellItem* pItem;
                    const PCWSTR pszInitialFolder = initialPath.wstring().c_str();
                    hr = SHCreateItemFromParsingName(pszInitialFolder, nullptr, IID_PPV_ARGS(&pItem));

                    if (SUCCEEDED(hr))
                    {
                        pFileOpen->SetDefaultFolder(pItem);
                        pItem->Release();
                    }
                }

                hr = pFileOpen->Show(nullptr);

                if (SUCCEEDED(hr))
                {
                    IShellItem* pItem;
                    hr = pFileOpen->GetResult(&pItem);
                    if (SUCCEEDED(hr))
                    {
                        PWSTR pszFilePath;
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                        if (SUCCEEDED(hr))
                        {
                            const std::wstring ws(pszFilePath);
                            path = ws;

                            CoTaskMemFree(pszFilePath);
                        }

                        pItem->Release();
                    }
                }
                pFileOpen->Release();
            }
            CoUninitialize();
        }
#endif

        Engine::Get().GetMainEventQueue().Enqueue([onClosed, path](Engine*)
        {
            onClosed(path);
        });
    });
}

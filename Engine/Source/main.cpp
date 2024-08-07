#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Core.h"
#include "Engine/Engine.h"
#include "Reflection.generated.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    CoInitialize(nullptr);
    
    wchar_t executablePath[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, executablePath, MAX_PATH))
    {
        return -1;
    }

    const std::filesystem::path libPath = std::filesystem::path(executablePath).parent_path() / L"Libraries";
    SetDllDirectory(libPath.c_str());
    
#if DEBUG
    std::filesystem::path logPath = std::filesystem::path(executablePath).parent_path().parent_path().parent_path().parent_path() / "Engine" / "Logs" / "Log.txt";
#elif
    std::filesystem::path logPath = "C:/Projects/InfinityEngine/Engine/Logs/LogShipping.txt";
#endif

    Logger& logger = Logger::GetInstance();
    logger.SetFile(logPath);
    if (!logger.Initialize())
    {
        return -1;
    }
    
    InitializeReflection();

    Engine& engine = Engine::Get();
    if (!engine.Initialize(hInstance))
    {
        LOG(L"Failed to initialize engine!");
        return 1;
    }
    engine.Run();

    CoUninitialize();

    return 0;
}

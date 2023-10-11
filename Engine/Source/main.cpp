#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Core.h"
#include "Reflection.generated.h"
#include "Engine/Engine.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    InitializeReflection();
    TypeRegistry::Get().PrintRegisteredTypes();
    
    Engine& engine = Engine::Get();
    if (!engine.Initialize(hInstance))
    {
        LOG(L"Failed to initialize engine!");
        return 1;
    }
    engine.Run();
    
    return 0;
}

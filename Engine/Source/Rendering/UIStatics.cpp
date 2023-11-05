#include "UIStatics.h"
#include "Engine/Subsystems/AssetManager.h"
#include "StaticMesh.h"

std::shared_ptr<StaticMesh> UIStatics::GetUIQuadMesh()
{
    static std::weak_ptr mesh = AssetManager::Get().FindAssetByName<StaticMesh>(L"UIQuad");

    return mesh.lock();
}

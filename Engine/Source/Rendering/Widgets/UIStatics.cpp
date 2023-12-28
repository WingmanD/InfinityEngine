#include "UIStatics.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Window.h"

std::shared_ptr<StaticMesh> UIStatics::GetUIQuadMesh()
{
    static std::weak_ptr mesh = AssetManager::Get().FindAssetByName<StaticMesh>(L"UIQuad");

    return mesh.lock();
}

Vector2 UIStatics::ToScreenSpace(const Vector2& positionWS, const std::shared_ptr<Window>& parentWindow)
{
    const Vector2 normalizedPosition = positionWS * Vector2(0.5f / parentWindow->GetAspectRatio() , -0.5f) + Vector2(0.5f, 0.5f);

    return normalizedPosition * Vector2(static_cast<float>(parentWindow->GetWidth()), static_cast<float>(parentWindow->GetHeight()));
}

Vector2 UIStatics::ToWidgetSpace(const Vector2& positionSS, const std::shared_ptr<Window>& parentWindow)
{
    const Vector2 normalizedPosition = positionSS / Vector2(static_cast<float>(parentWindow->GetWidth()), static_cast<float>(parentWindow->GetHeight())) - Vector2(0.5f, 0.5f);

    return normalizedPosition * Vector2(parentWindow->GetAspectRatio() * 2.0f, -2.0f); 
}

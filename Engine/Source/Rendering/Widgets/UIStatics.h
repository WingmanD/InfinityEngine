#pragma once

#include "Core.h"
#include <memory>

class StaticMesh;
class Window;

namespace UIStatics
{
std::shared_ptr<StaticMesh> GetUIQuadMesh();

Vector2 ToScreenSpace(const Vector2& positionWS, const std::shared_ptr<Window>& parentWindow);
Vector2 ToWidgetSpace(const Vector2& positionSS, const std::shared_ptr<Window>& parentWindow);
};

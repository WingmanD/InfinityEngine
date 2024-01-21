#pragma once

#include "Core.h"
#include <memory>

class StaticMesh;
class Font;
class Window;

namespace UIStatics
{
std::shared_ptr<StaticMesh> GetUIQuadMesh();
std::shared_ptr<Font> GetDefaultFont();

Vector2 ToScreenSpace(const Vector2& positionWS, const std::shared_ptr<Window>& parentWindow);
Vector2 ToWidgetSpace(const Vector2& positionSS, const std::shared_ptr<Window>& parentWindow);

void OpenFileDialog(const std::filesystem::path& initialPath, const std::function<void(const std::filesystem::path&)>& onClosed);
}

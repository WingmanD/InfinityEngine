#pragma once

#include "CanvasPanel.h"
#include "ViewportWidget.reflection.h"

class CCamera;

REFLECTED()
class ViewportWidget : public CanvasPanel
{
    GENERATED()

public:
    void SetCamera(CCamera* camera);
    CCamera* GetCamera() const;

protected:
    CCamera* _camera = nullptr;

    // Widget
protected:
    virtual bool InitializeRenderingProxy() override;
    void OnWidgetRectChanged() override;
};

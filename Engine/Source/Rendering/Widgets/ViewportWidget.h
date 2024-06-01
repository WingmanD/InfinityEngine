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

    void CaptureMouse();
    void ReleaseMouse();
    
    // Widget
protected:
    virtual bool InitializeRenderingProxy() override;
    void OnWidgetRectChanged() override;
    
    void OnFocusChangedInternal(bool focused) override;

private:
    CCamera* _camera = nullptr;
    bool _isMouseCaptured = false;
};

#pragma once

#include "CanvasPanel.h"
#include "ViewportWidget.reflection.h"

class CCamera;

REFLECTED()
class ViewportWidget : public CanvasPanel
{
    GENERATED()

public:
    ViewportWidget();
    
    void SetCamera(CCamera* camera);
    CCamera* GetCamera() const;

    void CaptureMouse();
    void ReleaseMouse();

    // Widget
public:
    virtual bool Initialize() override;
    
    // Widget
protected:
    virtual bool InitializeRenderingProxy() override;
    virtual void OnWidgetRectChanged() override;
    
    virtual void OnFocusChangedInternal(bool focused) override;

private:
    CCamera* _camera = nullptr;
    bool _isMouseCaptured = false;
};

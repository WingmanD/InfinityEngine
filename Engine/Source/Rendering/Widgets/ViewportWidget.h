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

    Vector3 GetMouseDirectionWS() const;

    void SetCaptureMouseOnClick(bool value);
    bool ShouldCaptureMouseOnClick() const;

    // Widget
public:
    virtual bool Initialize() override;
    
    // Widget
protected:
    virtual bool InitializeRenderingProxy() override;
    virtual void OnWidgetRectChanged() override;

    virtual bool OnReleasedInternal() override;
    virtual void OnFocusChangedInternal(bool focused) override;

private:
    CCamera* _camera = nullptr;
    bool _isMouseCaptured:1 = false;
    bool _shouldCaptureMouseOnClick:1 = true;
};

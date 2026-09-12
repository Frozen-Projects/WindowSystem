#pragma once

#include "CoreMinimal.h"
#include "RHIResources.h"
#include <atomic>

class SWindow;
class ISlateViewportProvider;
class FTextureRenderTargetResource;

class FWindowSystemViewportCapture : public TSharedFromThis<FWindowSystemViewportCapture, ESPMode::ThreadSafe>
{
public:
    void Start();
    void Stop();
    void Update(const SWindow* Window, FIntRect Rect, FTextureRenderTargetResource* Resource);
    bool IsReady() const;
    FString GetError() const;

private:
    void OnBackBufferReady(SWindow& Window, ISlateViewportProvider& Provider);

    FDelegateHandle BackBufferHandle;
    const SWindow* TargetWindow = nullptr;
    FIntRect CaptureRect = FIntRect(0, 0, 0, 0);
    FTextureRHIRef TargetTexture;
    std::atomic<bool> bReady = false;
    std::atomic<bool> bUnsupportedFormat = false;
};

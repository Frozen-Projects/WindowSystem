#include "Viewport/ViewportCapture.h"

#include "Framework/Application/SlateApplication.h"
#include "HDRHelper.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Rendering/SlateRenderer.h"
#include "RenderingThread.h"
#include "RHIStaticStates.h"
#include "ScreenPass.h"
#include "Slate/SlateViewportProvider.h"
#include "TextureResource.h"

void FWindowSystemViewportCapture::Start()
{
    check(IsInGameThread());
    BackBufferHandle = FSlateApplication::Get().GetRenderer()->OnBackBufferReadyToPresent().AddSP(AsShared(), &FWindowSystemViewportCapture::OnBackBufferReady);
}

void FWindowSystemViewportCapture::Stop()
{
    check(IsInGameThread());
    if (FSlateApplication::IsInitialized() && FSlateApplication::Get().GetRenderer())
    {
        FSlateApplication::Get().GetRenderer()->OnBackBufferReadyToPresent().Remove(BackBufferHandle);
    }
    BackBufferHandle.Reset();
    Update(nullptr, FIntRect(0, 0, 0, 0), nullptr);
}

void FWindowSystemViewportCapture::Update(const SWindow* Window, FIntRect Rect, FTextureRenderTargetResource* Resource)
{
    check(IsInGameThread());
    ENQUEUE_RENDER_COMMAND(WindowSystemCaptureState)([Self = AsShared(), Window, Rect, Resource](FRHICommandListImmediate& RHICmdList)
    {
        Self->TargetWindow = Window;
        Self->CaptureRect = Rect;
        Self->TargetTexture = Resource ? Resource->GetRenderTargetTexture() : nullptr;
        if (!Window || Rect.IsEmpty())
        {
            Self->bReady.store(false);
        }
    });
}

bool FWindowSystemViewportCapture::IsReady() const
{
    return bReady.load() && !bUnsupportedFormat.load();
}

FString FWindowSystemViewportCapture::GetError() const
{
    return bUnsupportedFormat.load() ? TEXT("Capture requires an SDR BGRA8, RGBA8, or RGB10A2 backbuffer. HDR tone mapping is not configured.") : FString();
}

void FWindowSystemViewportCapture::OnBackBufferReady(SWindow& Window, ISlateViewportProvider& Provider)
{
    check(IsInRenderingThread());
    if (&Window != TargetWindow || CaptureRect.IsEmpty() || !TargetTexture.IsValid())
    {
        return;
    }
    FRHITexture* BackBuffer = Provider.GetBackBufferResource();
    if (!BackBuffer || BackBuffer->GetDesc().NumSamples != 1)
    {
        return;
    }
    const EPixelFormat Format = BackBuffer->GetFormat();
    if (IsHDREnabled() || (Format != PF_B8G8R8A8 && Format != PF_R8G8B8A8 && Format != PF_A2B10G10R10))
    {
        bUnsupportedFormat.store(true);
        return;
    }
    bUnsupportedFormat.store(false);
    FIntRect SourceRect = CaptureRect;
    SourceRect.Clip(FIntRect(FIntPoint::ZeroValue, BackBuffer->GetDesc().Extent));
    if (SourceRect.IsEmpty())
    {
        return;
    }

    const FIntPoint OutputSize = TargetTexture->GetDesc().Extent;
    const double Scale = FMath::Min(static_cast<double>(OutputSize.X) / SourceRect.Width(), static_cast<double>(OutputSize.Y) / SourceRect.Height());
    const FIntPoint DrawSize(FMath::Clamp(FMath::RoundToInt(SourceRect.Width() * Scale), 1, OutputSize.X),
        FMath::Clamp(FMath::RoundToInt(SourceRect.Height() * Scale), 1, OutputSize.Y));
    const FIntPoint DrawPosition = (OutputSize - DrawSize) / 2;

    FRDGBuilder GraphBuilder(FRHICommandListImmediate::Get());
    FRDGTextureRef Input = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(BackBuffer, TEXT("WindowSystem Backbuffer")));
    FRDGTextureRef Output = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(TargetTexture, TEXT("WindowSystem Capture")));
    AddClearRenderTargetPass(GraphBuilder, Output, FLinearColor::Black);
    AddDrawTexturePass(GraphBuilder, FScreenPassViewInfo(), Input, Output, SourceRect.Min, SourceRect.Size(), DrawPosition, DrawSize, TStaticSamplerState<SF_Bilinear>::GetRHI());
    GraphBuilder.SetTextureAccessFinal(Input, ERHIAccess::Present);
    GraphBuilder.SetTextureAccessFinal(Output, ERHIAccess::SRVMask);
    GraphBuilder.Execute();
    bReady.store(true);
}

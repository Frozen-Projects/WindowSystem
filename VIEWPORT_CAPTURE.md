# Viewport capture update

Viewport capture is implemented entirely in WindowSystem. Its Blueprint API returns a normal `UTextureRenderTarget2D`; FF_Encoder consumes that texture through its existing API and has no dependency on this helper. FF_Encoder source was not modified for this update.

## Why the previous copy fails

- `ViewportRenderTarget` was initialized to null and never allocated anywhere in WindowSystem's source. The getter therefore returned null unless something outside that source assigned the member.
- `UGameViewportClient::Draw` precedes final Slate composition. Copying there cannot reliably include UMG/Slate widgets, and the custom background canvas may not have been flushed yet.
- A raw `CopyTexture` requires compatible formats and copy dimensions. It performs neither resizing nor BGRA8 conversion. The encoder requires BGRA8 with dimensions equal to its configured resolution.
- The queued lambda captured the UObject and read `GEngine`, the viewport, and the render-target UObject from the render thread. Their lifetime and ownership were not protected.
- `bCopyViewportToRenderTarget` defaulted to false, and the old getter did not enable it.

Calling `ViewportRenderTarget->InitAutoFormat` inside the render command also performs initialization on the wrong thread. The helper creates and initializes the target on the game thread once, using `InitCustomFormat(..., PF_B8G8R8A8, true)`; it does not reinitialize the target for each captured frame.

## Implementation

The capture helper binds to `FSlateRenderer::OnBackBufferReadyToPresent`. The installed UE 5.8.2 signature is:

```cpp
void OnBackBufferReady(SWindow& Window, ISlateViewportProvider& Provider);
```

The source texture comes from `Provider.GetBackBufferResource()` during that callback. The callback already runs on the render thread after Slate composition; it does not enqueue another delayed backbuffer lookup.

On the game thread, the owning `SViewport` is found inside its Slate window to obtain the game viewport's rectangle. Only that window is captured. The same crop logic supports a viewport embedded inside a larger Slate window. Actual multi-instance PIE sessions and packaged game windows have not been tested.

A render graph crops the composed backbuffer, converts supported SDR formats to BGRA8, and scales into a stable output target. Different aspect ratios are letterboxed. Window resizing changes the source rectangle without resizing the encoder's input target. The render thread accesses only the capture helper and RHI resources. Delegate removal and render-command draining occur when capture stops or the viewport client is destroyed.

The existing background canvas is now created after a world is available instead of in the viewport client's constructor. Its drawing and layout logic otherwise remain in place.

The module uses C++20 and treats deprecation warnings as errors. Broad UMG umbrella includes were replaced with specific public headers so the module builds with that setting in UE 5.8.2.

## Validation

Built successfully against installed UE 5.8.2, CL 56702186. The `WindowSystem.ViewportCapture` automation test passed on both D3D12 and D3D11 using the NVIDIA RTX 5090 Laptop GPU.

The test uses an offscreen Slate window with a game viewport, custom canvas drawing, a real UMG image widget, and a colored border outside the viewport. It verifies:

- BGRA8 output, readiness, repeated getter calls, and rejection of invalid dimensions.
- Custom canvas and changing UMG widget pixels in the composed texture, with the outer window border excluded.
- Aspect-ratio preservation, fixed output size during window resizing, stop/restart behavior, and viewport-detachment cleanup.
- Four frames accepted by the existing FF_Encoder and saved as H.264 MP4. Both recordings were independently decoded, and all four widget colors plus the custom canvas tile were checked in the decoded frames.

Reports and evidence are in the validation workspace:

- `C:\Users\erayo\Documents\ChatGPT\UE5\EncoderValidation\ViewportReports-D3D12\index.json`
- `C:\Users\erayo\Documents\ChatGPT\UE5\EncoderValidation\ViewportReports-D3D11\index.json`
- `C:\Users\erayo\Documents\ChatGPT\UE5\EncoderValidation\ViewportEvidence-D3D12`
- `C:\Users\erayo\Documents\ChatGPT\UE5\EncoderValidation\ViewportEvidence-D3D11`

These are controlled capture and encoder integration tests, not a test of the project's actual level or custom material assets. AMD hardware, HDR capture, and packaged builds were not tested. FFmpeg was used only for independent validation and is not a WindowSystem dependency.

## Blueprint setup after building this update

1. Keep `UCustomViewport` assigned as the Game Viewport Client class in Project Settings. Restart the editor after compiling reflected API changes.
2. Disable the encoder manager's `bAutoStart` while configuring capture.
3. Call WindowSystem's `StartViewportCapture` with the same resolution as `EncoderSettings.Resolution`, for example `(2560, 1600)`. Assign the returned texture to the encoder manager's `RenderTarget`.
4. Wait across game ticks until `IsViewportCaptureReady` returns true. Do not use a blocking loop on the game thread: Slate needs to render a frame first. If the target is null, the active viewport client or capture initialization is invalid; if readiness never arrives, inspect the client's `ViewportCaptureError` and the log.
5. Call `InitEncoder`. Keep `bEncodeEveryFrame` enabled for automatic capture. The actor encodes the latest completed viewport image; it can be one presentation behind and is not synchronized to one output packet per Slate presentation.
6. To finish: call the manager's `ShutdownEncoder`, then `SaveRecordings`, then WindowSystem's `StopViewportCapture`.

The existing `GetViewportRenderTarget` node also works as an idempotent start-and-get operation. It uses `ViewportCaptureResolution`, defaulting to `(2560, 1600)`. Calling it again after stopping restarts capture.

Stop the encoder before changing capture resolution or replacing its render target. A viewport/window resize alone does not require encoder reinitialization because output dimensions remain fixed.

This is capture of the composed game viewport inside one Slate window. It includes widgets and custom canvas drawing inside that rectangle. It is not desktop capture: native title bars, hardware cursor overlays, and additional OS windows are outside the captured texture. The implementation currently accepts SDR BGRA8, RGBA8, and RGB10A2 backbuffers. HDR/linear floating-point output requires a separate color conversion path and is rejected.

The Blueprint library retains its existing `GEngine->GameViewport` selection behavior, with null checks added. Selecting a particular world in a multi-instance PIE session is outside this update.

## Files to review

- `Source/WindowSystem/Public/Viewport/CustomViewport.h`
- `Source/WindowSystem/Private/Viewport/CustomViewport.cpp`
- `Source/WindowSystem/Private/Viewport/ViewportCapture.h` (new)
- `Source/WindowSystem/Private/Viewport/ViewportCapture.cpp` (new)
- `Source/WindowSystem/Public/BPLib/WS_BPLib.h`
- `Source/WindowSystem/Private/WS_BPLib.cpp`
- `Source/WindowSystem/WindowSystem.Build.cs` (Renderer dependency, C++20, deprecation warnings as errors)
- `Source/WindowSystem/Public/Viewport/CustomViewport_Includes.h`
- `Source/WindowSystem/Public/Window/Window_Includes.h`
- `Source/WindowSystem/Private/Window/Window_Instance.cpp` (explicit GameUserSettings include)

The workspace copy contains source and the plugin descriptor. It does not include WindowSystem's existing Content assets. Do not replace the complete original plugin directory with this source-only staging directory.

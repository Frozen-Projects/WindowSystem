#pragma once

#include "CoreMinimal.h"
#include "Window/Window_Includes.h"

// File Drag Drop Message Handler Subclass.
class FDragDropHandler : public IWindowsMessageHandler
{

private:

	static int32 GetWindowsBuildNumber();

public:

	virtual bool ProcessMessage(HWND Hwnd, uint32 Message, WPARAM WParam, LPARAM LParam, int32& OutResult) override;

};
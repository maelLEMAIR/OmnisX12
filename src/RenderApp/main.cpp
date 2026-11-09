#include <windows.h>
#include "main.h"

#include "D3D12/Renderer.h"
#include "Application.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int cmdShow)
{
    try
    {
        Application app(GetModuleHandle(nullptr));
        return app.Run();
    }
    catch (const DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"DirectX error", MB_OK | MB_ICONERROR);
        return 1;
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}
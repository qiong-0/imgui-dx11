// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include <Dwmapi.h> 
#pragma comment(lib, "dwmapi.lib")

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

bool menu = true, 透视 = true, 血量 = true, 信息 = true, 名字, 观战 = true, 物品, 范围;
bool 自瞄, 隔墙, 预判, 兔子跳, TS, SG, 热能, 内存, 人称;
int 距离 = 75, 模式 = 0, 热键, 速度 = 10, 范围2 = 30, 开枪;
ImVec4 透视1 = ImVec4(1.0f, 0.0f, 0.0f, 1.00f);
ImVec4 透视2 = ImVec4(0.0f, 1.0f, 0.0f, 1.00f);
ImVec4 信息1 = ImVec4(1.0f, 1.0f, 0.0f, 1.00f);
ImVec4 信息2 = ImVec4(0.43f, 0.8f, 1.0f, 1.00f);
ImVec4 名字1 = ImVec4(0.43f, 0.8f, 0.5f, 1.00f);
ImVec4 观战1 = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
ImVec4 范围1 = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
ImVec4 热能1 = ImVec4(1.0f, 0.0f, 0.0f, 1.00f);
ImVec4 热能2 = ImVec4(0.0f, 1.0f, 0.0f, 1.00f);

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void Rect(int x, int y, int w, int h, ImColor color)
{
    //GetForegroundDrawList
    //GetBackgroundDrawList
    //GetWindowDrawList
    ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color);
}

void Line(ImVec2 a, ImVec2 b, ImColor color)
{
    ImGui::GetBackgroundDrawList()->AddLine(a, b, color);
}

void Text(int x, int y, ImColor color, const char* text)
{
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(x, y), color, text);
}

void Circle(float x, float y, float radius, ImColor color)
{
    ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(x, y), radius, color);
}

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc;
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(RGB(0, 0, 0));
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"ImGui";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT, wc.lpszClassName, L"", WS_POPUP | WS_VISIBLE, 0, 0, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
    
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simhei.ttf", 14.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    io.IniFilename = NULL;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    ImVec4 color = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (GetAsyncKeyState(VK_INSERT) != 0)
        {
            menu = !menu;
            Sleep(125);
        }

        if (menu)
        {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
        }
        else
        {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin(u8"Hello, world! 你好, 世界!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit4("clear color", (float*)&color); // Edit 4 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        if (menu)
        {
            ImGui::Begin("##menu", (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
            if (ImGui::BeginTabBar("##menu2"))
            {
                if (ImGui::BeginTabItem(u8"透视类"))
                {
                    ImGui::Checkbox(u8"方框透视", &透视);
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##1", (float*)&透视1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##2", (float*)&透视2, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    ImGui::Checkbox(u8"显示血量", &血量);
                    ImGui::Checkbox(u8"显示信息", &信息);
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##3", (float*)&信息1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##4", (float*)&信息2, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    ImGui::Checkbox(u8"显示名字", &名字);
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##5", (float*)&名字1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    ImGui::Checkbox(u8"显示观战", &观战);
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##6", (float*)&观战1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    ImGui::Checkbox(u8"显示物品", &物品);
                    ImGui::Checkbox(u8"显示范围", &范围);
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##7", (float*)&范围1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    const char* 模式1[] = { u8"普通", u8"个人", u8"特殊" };
                    ImGui::Combo(u8"游戏模式", &模式, 模式1, IM_ARRAYSIZE(模式1));
                    ImGui::DragInt(u8"透视距离", &距离, 5, 75, 3000, "%d", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(u8"自瞄类"))
                {
                    ImGui::Checkbox(u8"自动瞄准", &自瞄);
                    const char* 热键1[] = { u8"左键", u8"右键", u8"左右键", u8"侧键4", u8"侧键5" };
                    ImGui::Combo(u8"自瞄热键", &热键, 热键1, IM_ARRAYSIZE(热键1));
                    ImGui::Checkbox(u8"隔墙不锁", &隔墙);
                    ImGui::Checkbox(u8"自动预判", &预判);
                    ImGui::SliderInt(u8"自瞄速度", &速度, 1, 10);
                    ImGui::SliderInt(u8"自瞄范围", &范围2, 1, 180);
                    const char* 开枪1[] = { u8"关闭", u8"右键", u8"侧键4", u8"侧键5" };
                    ImGui::Combo(u8"自瞄开枪", &开枪, 开枪1, IM_ARRAYSIZE(开枪1));
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(u8"功能类"))
                {
                    ImGui::Checkbox(u8"兔子跳", &兔子跳);
                    ImGui::Checkbox(u8"一键TS", &TS);
                    ImGui::Checkbox(u8"一键SG", &SG);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(u8"内存类"))
                {
                    ImGui::Checkbox(u8"全局热能", &热能);
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##8", (float*)&热能1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##9", (float*)&热能2, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    ImGui::Checkbox(u8"内存自瞄", &内存);
                    ImGui::Checkbox(u8"第三人称", &人称);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::Text(u8"绘制 FPS: %.3f 毫秒/帧 (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        Circle(50, 50, 100, 名字1);

        Rect(10, 10, 35, 50, 透视2);

        Line(ImVec2(1, 1), ImVec2(100, 100), 热能1);

        Text(150, 150, 范围1, u8"中文sft667");

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

//Headers
#include <Windows.h>
#include <stdio.h>
#include <iostream>

#include "Common.h"
#include "VkApplication.h"

#define USE_CONSOLE_BASED_APPLICATION 1

//Global function declaration
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM);

//Global variables
FILE *gpLogFile = NULL;
HWND  ghwnd;
HDC   ghdc;

DWORD style;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

int windowWidth  = 800;
int windowHeight = 600;

bool gbActiveWindow = false;
bool gbIsEscapeKeyPressed = false;
bool gbFullscreen = false;
bool gbDone = false;

//WinMain()
#if USE_CONSOLE_BASED_APPLICATION
int main()
#else
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInsatnce, LPSTR szCmdLine, int iCmdShow)
#endif
{

#if USE_CONSOLE_BASED_APPLICATION
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    int iCmdShow = SW_NORMAL;
#endif

    //function declarations
    void Initialize( void);
    void Display( void);
    void Update( double deltaTime);
    void UnInitialize( void);
    bool PollWindowEvent();
    
    //variable declarations
    WNDCLASSEX wndclass;
    HWND  hwnd;
    TCHAR szClassName[] = TEXT("Vulkan Triangle");

    //code
    if(fopen_s( &gpLogFile, "LogFile.txt", "w") != 0)
    {
        MessageBox( NULL, TEXT("Error while creating log file"), TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
        return( EXIT_FAILURE);
    }
    else
    {
        LogInfo("Log File Opened...");
    }
    
    //Initialize window attributes
    wndclass.cbSize         = sizeof(WNDCLASSEX);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW /* | CS_OWNDC */;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon( hInstance, IDI_APPLICATION);
    wndclass.hIconSm        = LoadIcon( hInstance, IDI_APPLICATION);
    wndclass.hCursor        = LoadCursor( NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH) GetStockObject( LTGRAY_BRUSH);
    wndclass.lpszClassName  = szClassName;
    wndclass.lpszMenuName   = NULL;
    wndclass.lpfnWndProc    = WndProc;
    
    if(!RegisterClassEx( &wndclass))
    {
        LogError( "Class Not Registered\n");
        return( EXIT_FAILURE);
    }
    
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    hwnd = CreateWindowEx(
                WS_EX_APPWINDOW,
                szClassName,
#if USE_CONSOLE_BASED_APPLICATION
                TEXT("Vulkan Console"),
#else
                TEXT("Vulkan Win32"),
#endif
                WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                (screenWidth - windowWidth) / 2, (screenHeight - windowHeight) / 2, windowWidth, windowHeight,
                NULL, NULL,
                hInstance, NULL
            );
    ghwnd = hwnd;

    ShowWindow( hwnd, iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus( hwnd);
    
    Initialize();

    //Game Loop
    while( gbDone == false)
    {
        if(!PollWindowEvent())
        {
            Display();
            Update(0.0f);
        }
    }

    UnInitialize();

    UnregisterClass( szClassName, hInstance);

    return(EXIT_SUCCESS);
}

// PollWindowEvent()
bool PollWindowEvent()
{
    // code
    MSG   msg;
    bool bIsMessageOccur = false;

    bIsMessageOccur = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE);
    if(bIsMessageOccur)
    {
        if(msg.message == WM_QUIT)
        {
            gbDone = true;
        }
        else
        {
            TranslateMessage( &msg);
            DispatchMessage( &msg);
        }
    }

    return bIsMessageOccur;
}

//Error Log
void PrintLog( int text_color, int background_color, int lineNo, char *fileName, char *functionName, char *format, ...)
{
    if( gpLogFile)
    {
        va_list argList;

        va_start( argList, format);

            fprintf( gpLogFile, "[%s\\%s() : %d]: ", fileName, functionName, lineNo);
            vfprintf( gpLogFile, format, argList);
            fprintf( gpLogFile, "\n");
            fflush( gpLogFile);

#if USE_CONSOLE_BASED_APPLICATION
    printf( "[%s\\%s() : %d]: \033[%d;%dm", fileName, functionName, lineNo, background_color, text_color);
    vprintf( format, argList);
    printf("\033[m");
    printf( "\n");
#endif

        va_end( argList);
    }
}

//WndProc()
LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //function declaration
    void Resize( int, int);
    void ToggleFullScreen( void);
    void UnInitialize( void);

    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    //variables
	static int mousePosX, mousePosY;
	static int iAccumDelta, zDelta;

	int mouseX, mouseY;
	int mouseDx, mouseDy;

	int newX = -1;
	int newY = -1;

	POINT pt;
    
    //code
    if(ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
    {
        return true;
    }

    if(ImGui::GetCurrentContext() != nullptr)
    {
        ImGuiIO& io = ImGui::GetIO();
        // if(io.WantCaptureKeyboard || io.WantCaptureMouse)
        if(io.WantCaptureMouse)
        {
            return true;
        }
    }

    switch(message)
    {
        case WM_SETFOCUS:
            gbActiveWindow = true;
        break;

        case WM_KILLFOCUS:
            gbActiveWindow = false;
        break;
        
        // case WM_ERASEBKGND:
        // return(0);
        
        case WM_SIZE:
            windowWidth = LOWORD(lParam);
            windowHeight = HIWORD(lParam);
            
            Resize( windowWidth, windowHeight);
        break;
        
        case WM_KEYDOWN:
            switch(wParam)
            {
                case VK_ESCAPE:
                    gbIsEscapeKeyPressed = true;
                break;
                
                case 0x46:  //'F' or 'f'
                    ToggleFullScreen();
                    gbFullscreen = !gbFullscreen;
                break;
            }
        break;
        
        case WM_LBUTTONDOWN:
            SetCapture( hwnd);
        break;

        case WM_LBUTTONUP:
            ReleaseCapture();
        break;
        
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
    }
    
    return( DefWindowProc(hwnd, message, wParam, lParam));
}

//ToggleFullScreen()
void ToggleFullScreen( void)
{
    //variable declarations
    MONITORINFO mi = { sizeof(MONITORINFO) };
    
    //code
    if(gbFullscreen == false)
    {
        style = GetWindowLong(ghwnd, GWL_STYLE);
        if(style & WS_OVERLAPPEDWINDOW)
        {
            if(
                GetWindowPlacement(ghwnd, &wpPrev) &&
                GetMonitorInfo(
                    MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY),
                    &mi
                )
            )
            {
                SetWindowLong(ghwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(
                    ghwnd,
                    HWND_TOP,
                    mi.rcMonitor.left,
                    mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_NOZORDER | SWP_FRAMECHANGED
                );
            }
        }
        
        //ShowCursor( FALSE);
    }
    else
    {
        SetWindowLong( ghwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement( ghwnd, &wpPrev);
        SetWindowPos(
            ghwnd,
            HWND_TOP,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED
        );
        
        //ShowCursor( TRUE);
    }
}

//Initialize()
void Initialize(void)
{
    //function declarations
    void Resize( int, int);
    void UnInitialize( void);

    //code
    RECT rc;
    GetClientRect( ghwnd, &rc);
    windowWidth = rc.right - rc.left;
    windowHeight = rc.bottom - rc.top;
    
    // Initialize Vulkan
    if(!VkApplication::Initialize(ghwnd))
    {
        LogError("VkApplication::Initialize() Failed.");
        DestroyWindow(ghwnd);
        return;
    }

    //warm-up Resize call
    Resize( windowWidth, windowHeight);
}

//Resize()
void Resize( int width, int height)
{
    //code
    VkApplication::ResizeCallback(width, height);
}


//Display()
void Display( void)
{
    // code
    VkApplication::DrawFrame();
}


//Update()
void Update( double deltaTime)
{
}


//UnInitialize()
void UnInitialize( void)
{
    //code
    if(gbFullscreen == true)
    {
        SetWindowLong( ghwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement( ghwnd, &wpPrev);
        SetWindowPos(
            ghwnd,
            HWND_TOP,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED
        );
        
        ShowCursor(TRUE);
    }

    //
    VkApplication::Uninitialize();

    if(ghdc)
    {
        ReleaseDC( ghwnd, ghdc);
        ghdc = NULL;
    }
    
    if(gpLogFile)
    {
        LogInfo("Log File Closed...");
        fclose(gpLogFile);
        gpLogFile = NULL;
    }
}


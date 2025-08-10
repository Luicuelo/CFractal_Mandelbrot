#include "dib.h"
#include "fractal_calc.h"
#include "wres.h"
#include "stsbar.h"
#include "main.h"
#include "threadpool.h"

#ifndef __cplusplus
    #include <stdbool.h>
#endif

struct threadpool_t *thread_pool;

HWND main_window_handle;
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
HINSTANCE instancia;

char szClassName[] = "WindowsApp";

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)
{
    RECT r = {0};
    HWND hwnd = NULL;
    MSG messages = {0};
    WNDCLASSEX wincl = {0};

    instancia = hThisInstance;
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_HREDRAW|CS_VREDRAW |CS_DBLCLKS;
    wincl.cbSize = sizeof (WNDCLASSEX);

    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = MAKEINTRESOURCE(ImenuP);
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);

    if (!RegisterClassEx (&wincl))
        return 0;

    r.left = 0;
    r.top = 0;
    r.right = WINDOW_WIDTH + 1;
    r.bottom = WINDOW_HEIGHT + 1;
    if (!AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, TRUE)) {
        MessageBox(NULL, "Error adjusting window", "Error", MB_OK);
        return -1;
    }

    hwnd = CreateWindowEx (
               0,
               szClassName,
               "Fractal",
               WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
               CW_USEDEFAULT,
               CW_USEDEFAULT,
               (r.right-r.left),
               (r.bottom-r.top),
               HWND_DESKTOP,
               NULL,
               hThisInstance,
               NULL
           );

    if (hwnd == NULL) {
        MessageBox(NULL, "Error creating window", "Error", MB_OK);
        return -1;
    }

    main_window_handle = hwnd;
    createStatusBar(hwnd, "Started", 2);
    ShowWindow(hwnd, nFunsterStil);
    UpdateWindow(hwnd);
    createDIB(hwnd);
    thread_pool = threadpool_create(DEFAULT_THREAD_COUNT, DEFAULT_QUEUE_SIZE,0);
    onInitializeFractal();

    // Main message loop
    while ((main_window_handle != 0) && GetMessage (&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    threadpool_destroy(thread_pool,0);
    UnregisterClass(szClassName,hThisInstance);

    return (int) messages.wParam;
}

// Prevent race conditions during shutdown by caching window handle
void onClearMessageQueue(void) {
    MSG msg = {0};
    HWND currentHandle = main_window_handle;
    
    if (currentHandle == NULL) {
        return;
    }
    
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        switch (msg.message) {
            case WM_CLOSE:
                DestroyWindow(currentHandle);
                break;
            case WM_DESTROY:
            case WM_QUIT:
                main_window_handle = 0;
                PostQuitMessage(0);            
                break;
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MOUSEMOVE:
            default:
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
        
        if (main_window_handle == NULL) {
            break;
        }
    }
}

void onRepaint(void) {
    InvalidateRect(main_window_handle, NULL, FALSE);
}

static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case BCerrar:
                    EndDialog(hwndDlg,1);
                    return 1;
            }
            break;
        case WM_CLOSE:
            EndDialog(hwndDlg,0);
            return TRUE;
    }
    return FALSE;
}

void OnCommand(HWND hwnd, int wID, HWND hwndCtl, UINT wNotifyCode)
{
    switch(wID)
    {
        case m_acercade:
            DialogBox(instancia, MAKEINTRESOURCE(dialogo), NULL, (DLGPROC) DialogFunc);
            break;
        case m_guardar:
            saveFractal(generateSaveFilename(), TRUE);
            break;
        case m_nuevo:
            onInitializeFractal();
            break;
        case m_salir:
            PostMessage(hwnd,WM_CLOSE,0,0);
            break;
    }
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps = {0};
    UINT wNotifyCode = 0;
    int wID = 0;
    HWND hwndCtl = NULL;

    switch (message)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
        case WM_QUIT:
            main_window_handle = 0;
            PostQuitMessage(0);            
            break;
        case WM_CREATE:
            break;
        case WM_SIZE:
            SendMessage(hWndStatusbar,message,wParam,lParam);
            initializeStatusBar(hWndStatusbar,2);
            break;
        case WM_COMMAND:
            wNotifyCode = HIWORD(wParam);
            wID = LOWORD(wParam);
            hwndCtl = (HWND) lParam;
            OnCommand(hwnd, wID, hwndCtl, wNotifyCode);
            break;
        case WM_PAINT:
            BeginPaint(hwnd, &ps);
            drawFractalBitmap(hwnd);
            EndPaint(hwnd, &ps);
            break;
        case WM_LBUTTONDOWN:
            onFractalMouseDown(LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_LBUTTONUP:
            onFractalMouseUp();
            break;
        case WM_RBUTTONDOWN:
            onFractalCancelSelection();
            break;
        case WM_MOUSEMOVE:
            onFractalMouseMove(LOWORD(lParam), HIWORD(lParam), hwnd);
            break;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                onFractalCancelSelection();
            }
            break;
        case WM_CHAR:
            onFractalKeyPress((BYTE)wParam);
            break;
        case WM_KEYUP:
            break;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
#include "utils.h"
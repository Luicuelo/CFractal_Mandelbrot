//#include <commctrl.h>
#include "div.h"
#include "dibuja.h"
#include "wres.h"
#include "stsbar.h"
#include "main.h"
#include "threadpool.h"

#ifndef __cplusplus
    #include <stdbool.h>
#endif

struct threadpool_t *thread_pool;

HWND main_window_handle;
/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
HINSTANCE instancia;

/*  Make the class name into a global variable  */
char szClassName[ ] = "WindowsApp";

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)

{
    RECT r;
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
	instancia=hThisInstance;
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_HREDRAW|CS_VREDRAW |CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = MAKEINTRESOURCE(ImenuP);                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);//COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/

    r.left=0;
    r.top=0;
    r.right=window_width+1;
    r.bottom=window_height+1;
    AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW,TRUE);


    hwnd = CreateWindowEx (
               0,                   /* Extended possibilites for variation */
               szClassName,         /* Classname */
               "Fractal",          /* Title Text */
               WS_EX_DLGMODALFRAME, /* default window */
               CW_USEDEFAULT,       /* Windows decides the position */
               CW_USEDEFAULT,       /* where the window ends up on the screen */
               (r.right-r.left),                 /* The programs width */
               (r.bottom-r.top),                 /* and height in pixels */
               HWND_DESKTOP,        /* The window is a child-window to desktop */
               NULL,                /* No menu */
               hThisInstance,       /* Program Instance handler */
               NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */

    main_window_handle = hwnd;
    CreateSBar(hwnd, "Iniciado", 2);
    ShowWindow(hwnd, nFunsterStil);
    UpdateWindow(hwnd);
    CreateDIB(hwnd);
    thread_pool = threadpool_create(10, 200, 0);
    initializeFractalDrawing();

    /* Run the message loop. It will run until GetMessage() returns 0 */

    while ((main_window_handle != 0) && GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    threadpool_destroy(thread_pool,0);
    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    UnregisterClass(szClassName,hThisInstance);


    return (int) messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

void vaciaCola(void){
    MSG msg;
    while(PeekMessage (& msg, NULL, 0, 0,PM_REMOVE)){

        switch (msg.message)   {
            case WM_QUIT:
                main_window_handle = 0;
            break;
            case WM_DESTROY:
                main_window_handle = 0;
            break;
            case WM_LBUTTONDOWN:
            break;
            case WM_LBUTTONUP:
            break;
            case WM_MOUSEMOVE:
            break;
            default:
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
    }
}
void repinta(void)
{
    InvalidateRect(
        main_window_handle, // handle of window with changed update region
        NULL, // address of rectangle coordinates
        FALSE // erase-background flag
    );

}


static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam,
LPARAM lParam)
{
switch (msg) {
	case WM_INITDIALOG:
	case WM_COMMAND: // Event: a button is pressed
		switch (LOWORD(wParam)) {
			case BCerrar:
                EndDialog(hwndDlg,1);
			return 1;
		}
	break;
	case WM_CLOSE: // Event: the dialog is closed
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
			DialogBox(instancia,
				MAKEINTRESOURCE(dialogo),
				NULL,
				(DLGPROC) DialogFunc);
		break;
        case m_guardar:
            SaveDib(cadenaSave(),TRUE);
        break;
        case m_nuevo:
            initializeFractalDrawing();
        break;
        case m_salir:
            PostMessage(hwnd,WM_CLOSE,0,0);
        break;
    }
}



LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    UINT wNotifyCode;
    int wID;
    HWND hwndCtl;

    switch (message)
    {
        case WM_DESTROY:
            main_window_handle = 0;
        case WM_QUIT:
            main_window_handle = 0;
        break;
        case WM_CREATE:
        break;
        case WM_SIZE:
            SendMessage(hWndStatusbar,message,wParam,lParam);
            InitializeStatusBar(hWndStatusbar,2);
            break;
        break;
        case WM_COMMAND:
            ;
            wNotifyCode = HIWORD(wParam); // notification code
            wID = LOWORD(wParam);         // item, control, or   accelerator identifier
            hwndCtl = (HWND) lParam;      // handle of control
            OnCommand(hwnd, wID, hwndCtl, wNotifyCode);
            break;
        case WM_PAINT:
			BeginPaint(hwnd,&ps);
            DrawDIB(hwnd);
            EndPaint(hwnd,&ps);
        break;
        case WM_LBUTTONDOWN:
            fractalMouseDown(LOWORD(lParam), HIWORD(lParam)); // Cambiado a fractalMouseDown
        break;
        case WM_LBUTTONUP:
            fractalMouseUp(); // Cambiado a fractalMouseUp
        break;
        case WM_MOUSEMOVE:
            fractalMouseMove(LOWORD(lParam), HIWORD(lParam), hwnd); // Cambiado a fractalMouseMove
        break;
        case WM_CHAR:
            fractalTecla((BYTE)wParam); // Cambiado a fractalTecla
        case WM_KEYDOWN:
        case WM_KEYUP:

        default:
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

/*
void reescala(void)
{
    double factorx; // Declarar factorx antes de su uso
    double factory; // Declarar factory antes de su uso
    factorx = (double)nwid / (double)(window_width);
    factory = (double)nhgt / (double)(window_height);
    global_pixel_size = (int)(1 / factorx) + 1;
}
*/
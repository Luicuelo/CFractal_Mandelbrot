#include "constantes.h"
#include "stsbar.h"
#include "main.h" // Incluir el encabezado donde se declara main_window_handle

#ifndef __cplusplus
    #include <stdbool.h>
#endif

// Declarar la variable como externa.
extern HWND main_window_handle;

// Handle para la barra de estado.
HWND hWndStatusbar;

// Función para crear la barra de estado.
BOOL CreateSBar(HWND hwndParent, char *initialText, int numberOfParts) {
    RECT parentRect;
    RECT statusBarRect;
    int parentWidth;
    int parentHeight;
    unsigned int flags = SWP_NOOWNERZORDER;

    // Crear la barra de estado.
    hWndStatusbar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_BORDER,
                                       initialText,
                                       hwndParent,
                                       IDM_STATUSBAR);

    // Ajustar el tamaño de la ventana principal para acomodar la barra de estado.
    GetWindowRect(hwndParent, &parentRect);
    GetWindowRect(hWndStatusbar, &statusBarRect);
    parentWidth = parentRect.right - parentRect.left;
    parentHeight = (parentRect.bottom - parentRect.top) + (statusBarRect.bottom - statusBarRect.top);
    SetWindowPos(hwndParent, 0, 0, 0, parentWidth, parentHeight, flags);

    if (hWndStatusbar) {
        InitializeStatusBar(hwndParent, numberOfParts);
        UpdateStatusBar(initialText, 0, 0);
        if (main_window_handle != 0)
            drawFractal(main_window_handle);
        return TRUE;
    }

    return FALSE;
}

// Función para inicializar la barra de estado con múltiples partes.
void InitializeStatusBar(HWND hwndParent, int numberOfParts) {
    int partWidths[40]; // Array para definir el ancho de las partes.
    HDC deviceContext;

    // Validate input parameters
    if (hwndParent == NULL || numberOfParts <= 0 || numberOfParts > 40) {
        return;
    }

    // Obtener el contexto del dispositivo de la ventana principal.
    deviceContext = GetDC(hwndParent);
    if (deviceContext == NULL) {
        return;
    }

    // Initialize array to prevent uninitialized access
    memset(partWidths, 0, sizeof(partWidths));

    // Configurar los anchos de las partes de la barra de estado.
    if (numberOfParts >= 1) partWidths[0] = window_width / 2;
    if (numberOfParts >= 2) partWidths[1] = window_width;

    // Liberar el contexto del dispositivo.
    ReleaseDC(hwndParent, deviceContext);

    // Configurar las partes de la barra de estado.
    if (hWndStatusbar != NULL) {
        SendMessage(hWndStatusbar,
                    SB_SETPARTS,
                    numberOfParts,
                    (LPARAM)(LPINT)partWidths);
    }
}

// Función para actualizar el texto de la barra de estado.
void UpdateStatusBar(LPSTR statusText, WORD partNumber, WORD displayFlags) {
    SendMessage(hWndStatusbar,
                SB_SETTEXT,
                partNumber | displayFlags,
                (LPARAM)statusText);
}

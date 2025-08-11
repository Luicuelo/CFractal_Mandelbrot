#include "stsbar.h"

// Function to create the status bar.
BOOL createStatusBar(HWND parentWindow, char *initialText, int numberOfParts) {
    RECT parentRect;
    RECT statusBarRect;
    int parentWidth;
    int parentHeight;
    unsigned int flags = SWP_NOOWNERZORDER;

    // Create the status bar.
    hWndStatusbar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_BORDER,
                                       initialText,
                                       parentWindow,
                                       IDM_STATUSBAR);

    // Adjust the main window size to accommodate the status bar.
    GetWindowRect(parentWindow, &parentRect);
    GetWindowRect(hWndStatusbar, &statusBarRect);
    parentWidth = parentRect.right - parentRect.left;
    parentHeight = (parentRect.bottom - parentRect.top) + (statusBarRect.bottom - statusBarRect.top);
    SetWindowPos(parentWindow, 0, 0, 0, parentWidth, parentHeight, flags);

    if (hWndStatusbar) {
        initializeStatusBar(parentWindow, numberOfParts);
        updateStatusBar(initialText, 0, 0);
        return TRUE;
    }

    return FALSE;
}

// Function to initialize the status bar with multiple parts.
void initializeStatusBar(HWND parentWindow, int numberOfParts) {
    int partWidths[40]; // Array to define the width of the parts.
    HDC deviceContext;

    // Validate input parameters
    if (parentWindow == NULL || numberOfParts <= 0 || numberOfParts > 40) {
        return;
    }

    // Get the device context of the main window.
    deviceContext = GetDC(parentWindow);
    if (deviceContext == NULL) {
        return;
    }

    // Initialize array to prevent uninitialized access
    memset(partWidths, 0, sizeof(partWidths));

    // Set the widths of the status bar parts.
    if (numberOfParts >= 1) partWidths[0] = WINDOW_WIDTH / 2;
    if (numberOfParts >= 2) partWidths[1] = WINDOW_WIDTH;

    // Release the device context.
    ReleaseDC(parentWindow, deviceContext);

    // Set the parts of the status bar.
    if (hWndStatusbar != NULL) {
        SendMessage(hWndStatusbar,
                    SB_SETPARTS,
                    numberOfParts,
                    (LPARAM)(LPINT)partWidths);
    }
}

// Function to update the status bar text.
void updateStatusBar(LPSTR statusText, WORD partNumber, WORD displayFlags) {
    SendMessage(hWndStatusbar,
                SB_SETTEXT,
                partNumber | displayFlags,
                (LPARAM)statusText);
}

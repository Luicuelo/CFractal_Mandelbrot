#include "constants.h"
#include <windows.h>
#include <commctrl.h>
#include <string.h>

#define IDM_STATUSBAR 3000

HWND hWndStatusbar;

BOOL createStatusBar(HWND parentWindow, char *initialText, int numberOfParts);
void initializeStatusBar(HWND parentWindow, int numberOfParts);
void updateStatusBar(LPSTR statusText, WORD partNumber, WORD displayFlags);


#ifndef STSBAR_H
#define STSBAR_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "common.h"
#include <commctrl.h>

#define IDM_STATUSBAR 3000

extern HWND hWndStatusbar;

BOOL createStatusBar(HWND parentWindow, char *initialText, int numberOfParts);
void initializeStatusBar(HWND parentWindow, int numberOfParts);
void updateStatusBar(LPSTR statusText, WORD partNumber, WORD displayFlags);

#endif // STSBAR_H

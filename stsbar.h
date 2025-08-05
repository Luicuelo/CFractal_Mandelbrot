#include "constantes.h"
#include <windows.h>
#include <commctrl.h>
#include <string.h>

#define IDM_STATUSBAR 3000

HWND hWndStatusbar;

BOOL createSBar(HWND hwndParent, char *initialText, int nrOfParts);
void initializeStatusBar(HWND hwndParent, int nrOfParts);
void updateStatusBar(LPSTR lpszStatusString, WORD partNumber, WORD displayFlags);


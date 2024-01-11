#include "constantes.h"
#include <windows.h>
#include <commctrl.h>
#define IDM_STATUSBAR 3000


extern HWND  hWndStatusbar;
BOOL CreateSBar(HWND hwndParent,char *initialText,int nrOfParts);
void InitializeStatusBar(HWND hwndParent,int nrOfParts);
void UpdateStatusBar(LPSTR lpszStatusString, WORD partNumber, WORD displayFlags);

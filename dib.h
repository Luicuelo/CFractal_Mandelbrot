#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "common.h"

void createDIB(HWND);
void drawFractalBitmap(HWND hWnd);
void saveFractal(LPCTSTR lpszFileName, BOOL bOverwriteExisting);
void drawSquare(int x, int y, int tam, BYTE color);
void fillColors(void);
//void drawBorder(int x, int y, int w, int h, BYTE color);








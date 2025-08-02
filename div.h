#include "constantes.h"
#include <windows.h>
#include <string.h>



#ifndef __cplusplus
    #include <stdbool.h>
#endif

void createDIB(HWND);
void drawFractal(HWND hWnd);
void saveFractal(LPCTSTR lpszFileName, BOOL bOverwriteExisting);
void fillColors(void);
void fillColorsAlternate(void);
void drawSquare(int x, int y, int w, int h, BYTE color);
void drawBorder(int x, int y, int w, int h, BYTE color);
void drawPixel(int x, int y, BYTE color);





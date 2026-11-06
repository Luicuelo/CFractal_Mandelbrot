#ifndef DIB_H
#define DIB_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "common.h"

// Shared fractal state owned by fractal_calc.c
extern int max_iterations;
extern bool invertColors;
extern int color_offset;

void createDIB(HWND);
void drawFractalBitmap(HWND hWnd);
void saveFractal(LPCTSTR lpszFileName, BOOL bOverwriteExisting);
void drawSquare(int x, int y, int tam, uint32_t iterations);
void fillColors(void);

#endif // DIB_H

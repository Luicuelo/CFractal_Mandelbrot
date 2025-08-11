
/**
 * @file fractal_calc.h
 * @brief Mandelbrot fractal rendering and interaction functions
 */

#ifndef FRACTAL_CALC_H
#define FRACTAL_CALC_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "dib.h"
#include "stsbar.h"
#include "main.h"
#include "threadpool.h"

#include "common.h"
#include "complex.h"
#include "types.h"


#define MANDELBROT_ESCAPE_RADIUS_SQUARED 4.0  // 2^2 = 4
bool invertColors;
// Common types moved to types.h to avoid circular dependencies

// Public interface functions
void onFractalMouseMove(int X, int Y, HWND hwnd);
void onFractalMouseDown(int X, int Y);
void onFractalMouseUp(void);
void onFractalCancelSelection(void);
void onFractalKeyPress(BYTE tecla);
void onClearMemory(void);
void onInitializeFractal(void);
char *generateSaveFilename(void);
int getOptimalThreadCount(void);


#endif // FRACTAL_CALC_H

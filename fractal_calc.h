
/**
 * @file fractal_calc.h
 * @brief Mandelbrot fractal rendering and interaction functions
 */

#ifndef FRACTAL_CALC_H
#define FRACTAL_CALC_H


#include <windows.h>
#include "constants.h"
#include "complex.h"

#ifndef __cplusplus
    #include <stdbool.h>
#endif

#define MANDELBROT_ESCAPE_RADIUS_SQUARED 4.0  // 2^2 = 4
bool invertColors;
typedef struct _renderFractalInternalParams{
  int current_block_size;
  int threadId;
} RenderFractalInternalParams;

typedef struct _point {
  int x;
  int y;
  int blockSize;
} Point;

void renderFractal(void);
void onFractalMouseMove(int X, int Y, HWND hwnd);
void onFractalMouseDown(int X, int Y);
void onFractalMouseUp(void);
void onFractalCancelSelection(void);
void onFractalKeyPress(BYTE tecla);
BYTE calculateMandelbrotPoint(int c, int f);
void onClearMemory(void);
void onInitializeFractal(void);
void onRepaint(void);
void automaticZoom(void);
void expandMemory(int startX, int startY, int newWidth, int newHeight, double scaleX, double scaleY);
void calculateAndDraw(Point *p);
void calculateSquarePoint(Point *p);
void calculatePixelPoint(Point *p);
void rescaleView(int pmouse_down_x, int pmouse_down_y, int pmouse_up_x, int pmouse_up_y);
void handleMouseMove(int x, int y, HWND hwnd);
void onMouseUp(void);
void onMouseDown(int x, int y);
void animateColorRotation(void);
char *generateSaveFilename(void);
char *cadenaSave(void);
int drawSelectionRectangle(HDC hDC);
void onSaveFractal(LPCTSTR lpszFileName, BOOL bOverwriteExisting);

#ifdef useUniformBlockOptimization
void fillMemorySquare(int x, int y, int w, int h, BYTE value);
void optimizeWhenCornersEqual(int xc, int yc, int blockSize, BYTE current);
#endif

#endif // FRACTAL_CALC_H

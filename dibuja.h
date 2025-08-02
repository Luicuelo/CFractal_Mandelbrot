#ifndef DIBUJA_H
#define DIBUJA_H

#include <windows.h>
#include "constantes.h"
#include "complejos.h"

#ifndef __cplusplus
    #include <stdbool.h>
#endif

//#define useConvergenceThreshold // Uncomment to enable convergence checking, this should accelerate the calculations
#ifdef useConvergenceThreshold
  const double CONVERGENCE_THRESHOLD = 8;
#endif

#define ESCAPE_RADIUS_SQUARED 4.0  // 2^2 = 4
#define menormax(c) (c < 0 ? 0 : (c > 254 ? 254 : c)) // 255 es un valor reservado.

bool invert;
typedef struct _point {
  int x;
  int y;
  int tam;
} Point;

void renderFractal(void);
void onFractalMouseMove(int X, int Y, HWND hwnd);
void onFractalMouseDown(int X, int Y);
void onFractalMouseUp(void);
void onFractalKeyPress(BYTE tecla);
BYTE calculateMandelbrotPoint(int c, int f);
void onClearMemory(void);
void onInitializeFractal(void);
void onRepaint(void);

int calculateColor(BYTE maxiters);
void expandMemory(int startX, int startY, int newWidth, int newHeight, double scaleX, double scaleY);
void calculateAndDraw(Point *p);
void calculateSquarePoint(Point *p);
void calculatePixelPoint(Point *p);
void rescaleView(void);
void handleMouseMove(int x, int y, HWND hwnd);
void onMouseUp(void);
void onMouseDown(int x, int y);
void animateColorRotation(void);
char *generateSaveFilename(void);
char *cadenaSave(void);
int drawSelectionRectangle(HDC hDC);
void onSaveFractal(LPCTSTR lpszFileName, BOOL bOverwriteExisting);

#endif // DIBUJA_H
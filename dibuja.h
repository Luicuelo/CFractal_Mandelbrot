#include <stdio.h>
#include <wingdi.h>
#include "constantes.h"
#include "threadpool.h"
//---------------------------------------------------------------------------

void renderFractal(void);
char * generateSaveFilename(void);
void comienza(void);
void onFractalMouseMove(int X, int Y, HWND hwnd);
void onFractalMouseDown(int X, int Y);
void onFractalMouseUp(void);
void fractalMouseUp(void);
void onFractalKeyPress(BYTE tecla);
void graba(void);
BYTE calculateMandelbrotPoint(int c, int f);
void onClearMemory(void);
void onInitializeFractal(void);

#ifndef DIBUJA_H
#define DIBUJA_H

typedef struct _punto {
  int x;
  int y;
  int tam;
} Punto;

void onRepaint(void); // Declaración de onRepaint

// Resto de las declaraciones y definiciones en dibuja.h

#endif // DIBUJA_H
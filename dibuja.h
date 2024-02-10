#include <stdio.h>
#include <wingdi.h>
#include "constantes.h"
#include "threadpool.h"
//---------------------------------------------------------------------------

void dibuja(void);
char * cadenaSave(void);
void comienza(void);
void fractalMouseMove(int X, int Y,HWND hwnd);
void fractalMouseDown(int X, int Y);
void fractalMouseUp(void);
void fractalTecla(BYTE tecla);
void graba(void);
BYTE  calculaPuntoM(int c, int f);
void vaciaMemoria();

#ifndef DIBUJA_H
#define DIBUJA_H

typedef struct _punto {
  int x;
  int y;
  int tam;
} Punto;

// Resto de las declaraciones y definiciones en dibuja.h

#endif // DIBUJA_H
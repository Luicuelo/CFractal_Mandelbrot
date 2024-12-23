#include "constantes.h"
#include <windows.h>
#define rectangulo(cl,fl,wd,hg,color) if (cl+wd>window_width) wd=window_width-cl; for (trect=(fl);trect<(fl+hg+1);trect++) memset((&Pixels[trect][0])+cl,color,wd+1);+cl,color,wd+1);




#define drawPixel(x,y,color) Pixels[y][x]=color;
#define setPixel(x,y,color) Pixels[y][x]=color; // Legacy compatibility
#ifndef __cplusplus
    #include <stdbool.h>
#endif


void CreateDIB(HWND);

extern bool invierte;
extern int colini;ffset;
extern BYTE Pixels[window_width][window_height];
extern int global_pixel_size;

void cuadradoR(int x,int y,int w,int h,BYTE color);
void bordeR(int x,int y,int w,int h,BYTE color);
void drawFractal(HWND hWnd);
void saveFractal(LPCTSTR lpszFileName, BOOL bOverwriteExisting);
void fillColors(void);
void llenaColores2(void);
void fillColorsAlternate(void);
void comienza(void);
void dibuja(void);
void comienza(void);
void fractalMouseMove(int X, int Y,HWND hwnd);
void fractalMouseDown(int X, int Y);
void fractalMouseUp(void);
void graba(void);
void drawSquare(int x, int y, int w, int h, BYTE color);
void drawBorder(int x, int y, int w, int h, BYTE color);


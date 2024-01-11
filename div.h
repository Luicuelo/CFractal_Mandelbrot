#include "constantes.h"
#include <windows.h>
#define rectangulo(cl,fl,wd,hg,color) if (cl+wd>wid) wd=wid-cl; for (trect=(fl);trect<(fl+hg+1);trect++) memset((&Pixels[trect][0])+cl,color,wd+1);




#define setPixel(x,y,color) Pixels[y][x]=color;
#ifndef __cplusplus
    #include <stdbool.h>
#endif


void CreateDIB(HWND);

extern bool invierte;
extern int colini;
extern BYTE Pixels[wid][hgt];

void cuadradoR(int x,int y,int w,int h,BYTE color);
void bordeR(int x,int y,int w,int h,BYTE color);
void DrawDIB(HWND hWnd);
void SaveDib (LPCTSTR lpszFileName, BOOL bOverwriteExisting);
void llenaColores(void);
void llenaColores2(void);
void comienza(void);
void dibuja(void);
void comienza(void);
void fractalMouseMove(int X, int Y,HWND hwnd);
void fractalMouseDown(int X, int Y);
void fractalMouseUp(void);
void graba(void);


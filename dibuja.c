// This is a C Program
///      Para aprovechar los puntos que ya estan calculados cuando hacemos un zoom utilizamos dos matrices
///      Esto da lugar a imprecisiones en el c�lculo. Para activar definir rapido a la hora de compilar.
///      Tambien se puede acelerar el calculo saliendo del bucle que calcula si una sucesion diverge cuando
///      la distancia entre dos terminos es mayor que una fijada (5 por ejemplo).

//#define zoomrapido
#undef zoomrapido

#include "complejos.h"
#include "div.h"
#include "stsbar.h"
#include "main.h"
#include "dibuja.h"
#include <math.h>
#ifndef __cplusplus
	#include <stdbool.h>
#endif

BYTE Memory[hgt][wid];
int trect;


#define menormax(c) (c < 0 ? 0 : (c > 254 ? 254 : c)) // 255 es un valor reservado.


double convergencia = 0.2;
bool botonapretado;
int bxd;
int byd;
int bxu;
int byu;

int colini;
int tamPixelglobal;

double inx;
double iny;
double xi;
double yi;

const double TAMINI = 2.9;

bool imagencargada;
//int maxiter = 150;
char s[255]; // se utiliza en la barra de estado

// int aciertos;

void vaciaMemoria(){
	 memset(Memory, 0, hgt * wid * sizeof(BYTE));
}

BYTE calculaPuntoM(int c, int f)
{
	int inr;
	Comp z;
	Comp last;
	Comp aux;
	Comp Sz;
	nc(0, 0, Sz);

	z.x = inx * c + xi;
	z.y = iny * f + yi;
	asigna(last, z);


	// What is the Mandelbrot set? It's the the set of all complex numbers z for which sequence defined by the iteration
	// Sz(0) = z,    Sz(n+1) = Sz(n)*Sz(n) + z,    n=0,1,2, ...    (1)
	// remains bounded
	//-------------------
	
	//int maxiter = 2 + log2(log2(zoom)) * 128;
	int maxiter=50;

	
	double resta=0;
	//for (inr = 0; inr < maxiter && (mdr(last)<16); inr++)
	for(inr = 0; inr < maxiter  && md(Sz) <= 2  && resta<convergencia; inr++)
	{
		cuadSuma(last, z, Sz); // definida como una macro
		asigna(last, Sz);
		 //rest(last,Sz,aux);
		 //resta=md(aux);
		 resta=abs(md(Sz)-md(last));
	}

	if (inr >= maxiter)
	{
		// converge,conjunto
		return (255);
	}
	else
	{
		// diverge.
		//return menormax((C*(255/50))-inr);
		return (inr);
		//return menormax(inr);
	}
}

void calculaPuntoCuadrado(Punto *p){

	BYTE maxiters;
	BYTE color;
	int xc=p->x;
	int yc=p->y;

	maxiters=Memory[yc][xc];
	if(maxiters>0)
	{
			//ya calculado
	}
	else {
			maxiters=calculaPuntoM(xc, yc);
			Memory[yc][xc]=maxiters;
	}
	
	color=calculaColor(maxiters);
	cuadradoR(p->x , p->y , p->tam,  p->tam,color );	
}

int calculaColor(BYTE maxiters){	
	if (maxiters==255) {
		return 128;
	}
	return menormax(255-maxiters);
}

void calculaPuntoPixel(Punto *p){
		
	BYTE maxiters;
	BYTE color;
	int xc=p->x;
	int yc=p->y;
	maxiters=Memory[yc][xc];
	if(maxiters==255)
	{
			//ya calculado, maxiters>0||
			
	}
	else {
			maxiters=calculaPuntoM(xc, yc);
			Memory[yc][xc]=maxiters;
	}
	
	color=calculaColor(maxiters);
	setPixel(p->x , p->y,color);	
}


void dibuja(void)
{

	int a;
	int min = 1000;
	imagencargada = FALSE;

	int i;
	int j;
	int tamPixelInicial = 256;



	for (int r = 0; r < 9; r++)
	{

		int ttamPixelActual = tamPixelInicial / (pow(2 ,r));
		if (ttamPixelActual>=tamPixelglobal) continue;
		//if(soloUnaVez && ttamPixelActual>1)continue;
	    //Tenemos que evitar calcular el punto que ya esta calculado en el paso anterior. (El central)
		Punto p;
		for (i = 0; i*ttamPixelActual < wid - ttamPixelActual; i += 1) //eje x empieza a la izquierda
		{				
			for (j = 0; j*ttamPixelActual < hgt - ttamPixelActual; j += 1) // esto es el eje i, empieza en la esquina superior y va bajando		
			{				
				p.x=i*ttamPixelActual;
				p.y=j*ttamPixelActual;	
				p.tam=ttamPixelActual;
				//zoom=((double)((TAMINI)/(wid))/inx)*100;
				//p.zoom=zoom;			
				if (ttamPixelActual <= 1) {		
					//este punto ya se calculo en el tamaño anterior.
					//if (!(i % 2 == 0) && !(j % 2 == 0)) continue;													
					threadpool_add(hilos, calculaPuntoPixel,&p,sizeof(Punto));
				}			
				else {
					threadpool_add(hilos, calculaPuntoCuadrado,&p,sizeof(Punto));
				}  
			}
		}


		threadpool_wait_all(hilos);
		DrawDIB(principal);
		vaciaCola();
	
		
	}
	wsprintf(s, "Fractal %d %%", 100);
	UpdateStatusBar(s, 0, 0);
	imagencargada = TRUE;
}

int pintaSel(HDC hDC)
{
	HPEN oldpen;
	HBRUSH oldbr;
	if (imagencargada)
	{
		if (botonapretado)
		{

			int rs;
			HBRUSH br;
			HPEN pen1 = CreatePen(PS_DASH, 1, RGB(50, 255, 50));
			br = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
			oldpen = (HPEN)SelectObject(hDC, pen1);
			oldbr = (HBRUSH)SelectObject(hDC, br);
			SelectObject(hDC, br);
			rs = Rectangle(hDC, bxd, byd, bxu, byu);
			SelectObject(hDC, oldpen);
			DeleteObject(pen1);
			SelectObject(hDC, oldbr);
			DeleteObject(br);
			return (rs);
		}
	}
	return (0);
}
//---------------------------------------------------------------------------


void comienza(void)
{
	bxd = 0;
	byd = 0;
	bxu = wid;
	byu = hgt;

	xi = -2.2;
	yi = -1.5;

	colini=0;
	inx = (double)((TAMINI) / (wid));
	iny = (double)((TAMINI) / (hgt));

	tamPixelglobal=wid;
	sprintf(s, "Plano Complejo: %1.2f/%1.2f:%1.2f/%1.2f ", xi, yi, xi + TAMINI, yi + TAMINI);
	UpdateStatusBar(s, 1, 0);
#ifdef zoomrapido
	borraPixels2();
#endif
	// invierte=true;
	vaciaMemoria();
	llenaColores();

	cuadradoR(0 , 0 ,wid,  hgt, 0);
	DrawDIB(principal);
	vaciaCola();
	dibuja();
}

// This function expands the Memory array to fill the entire space, leaving the points in between as zero
/*
void expandMemory(int startX, int startY, int newWidth, int newHeight, double scaleX, double scaleY) {
    BYTE **oldMemoryToExpand = (BYTE **)malloc(newHeight * sizeof(BYTE *));
    for (int i = 0; i < newHeight; i++) {
        oldMemoryToExpand[i] = (BYTE *)malloc(newWidth * sizeof(BYTE));
    }

    // Copy oldMemoryToExpand
    for (int i = 0; i < newHeight; i++) {
        memcpy(oldMemoryToExpand[i], &Memory[startY + i][startX], newWidth);  
    }
    vaciaMemoria();

    for (int i = 0; i < newHeight; i++) {
        for (int j = 0; j < newWidth; j++) {
			int posy=(int)(i / scaleY);
			int posx=(int)(j / scaleX);
            Memory[posy][posx] = oldMemoryToExpand[i][j];
			cuadradoR(posx,posy,tamPixelglobal,tamPixelglobal,calculaColor(Memory[posy][posx] ));
        }
    }
    // Free the memory for each row
    for (int i = 0; i < newHeight; i++) {
        free(oldMemoryToExpand[i]);
    }
    free(oldMemoryToExpand);
}
*/

void expandMemory(int startX, int startY, int newWidth, int newHeight, double scaleX, double scaleY) {

    BYTE *oldMemoryToExpand = (BYTE *)malloc(newHeight * newWidth* sizeof(BYTE));
 
    // Copy oldMemoryToExpand
    for (int i = 0; i < newHeight; i++) {
        memcpy(&oldMemoryToExpand[i*newWidth], &Memory[startY + i][startX], newWidth);  		
    }
    vaciaMemoria();

    for (int i = 0; i < newHeight; i++) {
        for (int j = 0; j < newWidth; j++) {
			int posy=(int)(i / scaleY);
			int posx=(int)(j / scaleX);
            Memory[posy][posx] = oldMemoryToExpand[i*newWidth+j];
			cuadradoR(posx,posy,tamPixelglobal,tamPixelglobal,calculaColor(Memory[posy][posx] ));
        }
    }

    free(oldMemoryToExpand);
}

void reescala(void)
{

	//xi , yi son las coordenadas del origen en el plano complejo.
	//inx , iny es la escala del plano complejo. Si multiplicamos la posición del pixel en la imagen nos da el punto correspondiente en el plano complejo.
	xi += inx * (double)bxd;
	yi += iny * (double)byd;

	//(bxu - bxd) es el ancho del nuevo rectangulo.
	//(byu - byd) es el alto del nuevo rectangulo.
	int nwid=(bxu - bxd+1) ;
	int nhgt=(byu - byd+1) ;

	double factorx=(double)nwid / (double)(wid);
	double factory=(double)nhgt / (double)(hgt);

	tamPixelglobal=(int)(1/factorx)+1;
	expandMemory (bxd,byd, nwid ,nhgt,factorx, factory);
	DrawDIB(principal);
	inx *= factorx;
	iny *= factory;
}
//---------------------------------------------------------------------------
void mueve(int x, int y, HWND hwnd)
{
	HDC hDC;
	double txi;
	double tyi;
	double txf;
	double tyf;

	if (imagencargada && botonapretado)
	{
		int anchoR;
		int altoR;
		bxu = x;
		byu = y;
		anchoR = (bxu - bxd);
		altoR = (byu - byd);
		((anchoR) < (altoR)) ? (bxu = bxd + altoR) : (byu = byd + anchoR);

		DrawDIB(hwnd);
		hDC = GetDC(hwnd);
		pintaSel(hDC);

		wsprintf(s, " Posicion: %d/%d : %d/%d", bxd, byd, bxu, byu);
		UpdateStatusBar(s, 0, 0);

		txi = (xi + inx * (double)bxd);
		tyi = (yi + iny * (double)byd);

		txf = (xi + inx * (double)bxu);
		tyf = (yi + iny * (double)byu);

		sprintf(s, "Plano Complejo: %1.2f/%1.2f:%1.2f/%1.2f ", txi, tyi, txf, tyf);
		UpdateStatusBar(s, 1, 0);

		ReleaseDC(hwnd, hDC);
	}
}

char *cadenaSave(void)
{

	double txi = (xi) * 100.0;
	double tyi = (yi) * 100.0;

	double txf = (inx * ((double)(wid)) + xi) * 100.0;
	double tyf = (iny * ((double)(hgt)) + yi) * 100.0;

	sprintf(s, "Fractal%2.0f_%2.0f_%2.0f_%2.0f.bmp ", txi, tyi, txf, tyf);
	return (s);
}
void arriba(void)
{

	if (imagencargada)
	{
		botonapretado = FALSE;
		reescala();
		dibuja();
	}
}

void abajo(int x, int y)
{
	if (imagencargada)
	{
		bxd = x;
		byd = y;
		botonapretado = TRUE;
	}
}

//---------------------------------------------------------------------------
bool rotCol = false;
void eRotCol(void)
{
	while ((rotCol) && (principal != 0))
	{
		colini = colini + 1;
		if (colini>254) colini=0;
		llenaColores();
		if (principal != 0)
			DrawDIB(principal);
		sprintf(s, "Color: %d ", colini);
		UpdateStatusBar(s, 0, 0);
		Sleep(50);
		vaciaCola();
	}
}

void fractalTecla(BYTE tecla)
{
	if (tecla == 't' || tecla == 'T')
	{
		rotCol = !rotCol;
		eRotCol();
	}
	if (tecla == 'i' || tecla == 'I')
	{
		invierte = !invierte;
		llenaColores();
		if (principal != 0)
			DrawDIB(principal);
	}
}

void fractalMouseMove(int X, int Y, HWND hwnd)
{
	mueve(X, Y, hwnd);
}
//---------------------------------------------------------------------------

void fractalMouseDown(int X, int Y)
{
	abajo(X, Y);
}
//---------------------------------------------------------------------------

void fractalMouseUp(void)
{
	arriba();
}
//---------------------------------------------------------------------------

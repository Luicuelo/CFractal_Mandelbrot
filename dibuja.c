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

BYTE Memory[window_height][window_width];
int temp_rect;


#define menormax(c) (c < 0 ? 0 : (c > 254 ? 254 : c)) // 255 es un valor reservado.


double convergence_threshold = 0.2;
bool botonapretado;
int mouse_down_x;
int mouse_down_y;
int mouse_up_x;
int mouse_up_y;

int color_offset;
int global_pixel_size;

double current_zoom = 1.0;
int max_iterations = 50;
double complex_step_x;
double complex_step_y;
double complex_origin_x;
double complex_origin_y;

const double TAMINI = 2.9;

bool imagencargada;
//int maxiter = 150;
char s[255]; // se utiliza en la barra de estado

// int aciertos;

extern HWND main_window_handle;
extern struct threadpool_t *thread_pool;

void vaciaMemoria(){
	 memset(Memory, 0, window_height * window_width * sizeof(BYTE));
}

BYTE calculaPuntoM(int c, int f)
{
	int inr;
	Comp z;
	Comp last;
	Comp aux;
	Comp Sz;
	nc(0, 0, Sz);

	z.x = complex_step_x * c + complex_origin_x;
	z.y = complex_step_y * f + complex_origin_y;
	asigna(last, z);


	// What is the Mandelbrot set? It's the the set of all complex numbers z for which sequence defined by the iteration
	// Sz(0) = z,    Sz(n+1) = Sz(n)*Sz(n) + z,    n=0,1,2, ...    (1)
	// remains bounded
	//-------------------
	
	//int maxiter = 2 + log2(log2(zoom)) * 128;
	//int maxiter=50;

	// Calcula el número máximo de iteraciones basado en el nivel de zoom

	//int maxiter =50;

	
	double resta=0;
	//for (inr = 0; inr < maxiter && (mdr(last)<16); inr++)
	for(inr = 0; inr < max_iterations  && md(Sz) <= 2  && resta < convergence_threshold; inr++)
	{
		cuadSuma(last, z, Sz); // definida como una macro
		asigna(last, Sz);
		 //rest(last,Sz,aux);
		 //resta=md(aux);
		 resta=abs(md(Sz)-md(last));
	}

	if (inr >= max_iterations)
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
		return 255; //negro
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
		if (ttamPixelActual>=global_pixel_size) continue;
		//if(soloUnaVez && ttamPixelActual>1)continue;
	    //Tenemos que evitar calcular el punto que ya esta calculado en el paso anterior. (El central)
		Punto p;
		for (i = 0; i*ttamPixelActual < window_width - ttamPixelActual; i += 1) //eje x empieza a la izquierda
		{				
			for (j = 0; j*ttamPixelActual < window_height - ttamPixelActual; j += 1) // esto es el eje i, empieza en la esquina superior y va bajando		
			{				
				p.x=i*ttamPixelActual;
				p.y=j*ttamPixelActual;	
				p.tam=ttamPixelActual;
				//zoom=((double)((TAMINI)/(window_width))/inx)*100;
				//p.zoom=zoom;			
				if (ttamPixelActual <= 1) {		
					//este punto ya se calculo en el tamaño anterior.
					//if (!(i % 2 == 0) && !(j % 2 == 0)) continue;													
					threadpool_add(thread_pool, calculaPuntoPixel,&p,sizeof(Punto));
				}			
				else {
					threadpool_add(thread_pool, calculaPuntoCuadrado,&p,sizeof(Punto));
				}  
			}
		}


		threadpool_wait_all(thread_pool);
		DrawDIB(main_window_handle);
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
			rs = Rectangle(hDC, mouse_down_x, mouse_down_y, mouse_up_x, mouse_up_y);
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


void initializeFractalDrawing(void)
{
	mouse_down_x = 0;
	mouse_down_y = 0;
	mouse_up_x = window_width;
	mouse_up_y = window_height;

	complex_origin_x = -2.2;
	complex_origin_y = -1.5;

	color_offset=0;
	complex_step_x = (double)((TAMINI) / (window_width));
	complex_step_y = (double)((TAMINI) / (window_height));

	global_pixel_size=window_width;
	sprintf(s, "Plano Complejo: %1.2f/%1.2f:%1.2f/%1.2f ", complex_origin_x, complex_origin_y, complex_origin_x + TAMINI, complex_origin_y + TAMINI);
	UpdateStatusBar(s, 1, 0);
#ifdef zoomrapido
	borraPixels2();
#endif
	// invierte=true;
	vaciaMemoria();
	llenaColores();

	cuadradoR(0 , 0 ,window_width,  window_height, 0);
	DrawDIB(main_window_handle);
	vaciaCola();
	dibuja();
}


void expandMemory(int startX, int startY, int newWidth, int newHeight, double scaleX, double scaleY) {

    BYTE *oldMemoryToExpand = (BYTE *)malloc(newHeight * newWidth* sizeof(BYTE));
   if (oldMemoryToExpand == NULL) {
        // Handle memory allocation failure
        return;
    }

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
			cuadradoR(posx,posy,global_pixel_size,global_pixel_size,calculaColor(Memory[posy][posx] ));
        }
    }

    free(oldMemoryToExpand);
}

void reescala(void)
{
	//complex_origin_x , complex_origin_y son las coordenadas del origen en el plano complejo.
	//complex_step_x , complex_step_y es la escala del plano complejo. Si multiplicamos la posición del pixel en la imagen nos da el punto correspondiente en el plano complejo.
	complex_origin_x += complex_step_x * (double)mouse_down_x;
	complex_origin_y += complex_step_y * (double)mouse_down_y;

	//(mouse_up_x - mouse_down_x) es el ancho del nuevo rectangulo.
	//(mouse_up_y - mouse_down_y) es el alto del nuevo rectangulo.
	int nwid=(mouse_up_x - mouse_down_x+1) ;
	int nhgt=(mouse_up_y - mouse_down_y+1) ;

	double factorx=(double)nwid / (double)(window_width);
	double factory=(double)nhgt / (double)(window_height);

	global_pixel_size=(int)(1/factorx)+1;
	expandMemory (mouse_down_x,mouse_down_y, nwid ,nhgt,factorx, factory);
	DrawDIB(main_window_handle);
	complex_step_x *= factorx;
	complex_step_y *= factory;

	// Ajustar max_iterations en función del nivel de zoom, con un límite superior
	current_zoom = 1.0 / ((factorx + factory) / 2.0);
	max_iterations = 150 + log2(log2(current_zoom)) * 50;
	if (max_iterations > 300) {
		max_iterations = 300; // Límite superior para max_iterations
	}
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
		mouse_up_x = x;
		mouse_up_y = y;
		anchoR = (mouse_up_x - mouse_down_x);
		altoR = (mouse_up_y - mouse_down_y);
		((anchoR) < (altoR)) ? (mouse_up_x = mouse_down_x + altoR) : (mouse_up_y = mouse_down_y + anchoR);

		DrawDIB(hwnd);
		hDC = GetDC(hwnd);
		pintaSel(hDC);

		wsprintf(s, " Posicion: %d/%d : %d/%d", mouse_down_x, mouse_down_y, mouse_up_x, mouse_up_y);
		UpdateStatusBar(s, 0, 0);

		txi = (complex_origin_x + complex_step_x * (double)mouse_down_x);
		tyi = (complex_origin_y + complex_step_y * (double)mouse_down_y);

		txf = (complex_origin_x + complex_step_x * ((double)window_width) + complex_origin_x) * 100.0;
		tyf = (complex_origin_y + complex_step_y * (double)mouse_up_y);

		sprintf(s, "Plano Complejo: %1.2f/%1.2f:%1.2f/%1.2f ", txi, tyi, txf, tyf);
		UpdateStatusBar(s, 1, 0);

		ReleaseDC(hwnd, hDC);
	}
}

char *cadenaSave(void)
{

	double txi = (complex_origin_x) * 100.0;
	double tyi = (complex_origin_y) * 100.0;

	double txf = (complex_step_x * ((double)(window_width)) + complex_origin_x) * 100.0;
	double tyf = (complex_step_y * ((double)(window_height)) + complex_origin_y) * 100.0;

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
		mouse_down_x = x;
		mouse_down_y = y;
		botonapretado = TRUE;
	}
}

//---------------------------------------------------------------------------
bool rotCol = false;
void eRotCol(void)
{
	while ((rotCol) && (main_window_handle != 0))
	{
		color_offset = color_offset + 1;
		if (color_offset>254) color_offset=0;
		llenaColores();
		if (main_window_handle != 0)
			DrawDIB(main_window_handle);
		sprintf(s, "Color: %d ", color_offset);
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
		if (main_window_handle != 0)
			DrawDIB(main_window_handle);
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

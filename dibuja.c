

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

// #define useConvergenceThreshold // Uncomment to enable convergence checking

const double ESCAPE_RADIUS_SQUARED = 4.0; // 2^2 = 4
#ifdef useConvergenceThreshold
const double CONVERGENCE_THRESHOLD = 1;
#endif
bool isColorRotationActive = false;
bool isButtonPressed;
int mouse_down_x;
int mouse_down_y;
int mouse_up_x;
int mouse_up_y;

int color_offset;
int global_pixel_size;

double current_zoom = 1.0;
double absolute_zoom = 1.0; // Track total zoom from initial state
int max_iterations = 50;
double complex_step_x;
double complex_step_y;
double complex_origin_x;
double complex_origin_y;

const double INITIALFRACTALSIZE = 2.9;

bool isImageLoaded;
//int maxiter = 150;
char s[255]; // se utiliza en la barra de estado

// int aciertos;

extern HWND main_window_handle;
extern struct threadpool_t *thread_pool;

// Clear the memory buffer that stores calculated Mandelbrot iterations
void onClearMemory() {
    memset(Memory, 0, window_height * window_width * sizeof(BYTE));
}

// Calculate if a point belongs to the Mandelbrot set
// Returns 255 if in set, otherwise returns iteration count when it diverged
BYTE calculateMandelbrotPoint(int c, int f)
{
	int iterations;
	Comp z;
	Comp previous;
	Comp difference;
	Comp current;
	nc(0, 0, current);

	// Convert pixel coordinates to complex plane coordinates
	z.x = complex_step_x * c + complex_origin_x;
	z.y = complex_step_y * f + complex_origin_y;
	asigna(previous, z);

	// Mandelbrot iteration: z(n+1) = z(n)^2 + c
	// Check if sequence remains bounded (|z| <= 2)
	
#ifdef useConvergenceThreshold
	double convergence_diff = 0.0;
	for(iterations = 0; iterations < max_iterations && mdSquared(current) <= ESCAPE_RADIUS_SQUARED && convergence_diff < CONVERGENCE_THRESHOLD; iterations++)
	{
		cuadSuma(previous, z, current);
		asigna(previous, current);
		rest(previous, current, difference);
		convergence_diff = mdSquared(difference);
	}
#else
	for(iterations = 0; iterations < max_iterations && mdSquared(current) <= ESCAPE_RADIUS_SQUARED; iterations++)
	{
		cuadSuma(previous, z, current);
		asigna(previous, current);
	}
#endif

	if (iterations >= max_iterations)
	{
		// Point is in the Mandelbrot set
		return (255);
	}
	else
	{
		// Point diverged
		return (iterations);
	}
}

// Calculate Mandelbrot value and draw a square block of pixels
void calculateAndDrawSquare(Punto *p){

	BYTE maxiters;
	BYTE color;
	int xc=p->x;
	int yc=p->y;

	// Check if point already calculated
	maxiters=Memory[yc][xc];
	if(maxiters>0)
	{
			// Already calculated
	}
	else {
			// Calculate Mandelbrot value and store in memory
			maxiters=calculateMandelbrotPoint(xc, yc);
			Memory[yc][xc]=maxiters;
	}
	
	color=calculateColor(maxiters);
	drawSquare(p->x , p->y , p->tam,  p->tam,color );	
}

// Convert iteration count to color value (0-255)
// Scales colors based on current max_iterations for smooth gradients
int calculateColor(BYTE maxiters){	
	if (maxiters==255) {
		return 255; // Black for points in the set
	}
	// Scale color based on max_iterations for better distribution
	int scaled_color = (int)((double)maxiters * 254.0 / (double)max_iterations);
	return menormax(254 - scaled_color);
}

// Calculate Mandelbrot value and draw a single pixel
void calculateAndDrawPixel(Punto *p){
		
	BYTE maxiters;
	BYTE color;
	int xc=p->x;
	int yc=p->y;
	
	// Check if point already calculated
	maxiters=Memory[yc][xc];
	if(maxiters==255)
	{
			// Already calculated
			
	}
	else {
			// Calculate Mandelbrot value and store in memory
			maxiters=calculateMandelbrotPoint(xc, yc);
			Memory[yc][xc]=maxiters;
	}
	
	color=calculateColor(maxiters);
	drawPixel(p->x , p->y,color);	
}


// Progressive fractal rendering using multiple passes with decreasing block sizes
// Starts with large blocks and refines to individual pixels for smooth user experience
void renderFractal(void)
{

	int a;
	int min = 1000;
	isImageLoaded = FALSE;

	int i;
	int j;
	int initial_block_size = 256;

	// Progressive rendering: 9 passes from 256x256 blocks down to 1x1 pixels
	for (int r = 0; r < 9; r++)
	{

		int current_block_size = initial_block_size >> r; // Bit shift is faster than pow(2,r)
		if (current_block_size>=global_pixel_size) continue;
		
		// Create work items for thread pool
		Punto p;
		for (i = 0; i*current_block_size < window_width - current_block_size; i += 1)
		{				
			for (j = 0; j*current_block_size < window_height - current_block_size; j += 1)
			{				
				p.x=i*current_block_size;
				p.y=j*current_block_size;	
				p.tam=current_block_size;
				
				if (current_block_size <= 1) {		
					// Final pass: individual pixels
					threadpool_add(thread_pool, calculateAndDrawPixel,&p,sizeof(Punto));
				}			
				else {
					// Early passes: square blocks
					threadpool_add(thread_pool, calculateAndDrawSquare,&p,sizeof(Punto));
				}  
			}
		}

		// Wait for all threads to complete this pass
		threadpool_wait_all(thread_pool);
		drawFractal(main_window_handle);
		onClearMessageQueue();
	}
	wsprintf(s, "Fractal %d %%", 100);
	UpdateStatusBar(s, 0, 0);
	isImageLoaded = TRUE;
}

// Draw the selection rectangle when user is dragging to select zoom area
int drawSelectionRectangle(HDC hDC)
{
	HPEN oldpen;
	HBRUSH oldbr;
	if (isImageLoaded)
	{
		if (isButtonPressed)
		{

			int rs;
			HBRUSH br;
			// Create green dashed pen for selection rectangle
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


// Initialize fractal to default view (full Mandelbrot set)
void onInitializeFractal(void) {
    // Reset mouse selection area
    mouse_down_x = 0;
    mouse_down_y = 0;
    mouse_up_x = window_width;
    mouse_up_y = window_height;

    // Set initial complex plane coordinates (standard Mandelbrot view)
    complex_origin_x = -2.2;
    complex_origin_y = -1.5;

    color_offset = 0;
    isColorRotationActive = false; // Reset color rotation
    absolute_zoom = 1.0; // Reset absolute zoom
    
    // Calculate step size for converting pixels to complex coordinates
    complex_step_x = (double)((INITIALFRACTALSIZE) / (window_width));
    complex_step_y = (double)((INITIALFRACTALSIZE) / (window_height));

    global_pixel_size = window_width;
    sprintf(s, "Complex Plane: %1.2f/%1.2f:%1.2f/%1.2f ", complex_origin_x, complex_origin_y, complex_origin_x + INITIALFRACTALSIZE, complex_origin_y + INITIALFRACTALSIZE);
    UpdateStatusBar(s, 1, 0);

    onClearMemory();
    fillColors();

    drawSquare(0, 0, window_width, window_height, 0);
    drawFractal(main_window_handle);
    onClearMessageQueue();
    renderFractal();
}


// Expand previously calculated memory region to fill screen when zooming
// Reuses existing calculations to provide immediate visual feedback
void expandMemory(int startX, int startY, int newWidth, int newHeight, double scaleX, double scaleY) {
    // Validate input parameters to prevent overflow
    if (startX < 0 || startY < 0 || newWidth <= 0 || newHeight <= 0 ||
        startX >= window_width || startY >= window_height ||
        scaleX <= 0.0 || scaleY <= 0.0) {
        return;
    }

    // Check for potential integer overflow in malloc calculation
    if (newWidth > INT_MAX / newHeight || (size_t)newWidth * newHeight > SIZE_MAX / sizeof(BYTE)) {
        return;
    }

    BYTE *oldMemoryToExpand = (BYTE *)malloc((size_t)newHeight * newWidth * sizeof(BYTE));
    if (oldMemoryToExpand == NULL) {
        // Handle memory allocation failure
        return;
    }

    // Copy selected region from current memory
    for (int i = 0; i < newHeight && (startY + i) < window_height; i++) {
        int copyWidth = (startX + newWidth <= window_width) ? newWidth : (window_width - startX);
        if (copyWidth > 0) {
            memcpy(&oldMemoryToExpand[i*newWidth], &Memory[startY + i][startX], copyWidth);
        }
    }
    onClearMemory();

    // Scale up the copied region to fill the entire screen
    for (int i = 0; i < newHeight; i++) {
        for (int j = 0; j < newWidth; j++) {
            int posy = (int)(i / scaleY);
            int posx = (int)(j / scaleX);
            
            // Bounds checking to prevent buffer overflow
            if (posy >= 0 && posy < window_height && posx >= 0 && posx < window_width) {
                Memory[posy][posx] = oldMemoryToExpand[i*newWidth+j];
                drawSquare(posx,posy,global_pixel_size,global_pixel_size,calculateColor(Memory[posy][posx]));
            }
        }
    }

    free(oldMemoryToExpand);
}

// Rescale and zoom into the selected area of the fractal
void rescaleView(void)
{
	// Update complex plane origin based on mouse selection
	complex_origin_x += complex_step_x * (double)mouse_down_x;
	complex_origin_y += complex_step_y * (double)mouse_down_y;

	// Calculate dimensions of selected rectangle
	int newWidth = (mouse_up_x - mouse_down_x + 1);
	int newHeight = (mouse_up_y - mouse_down_y + 1);

	// Calculate zoom factors
	double factorX = (double)newWidth / (double)(window_width);
	double factorY = (double)newHeight / (double)(window_height);

	// Update pixel size for progressive rendering
	global_pixel_size = (int)(1/factorX) + 1;
	
	// Expand existing calculations to fill screen
	expandMemory(mouse_down_x, mouse_down_y, newWidth, newHeight, factorX, factorY);
	drawFractal(main_window_handle);
	
	// Update complex plane step size
	complex_step_x *= factorX;
	complex_step_y *= factorY;

	// Calculate absolute zoom from initial state
	double zoom_factor = 1.0 / ((factorX + factorY) / 2.0);
	absolute_zoom *= zoom_factor;
	
	// Adaptive max_iterations based on absolute zoom level
	max_iterations = (int)(50 + log2(absolute_zoom) * 50);
	if (max_iterations < 50) max_iterations = 50;
	if (max_iterations > 2000) max_iterations = 2000;
	
	// Debug: show zoom and iterations in status bar
	sprintf(s, "Zoom: %.1fx, Iterations: %d", absolute_zoom, max_iterations);
	UpdateStatusBar(s, 0, 0);
}
//---------------------------------------------------------------------------
// Handle mouse movement during selection rectangle drawing
void handleMouseMove(int x, int y, HWND hwnd)
{
	HDC hDC;
	double complexStartX, complexStartY;
	double complexEndX, complexEndY;

	if (isImageLoaded && isButtonPressed)
	{
		int rectWidth, rectHeight;
		
		// Update mouse position
		mouse_up_x = x;
		mouse_up_y = y;
		
		// Make selection rectangle square (maintain aspect ratio)
		rectWidth = (mouse_up_x - mouse_down_x);
		rectHeight = (mouse_up_y - mouse_down_y);
		((rectWidth) < (rectHeight)) ? (mouse_up_x = mouse_down_x + rectHeight) : (mouse_up_y = mouse_down_y + rectWidth);

		// Redraw fractal and selection rectangle
		drawFractal(hwnd);
		hDC = GetDC(hwnd);
		drawSelectionRectangle(hDC);

		// Update status bar with pixel coordinates
		wsprintf(s, " Position: %d/%d : %d/%d", mouse_down_x, mouse_down_y, mouse_up_x, mouse_up_y);
		UpdateStatusBar(s, 0, 0);

		// Calculate complex plane coordinates for status display
		complexStartX = (complex_origin_x + complex_step_x * (double)mouse_down_x);
		complexStartY = (complex_origin_y + complex_step_y * (double)mouse_down_y);

		complexEndX = (complex_origin_x + complex_step_x * ((double)window_width) + complex_origin_x) * 100.0;
		complexEndY = (complex_origin_y + complex_step_y * (double)mouse_up_y);

		sprintf(s, "Complex Plane: %1.2f/%1.2f:%1.2f/%1.2f ", complexStartX, complexStartY, complexEndX, complexEndY);
		UpdateStatusBar(s, 1, 0);

		ReleaseDC(hwnd, hDC);
	}
}

// Generate filename for saving fractal based on current view coordinates
char *generateSaveFilename(void)
{

	double txi = (complex_origin_x) * 100.0;
	double tyi = (complex_origin_y) * 100.0;

	double txf = (complex_step_x * ((double)(window_width)) + complex_origin_x) * 100.0;
	double tyf = (complex_step_y * ((double)(window_height)) + complex_origin_y) * 100.0;

	sprintf(s, "Fractal%2.0f_%2.0f_%2.0f_%2.0f.bmp ", txi, tyi, txf, tyf);
	return (s);
}
// Handle mouse button release - complete zoom operation
void onMouseUp(void)
{
	if (isImageLoaded)
	{
		isButtonPressed = FALSE;
		rescaleView();
		onRepaint();
		renderFractal();
	}
}

// Handle mouse button press - start selection rectangle
void onMouseDown(int x, int y)
{
	if (isImageLoaded)
	{
		mouse_down_x = x;
		mouse_down_y = y;
		isButtonPressed = TRUE;
	}
}

//---------------------------------------------------------------------------

// Animate color rotation effect
void animateColorRotation(void)
{
	while ((isColorRotationActive) && (main_window_handle != 0))
	{
		color_offset = color_offset + 1;
		if (color_offset>254) color_offset=0;
		fillColors();
		if (main_window_handle != 0)
			drawFractal(main_window_handle);
		sprintf(s, "Color: %d ", color_offset);
		UpdateStatusBar(s, 0, 0);
		Sleep(50);
		onClearMessageQueue(); // Cambiado de vaciaCola
	}
}

void onFractalKeyPress(BYTE tecla) {
    if (tecla == 't' || tecla == 'T') {
        isColorRotationActive = !isColorRotationActive;
        animateColorRotation();
    }
    if (tecla == 'i' || tecla == 'I') {
        invierte = !invierte;
        fillColors();
        if (main_window_handle != 0)
            drawFractal(main_window_handle);
    }
}

void onFractalMouseMove(int X, int Y, HWND hwnd) {
    handleMouseMove(X, Y, hwnd);
}
//---------------------------------------------------------------------------

void onFractalMouseDown(int X, int Y) {
    onMouseDown(X, Y);
}
//---------------------------------------------------------------------------

void onFractalMouseUp(void) {
    onMouseUp();
}
//---------------------------------------------------------------------------

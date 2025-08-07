#include "complejos.h"
#include "div.h"
#include "stsbar.h"
#include "main.h"
#include "dibuja.h"
#include <math.h>
#include "constantes.h"
#ifndef __cplusplus
	#include <stdbool.h>
#endif
#include <stdio.h> 

int max_iterations = DEFAULT_MAXITERATIONS;  
BYTE Memory[WINDOW_HEIGHT][WINDOW_WIDTH];
int temp_rect;

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
double complex_step_x;
double complex_step_y;
double complex_origin_x;
double complex_origin_y;

const double INITIALFRACTALSIZE = 2.9;

bool isImageLoaded;
char s[255]; // Status bar buffer

void onClearMemory() {
    memset(Memory, 0, WINDOW_HEIGHT * WINDOW_WIDTH * sizeof(BYTE));
}

/**
 * Calculate if a point belongs to the Mandelbrot set
 * @param column Pixel x coordinate
 * @param row Pixel y coordinate  
 * @return 255 if in set, otherwise iteration count when diverged
 */
BYTE calculateMandelbrotPoint(int column, int row)
{
	int iterations;
	Comp c;
	Comp current;
#ifdef useConvergenceThreshold	
	Comp previous;
	Comp difference;
	initComplex(0, 0, previous);
#endif

	// Convert pixel coordinates to complex plane coordinates
	c.x = complex_step_x * column + complex_origin_x;
	c.y = complex_step_y * row + complex_origin_y;

	assign (current, c);
	// Mandelbrot iteration: z(n+1) = z(n)^2 + c, z(0)=c

#ifdef useConvergenceThreshold
	double convergence_diff = 0.0;
	for(iterations = 0; iterations < max_iterations && mdSquared(current) <= ESCAPE_RADIUS_SQUARED && convergence_diff < CONVERGENCE_THRESHOLD; iterations++)
	{
		squareAdd (previous, c, current);
		diff(previous, current, difference);
		assign (previous, current);
		convergence_diff = mdSquared(difference);
	}
#else
	for(iterations = 0; iterations < max_iterations && mdSquared(current) <= ESCAPE_RADIUS_SQUARED; iterations++)
	{
		squareAddAssign(current, c);
	}
#endif

	if (iterations >= max_iterations) return (MANDELBROTPOINT_VALUE);
	return (iterations);
}

#ifdef useUniformBlockOptimization
void fillMemorySquare(int x, int y, int w, int h, BYTE value) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + w > WINDOW_WIDTH - 1) w = WINDOW_WIDTH - 1 - x;
    if (y + h > WINDOW_HEIGHT - 1) h = WINDOW_HEIGHT - 1 - y;
    for (int a = 0; a < h; a++)
        memset(&Memory[y + a][x], value, w);
}

// Only optimize if point is outside the set - optimization fails near the neck
bool areCornersUniformAndCalculated(int a, int b, int c, int d) {
	return (a > 0 && a == b && b == c && c == d && (a != MANDELBROTPOINT_VALUE));
}
#endif

void calculateAndDraw(Point *p){
	BYTE iterations;
	int xc=p->x;
	int yc=p->y;
	int tam=p->tam;

	iterations=Memory[yc][xc];	
	if(iterations==0) {
		iterations=calculateMandelbrotPoint(xc, yc);
		Memory[yc][xc]=iterations;
	}

#ifdef useUniformBlockOptimization
	// For blocks > 1, check if this is bottom-right corner of a uniform block
	if (tam > 1 && xc >= tam && yc >= tam && tam<=BLOCK_OPTIMIZATION_SIZE) {
		BYTE topLeft = Memory[yc-tam][xc-tam];
		BYTE topRight = Memory[yc-tam][xc];
		BYTE bottomLeft = Memory[yc][xc-tam];

		if (tam>1 && areCornersUniformAndCalculated(topLeft, topRight, bottomLeft, iterations)) {
			fillMemorySquare(xc-tam, yc-tam, tam, tam, iterations);    
		}
	}
#endif

	drawSquare(xc,yc,tam,iterations);	
}

/**
 * Progressive fractal rendering using multiple passes with decreasing block sizes
 * Starts with large blocks and refines to individual pixels for smooth user experience
 */
void renderFractalInternal(RenderFractalInternalParams *rp){
	int current_block_size = rp->current_block_size;
	int k= rp->thread;

	Point p;
	for (int i = k; i*current_block_size < WINDOW_WIDTH ; i += DEFAULT_THREAD_COUNT)
	{				
		for (int j = 0; j*current_block_size < WINDOW_HEIGHT ; j += 1)
		{	
			if (current_block_size == 1 && Memory[j][i] != 0) {
        		drawSquare(i, j,1, Memory[j][i]);
    		} else {			
				p.x=i*current_block_size;
				p.y=j*current_block_size;	
				p.tam=current_block_size;		
				calculateAndDraw(&p);
			}
		}
	}	
}

void renderFractal(void)
{
	isImageLoaded = FALSE;
	int initial_block_size = 256;

	// Progressive rendering: 9 passes from 256x256 blocks down to 1x1 pixels
	for (int r = 0; r < 9; r++)
	{
		int current_block_size = initial_block_size >> r; // Bit shift is faster than pow(2,r)
		if (current_block_size < 1) break; 
		if (current_block_size>=global_pixel_size) continue;
		
		for(int k = 0; k < DEFAULT_THREAD_COUNT; k++){
			RenderFractalInternalParams rp;
			rp.current_block_size=current_block_size;
			rp.thread=k;
			threadpool_add(thread_pool, renderFractalInternal, &rp);

#ifdef DEBUG_PAUSE_ITERATIONS
			char debug_msg[100];
			sprintf(debug_msg, "Debug: Iteration %d-%d\nPress OK to continue...", r,k);
			MessageBox(main_window_handle, debug_msg, "Debug Pause", MB_OK);
#endif
		}
	
		threadpool_wait_all(thread_pool);	
		drawFractal(main_window_handle);
		onClearMessageQueue();
	}
	sprintf(s, "Iterations: %d ",  max_iterations);
	updateStatusBar(s, 0, 0);
	isImageLoaded = TRUE;
}

int drawSelectionRectangle(HDC hDC)
{
	HPEN oldpen = NULL;
	HBRUSH oldbr = NULL;
	if (isImageLoaded && isButtonPressed)
	{
		int rs = 0;
		HBRUSH br = NULL;
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
	return (0);
}

// Initialize fractal to default view (full Mandelbrot set)
void onInitializeFractal(void) {
    mouse_down_x = 0;
    mouse_down_y = 0;
    mouse_up_x = WINDOW_WIDTH;
    mouse_up_y = WINDOW_HEIGHT;

    // Set initial complex plane coordinates (standard Mandelbrot view)
    complex_origin_x = -2.2;
    complex_origin_y = -1.5;

    color_offset = 0;
	invertColors=0;
    isColorRotationActive = false;
    absolute_zoom = 1.0;
    max_iterations = DEFAULT_MAXITERATIONS;

    // Calculate step size for converting pixels to complex coordinates
    complex_step_x = ((INITIALFRACTALSIZE) / (double)(WINDOW_WIDTH));
    complex_step_y = ((INITIALFRACTALSIZE) / (double)(WINDOW_HEIGHT));

    global_pixel_size = WINDOW_WIDTH;
    sprintf(s, "Complex Plane: (%.2f,%.2f) : (%.2f,%.2f)", complex_origin_x, complex_origin_y, complex_origin_x + INITIALFRACTALSIZE, complex_origin_y + INITIALFRACTALSIZE);
    updateStatusBar(s, 1, 0);

    onClearMemory();
    fillColors();

    drawSquare(0, 0, WINDOW_WIDTH, 15);
    drawFractal(main_window_handle);
    onClearMessageQueue();
    renderFractal();
}

// Reuses existing calculations to provide immediate visual feedback when zooming
void expandMemory(int startX, int startY, int newWidth, int newHeight, double scaleX, double scaleY) {
    if (startX < 0 || startY < 0 || newWidth <= 0 || newHeight <= 0 ||
        startX >= WINDOW_WIDTH || startY >= WINDOW_HEIGHT ||
        scaleX <= 0.0 || scaleY <= 0.0) {
        return;
    }

    // Check for potential integer overflow in malloc calculation
    if (newWidth > INT_MAX / newHeight || (size_t)newWidth * newHeight > SIZE_MAX / sizeof(BYTE)) {
        return;
    }

    BYTE *oldMemoryToExpand = (BYTE *)malloc((size_t)newHeight * newWidth * sizeof(BYTE));
    if (oldMemoryToExpand == NULL) {
        return;
    }

    // Copy selected region from current memory
    for (int i = 0; i < newHeight && (startY + i) < WINDOW_HEIGHT; i++) {
        int copyWidth = (startX + newWidth <= WINDOW_WIDTH) ? newWidth : (WINDOW_WIDTH - startX);
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
            
            if (posy >= 0 && posy < WINDOW_HEIGHT && posx >= 0 && posx < WINDOW_WIDTH) {
                Memory[posy][posx] = oldMemoryToExpand[i*newWidth+j];
                drawSquare(posx,posy,global_pixel_size,(Memory[posy][posx]));
            }
        }
    }

    free(oldMemoryToExpand);
}

void rescaleView(void)
{
	complex_origin_x += complex_step_x * (double)mouse_down_x;
	complex_origin_y += complex_step_y * (double)mouse_down_y;

	int newWidth = (mouse_up_x - mouse_down_x + 1);
	int newHeight = (mouse_up_y - mouse_down_y + 1);

	double factorX = (double)newWidth / (double)(WINDOW_WIDTH);
	double factorY = (double)newHeight / (double)(WINDOW_HEIGHT);

	global_pixel_size = (int)(1/factorX) + 1;
	
	expandMemory(mouse_down_x, mouse_down_y, newWidth, newHeight, factorX, factorY);
	drawFractal(main_window_handle);
	
	complex_step_x *= factorX;
	complex_step_y *= factorY;

	double zoom_factor = 1.0 / ((factorX + factorY) / 2.0);
	absolute_zoom *= zoom_factor;
	
	// Adaptive max_iterations based on absolute zoom level
	max_iterations = (int)(50 + log2(absolute_zoom) * 50);
	if (max_iterations < 50) max_iterations = 50;
	if (max_iterations > 2000) max_iterations = 2000;
	 
	//sprintf(s, "Zoom: %.1fx, Iterations: %d", zoom_factor, max_iterations);
	//updateStatusBar(s, 0, 0);
	sprintf(s, "Complex Plane: (%.2f,%.2f) : (%.2f,%.2f)", complex_origin_x, complex_origin_y, complex_origin_x + complex_step_x*WINDOW_WIDTH, complex_origin_y + complex_step_y*WINDOW_HEIGHT);
    updateStatusBar(s, 1, 0);
}

void handleMouseMove(int x, int y, HWND hwnd)
{
	HDC hDC = NULL;
	double complexStartX = 0.0, complexStartY = 0.0;
	double complexEndX = 0.0, complexEndY = 0.0;

	if (isImageLoaded && isButtonPressed)
	{
		int rectWidth, rectHeight;
		
		mouse_up_x = x;
		mouse_up_y = y;
		
		// Make selection rectangle square (maintain aspect ratio)
		rectWidth = (mouse_up_x - mouse_down_x);
		rectHeight = (mouse_up_y - mouse_down_y);
		((rectWidth) < (rectHeight)) ? (mouse_up_x = mouse_down_x + rectHeight) : (mouse_up_y = mouse_down_y + rectWidth);

		drawFractal(hwnd);
		hDC = GetDC(hwnd);
		drawSelectionRectangle(hDC);

		wsprintf(s, " Position: %d/%d : %d/%d", mouse_down_x, mouse_down_y, mouse_up_x, mouse_up_y);
		updateStatusBar(s, 0, 0);

		complexStartX = (complex_origin_x + complex_step_x * (double)mouse_down_x);
		complexStartY = (complex_origin_y + complex_step_y * (double)mouse_down_y);

		complexEndX = (complex_origin_x + complex_step_x * (double)mouse_up_x);
		complexEndY = (complex_origin_y + complex_step_y * (double)mouse_up_y);

		sprintf(s, "Complex Plane: (%.2f,%.2f) : (%.2f,%.2f)", complexStartX, complexStartY, complexEndX, complexEndY);
		updateStatusBar(s, 1, 0);

		ReleaseDC(hwnd, hDC);
	}
}

char *generateSaveFilename(void)
{
	double txi = (complex_origin_x) * 100.0;
	double tyi = (complex_origin_y) * 100.0;

	double txf = (complex_step_x * ((double)(WINDOW_WIDTH)) + complex_origin_x) * 100.0;
	double tyf = (complex_step_y * ((double)(WINDOW_HEIGHT)) + complex_origin_y) * 100.0;

	sprintf(s, "Fractal%2.0f_%2.0f_%2.0f_%2.0f.bmp ", txi, tyi, txf, tyf);
	return (s);
}

void onMouseUp(void)
{
	if (isImageLoaded && isButtonPressed)
	{
		isButtonPressed = FALSE;
		rescaleView();
		onRepaint();
		renderFractal();
	}
}

void onMouseDown(int x, int y)
{
	if (isImageLoaded)
	{
		mouse_down_x = x;
		mouse_down_y = y;
		isButtonPressed = TRUE;
	}
}

// Cancel zoom selection and redraw fractal without selection rectangle
void onFractalCancelSelection(void) {
	if (isButtonPressed) {
		isButtonPressed = FALSE;
		onRepaint();

		// Restore current view coordinates in status bar
		sprintf(s, "Complex Plane: (%.2f,%.2f) : (%.2f,%.2f)", 
			complex_origin_x, complex_origin_y, 
			complex_origin_x + complex_step_x*WINDOW_WIDTH, 
			complex_origin_y + complex_step_y*WINDOW_HEIGHT);
		updateStatusBar(s, 1, 0);
	}
}

void animateColorRotation(void)
{
	while ((isColorRotationActive) && (main_window_handle != 0))
	{
		color_offset = color_offset + 1;
		if (color_offset>=COLOR_COUNT) color_offset=0;
		fillColors();
		if (main_window_handle != 0)
			drawFractal(main_window_handle);
		Sleep(50);
		onClearMessageQueue();
	}
}

void onFractalKeyPress(BYTE key) {
    if (key == 't' || key == 'T') {
        isColorRotationActive = !isColorRotationActive;
        animateColorRotation();
    }
    if (key == 'i' || key == 'I') {
        invertColors = !invertColors;
        fillColors();
        if (main_window_handle != 0)
            drawFractal(main_window_handle);
    }
}

void onFractalMouseMove(int X, int Y, HWND hwnd) {
    if (X < 0 || X >= WINDOW_WIDTH || Y < 0 || Y >= WINDOW_HEIGHT) {
        return;
    }
    handleMouseMove(X, Y, hwnd);
}

void onFractalMouseDown(int X, int Y) {
    if (X < 0 || X >= WINDOW_WIDTH || Y < 0 || Y >= WINDOW_HEIGHT) {
        return;
    }
    onMouseDown(X, Y);
}

void onFractalMouseUp(void) {
    onMouseUp();
}
#include "fractal_calc.h"


int max_iterations = DEFAULT_MAXITERATIONS;
BYTE Memory[WINDOW_HEIGHT][WINDOW_WIDTH];
int temp_rect;

bool isColorRotationActive = false;
bool isButtonPressed;
bool isImageLoaded;
bool isAutomaticZoomOn;

int mouse_move_x;
int mouse_move_y;

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
char s[255]; // Status bar buffer


int getOptimalThreadCount(void) {
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	int cores = (int)sysinfo.dwNumberOfProcessors - 2;

	if (cores <= 0) return 4;  // Fallback for detection failure
	return (cores > 16) ? 16 : cores;  // Cap at 16 to avoid thread overhead
}

void onClearMemory() {
	memset(Memory, 0, WINDOW_HEIGHT * WINDOW_WIDTH * sizeof(BYTE));
}

#ifdef useUniformBlockOptimization
static void fillMemorySquare(int x, int y, int w, int h, BYTE value) {
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x + w > WINDOW_WIDTH - 1) w = WINDOW_WIDTH - 1 - x;
	if (y + h > WINDOW_HEIGHT - 1) h = WINDOW_HEIGHT - 1 - y;
	for (int a = 0; a < h; a++)
		memset(&Memory[y + a][x], value, w);
}

static bool isDeepInsideSet(int xc, int yc, int blockSize) {
	int dx[] = { 2, 2, 2, 1, 0 };
	int dy[] = { 0, 1, 2, 2, 2 };
	for (int i = 0; i < 5; i++) {
		int x = xc - blockSize * dx[i];
		int y = yc - blockSize * dy[i];
		if (x < 0 || y < 0 || x >= WINDOW_WIDTH || y >= WINDOW_HEIGHT)
			continue;
		if (Memory[y][x] != MANDELBROTPOINT_VALUE)
			return false;
	}
	return true;
}

static void optimizeWhenCornersEqual(int xc, int yc, int blockSize, BYTE current) {
	if (blockSize > 1 && xc >= blockSize && yc >= blockSize && blockSize <= BLOCK_OPTIMIZATION_SIZE) {
		BYTE topLeft = Memory[yc - blockSize][xc - blockSize];
		BYTE topRight = Memory[yc - blockSize][xc];
		BYTE bottomLeft = Memory[yc][xc - blockSize];

		if (current == 0) return ;
		if (!(topLeft == topRight && topRight == bottomLeft && bottomLeft == current)) return ;

		if (current == MANDELBROTPOINT_VALUE && absolute_zoom == 1.0) return ;
		if (current == MANDELBROTPOINT_VALUE && !isDeepInsideSet(xc, yc, blockSize)) return ;
		fillMemorySquare(xc - blockSize, yc - blockSize, blockSize, blockSize, current);	
	}
}
#endif

/**
 * Calculate if a point belongs to the Mandelbrot set
 * @param column Pixel x coordinate
 * @param row Pixel y coordinate
 * @return 255 if in set, otherwise iteration count when diverged
 */
static BYTE calculateMandelbrotPoint(int column, int row)
{
	int iterations;
	Comp c;
	Comp current;
#ifdef useConvergenceThreshold	
	Comp previous;
	Comp difference;
	initComplex(0, 0, previous);
#endif

	c.x = complex_step_x * column + complex_origin_x;
	c.y = complex_step_y * row + complex_origin_y;

	assign(current, c);

#ifdef useConvergenceThreshold
	double convergence_diff = 0.0;
	for (iterations = 0; iterations < max_iterations && mdSquared(current) <= MANDELBROT_ESCAPE_RADIUS_SQUARED && convergence_diff < CONVERGENCE_THRESHOLD; iterations++)
	{
		squareAdd(previous, c, current);
		diff(previous, current, difference);
		assign(previous, current);
		convergence_diff = mdSquared(difference);
	}
#else
	for (iterations = 0; iterations < max_iterations && mdSquared(current) <= MANDELBROT_ESCAPE_RADIUS_SQUARED; iterations++)
	{
		squareAddAssign(current, c);
	}
#endif

	if (iterations >= max_iterations) return (MANDELBROTPOINT_VALUE);
	return (iterations);
}

static void calculateAndDraw(Point* p) {
	BYTE iterations;
	int xc = p->x;
	int yc = p->y;
	int blockSize = p->blockSize;

	iterations = Memory[yc][xc];
	if (iterations == 0) {
		iterations = calculateMandelbrotPoint(xc, yc);
		Memory[yc][xc] = iterations;
	}

#ifdef useUniformBlockOptimization
	optimizeWhenCornersEqual(xc, yc, blockSize, iterations);
#endif

	drawSquare(xc, yc, blockSize, iterations);
}


/**
 * Progressive fractal rendering using multiple passes with decreasing block sizes
 * Starts with large blocks and refines to individual pixels for smooth user experience
 */
static void renderFractalInternal(RenderFractalInternalParams* rp) {
	int pcurrent_block_size = rp->current_block_size;
	int threadId = rp->threadId;

	Point p;
	int threadCount = getOptimalThreadCount();
	for (int i = threadId; i * pcurrent_block_size < WINDOW_WIDTH; i += threadCount)
	{
		for (int j = 0; j * pcurrent_block_size < WINDOW_HEIGHT; j += 1)
		{
			if (pcurrent_block_size == 1 && Memory[j][i] != 0) {
				drawSquare(i, j, 1, Memory[j][i]);
			}
			else {
				p.x = i * pcurrent_block_size;
				p.y = j * pcurrent_block_size;
				p.blockSize = pcurrent_block_size;
				calculateAndDraw(&p);
			}
		}
	}
}

static int drawSelectionRectangle(HDC hDC)
{
	if (isImageLoaded && isButtonPressed)
	{
		HPEN pen1 = CreatePen(PS_DASH, 1, RGB(50, 255, 50));
		HBRUSH br = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
		HPEN oldpen = (HPEN)SelectObject(hDC, pen1);
		HBRUSH oldbr = (HBRUSH)SelectObject(hDC, br);
		int rs = Rectangle(hDC, mouse_down_x, mouse_down_y, mouse_up_x, mouse_up_y);
		SelectObject(hDC, oldpen);
		SelectObject(hDC, oldbr);
		DeleteObject(pen1);
		return rs;
	}
	return 0;
}


static void expandMemory(int startX, int startY, int newWidth, int newHeight, double scaleX, double scaleY) {
	if (startX < 0 || startY < 0 || newWidth <= 0 || newHeight <= 0 ||
		startX >= WINDOW_WIDTH || startY >= WINDOW_HEIGHT ||
		scaleX <= 0.0 || scaleY <= 0.0) {
		return;
	}

	if (newWidth > INT_MAX / newHeight || (size_t)newWidth * newHeight > SIZE_MAX / sizeof(BYTE)) {
		return;
	}

	BYTE* oldMemoryToExpand = (BYTE*)malloc((size_t)newHeight * newWidth * sizeof(BYTE));
	if (oldMemoryToExpand == NULL) {
		return;
	}

	for (int i = 0; i < newHeight && (startY + i) < WINDOW_HEIGHT; i++) {
		int copyWidth = (startX + newWidth <= WINDOW_WIDTH) ? newWidth : (WINDOW_WIDTH - startX);
		if (copyWidth > 0) {
			memcpy(&oldMemoryToExpand[i * newWidth], &Memory[startY + i][startX], copyWidth);
		}
	}
	onClearMemory();

	for (int i = 0; i < newHeight; i++) {
		for (int j = 0; j < newWidth; j++) {
			int posy = (int)(i / scaleY);
			int posx = (int)(j / scaleX);

			if (posy >= 0 && posy < WINDOW_HEIGHT && posx >= 0 && posx < WINDOW_WIDTH) {
				Memory[posy][posx] = oldMemoryToExpand[i * newWidth + j];
				drawSquare(posx, posy, global_pixel_size, (Memory[posy][posx]));
			}
		}
	}
	drawFractalBitmap(main_window_handle);
	free(oldMemoryToExpand);
}

static void rescaleView(int pmouse_down_x, int pmouse_down_y, int pmouse_up_x, int pmouse_up_y)
{
	complex_origin_x += complex_step_x * (double)pmouse_down_x;
	complex_origin_y += complex_step_y * (double)pmouse_down_y;

	int newWidth = (pmouse_up_x - pmouse_down_x + 1);
	int newHeight = (pmouse_up_y - pmouse_down_y + 1);

	double factorX = (double)newWidth / (double)(WINDOW_WIDTH);
	double factorY = (double)newHeight / (double)(WINDOW_HEIGHT);

	global_pixel_size = (int)(1 / factorX) + 1;

	if (isAutomaticZoomOn) {
		onClearMemory();
	}
	else {
		expandMemory(pmouse_down_x, pmouse_down_y, newWidth, newHeight, factorX, factorY);
	}

	complex_step_x *= factorX;
	complex_step_y *= factorY;

	double zoom_factor = 1.0 / ((factorX + factorY) / 2.0);
	absolute_zoom *= zoom_factor;

	max_iterations = (int)(50 + log2(absolute_zoom) * 50);
	if (max_iterations < 50) max_iterations = 50;
	if (max_iterations > 2000) max_iterations = 2000;

	if (complex_step_x < 1e-6) {
		sprintf_s(s, sizeof(s), "Complex Plane: (%.8f,%.8f) : (%.8f,%.8f)", complex_origin_x, complex_origin_y, complex_origin_x + complex_step_x * WINDOW_WIDTH, complex_origin_y + complex_step_y * WINDOW_HEIGHT);
	}
	else {
		sprintf_s(s, sizeof(s), "Complex Plane: (%.2f,%.2f) : (%.2f,%.2f)", complex_origin_x, complex_origin_y, complex_origin_x + complex_step_x * WINDOW_WIDTH, complex_origin_y + complex_step_y * WINDOW_HEIGHT);
	}
	updateStatusBar(s, 1, 0);
}


static void animateColorRotation(void)
{
	while ((isColorRotationActive) && (main_window_handle != 0))
	{
		color_offset = color_offset + 1;
		if (color_offset >= COLOR_COUNT) color_offset = 0;
		fillColors();
		if (main_window_handle != 0)
			drawFractalBitmap(main_window_handle);
		Sleep(50);
		onClearMessageQueue();
	}
}

static void renderFractal(void)
{
	isImageLoaded = FALSE;
	int initial_block_size = 256;

	// Progressive rendering: 9 passes from 256x256 blocks down to 1x1 pixels
	for (int r = 0; r < 9; r++)
	{
		int current_block_size = initial_block_size >> r; // Bit shift is faster than pow(2,r)
		if (current_block_size < 1) break;
		if (current_block_size >= global_pixel_size) continue;

		int threadCount = getOptimalThreadCount();
		for (int k = 0; k < threadCount; k++) {
			RenderFractalInternalParams rp;
			rp.current_block_size = current_block_size;
			rp.threadId = k;

			threadpool_add(thread_pool, renderFractalInternal, &rp);

		}

		threadpool_wait_all(thread_pool);
		drawFractalBitmap(main_window_handle);
		onClearMessageQueue();

#ifdef DEBUG_PAUSE_ITERATIONS
		char s[100];
		sprintf_s(s, sizeof(s), "Debug: Iteration %d\nPress OK to continue...", r);
		MessageBox(main_window_handle, s, "Debug Pause", MB_OK);
#endif


	}
	sprintf_s(s, sizeof(s), "Mouse (%d,%d) Iterations: %d", mouse_move_x, mouse_move_y, max_iterations);
	updateStatusBar(s, 0, 0);
	isImageLoaded = TRUE;

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
	invertColors = 0;
	isColorRotationActive = false;
	absolute_zoom = 1.0;
	max_iterations = DEFAULT_MAXITERATIONS;

	// Calculate step size for converting pixels to complex coordinates
	complex_step_x = ((INITIALFRACTALSIZE) / (double)(WINDOW_WIDTH));
	complex_step_y = ((INITIALFRACTALSIZE) / (double)(WINDOW_HEIGHT));

	global_pixel_size = WINDOW_WIDTH;
	sprintf_s(s, sizeof(s), "Complex Plane: (%.2f,%.2f) : (%.2f,%.2f)", complex_origin_x, complex_origin_y, complex_origin_x + INITIALFRACTALSIZE, complex_origin_y + INITIALFRACTALSIZE);
	updateStatusBar(s, 1, 0);

	onClearMemory();
	fillColors();

	drawSquare(0, 0, WINDOW_WIDTH, 15);
	drawFractalBitmap(main_window_handle);
	onClearMessageQueue();
	renderFractal();
}





static void handleMouseMove(int x, int y, HWND hwnd)
{
	HDC hDC = NULL;
	double complexStartX = 0.0, complexStartY = 0.0;
	double complexEndX = 0.0, complexEndY = 0.0;
	mouse_move_x = x;
	mouse_move_y = y;

	if (isImageLoaded) {
		sprintf_s(s, sizeof(s), "Mouse (%d,%d) Iterations: %d", mouse_move_x, mouse_move_y, max_iterations);
	}
	else {
		sprintf_s(s, sizeof(s), "Mouse (%d,%d) Iterations: %d - RENDERING...", mouse_move_x, mouse_move_y, max_iterations);
	}
	updateStatusBar(s, 0, 0);

	if (isImageLoaded && isButtonPressed)
	{
		int rectWidth, rectHeight;

		mouse_up_x = x;
		mouse_up_y = y;

		// Make selection rectangle square (maintain aspect ratio)
		rectWidth = (mouse_up_x - mouse_down_x);
		rectHeight = (mouse_up_y - mouse_down_y);
		((rectWidth) < (rectHeight)) ? (mouse_up_x = mouse_down_x + rectHeight) : (mouse_up_y = mouse_down_y + rectWidth);

		drawFractalBitmap(hwnd);
		hDC = GetDC(hwnd);
		drawSelectionRectangle(hDC);


		complexStartX = (complex_origin_x + complex_step_x * (double)mouse_down_x);
		complexStartY = (complex_origin_y + complex_step_y * (double)mouse_down_y);

		complexEndX = (complex_origin_x + complex_step_x * (double)mouse_up_x);
		complexEndY = (complex_origin_y + complex_step_y * (double)mouse_up_y);

		if (complex_step_x < 1e-6) {
			sprintf_s(s, sizeof(s), "Complex Plane: (%.8f,%.8f) : (%.8f,%.8f)", complexStartX, complexStartY, complexEndX, complexEndY);
		}
		else {
			sprintf_s(s, sizeof(s), "Complex Plane: (%.2f,%.2f) : (%.2f,%.2f)", complexStartX, complexStartY, complexEndX, complexEndY);
		}
		updateStatusBar(s, 1, 0);

		ReleaseDC(hwnd, hDC);
	}
}

char* generateSaveFilename(void)
{
	double txi = (complex_origin_x) * 100.0;
	double tyi = (complex_origin_y) * 100.0;

	double txf = (complex_step_x * ((double)(WINDOW_WIDTH)) + complex_origin_x) * 100.0;
	double tyf = (complex_step_y * ((double)(WINDOW_HEIGHT)) + complex_origin_y) * 100.0;

	sprintf_s(s, sizeof(s), "Fractal%2.0f_%2.0f_%2.0f_%2.0f.bmp ", txi, tyi, txf, tyf);
	return (s);
}

static void automaticZoom(void) {

	int reduceddWith = (WINDOW_WIDTH)-AUTOMATIC_ZOOM_PIXELS * 2;
	int reducedHeight = (WINDOW_HEIGHT)-AUTOMATIC_ZOOM_PIXELS * 2;
	int pmouse_move_x, pmouse_move_y;

	while (isAutomaticZoomOn) {

		pmouse_move_x = ((double)mouse_move_x / (double)WINDOW_WIDTH) * (AUTOMATIC_ZOOM_PIXELS * 2);
		pmouse_move_y = ((double)mouse_move_y / (double)WINDOW_HEIGHT) * (AUTOMATIC_ZOOM_PIXELS * 2);

		rescaleView(pmouse_move_x, pmouse_move_y, pmouse_move_x + reduceddWith, pmouse_move_y + reducedHeight);
		onRepaint();
		renderFractal();

		onClearMessageQueue();

		// check if the windows exists
		if (main_window_handle == 0) {
			isAutomaticZoomOn = FALSE;
			break;
		}
	}
}



// Cancel zoom selection and redraw fractal without selection rectangle
void onFractalCancelSelection(void) {
	if (isButtonPressed) {
		isButtonPressed = FALSE;
		onRepaint();

		// Restore current view coordinates in status bar
		sprintf_s(s, sizeof(s), "Complex Plane: (%.2f,%.2f) : (%.2f,%.2f)",
			complex_origin_x, complex_origin_y,
			complex_origin_x + complex_step_x * WINDOW_WIDTH,
			complex_origin_y + complex_step_y * WINDOW_HEIGHT);
		updateStatusBar(s, 1, 0);
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
			drawFractalBitmap(main_window_handle);
	}
	if (key == 'z' || key == 'Z') {
		isAutomaticZoomOn = !isAutomaticZoomOn;
		automaticZoom();
	}
}

void onFractalMouseMove(int X, int Y, HWND hwnd) {
	if (X < 0 || X >= WINDOW_WIDTH || Y < 0 || Y >= WINDOW_HEIGHT) {
		return;
	}
	handleMouseMove(X, Y, hwnd);
}

void onFractalMouseDown(int x, int y) {
	if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
		return;
	}
	if (isImageLoaded)
	{
		mouse_down_x = x;
		mouse_down_y = y;
		isButtonPressed = TRUE;
	}
}

void onFractalMouseUp(void) {
		if (isImageLoaded && isButtonPressed)
	{
		isButtonPressed = FALSE;
		rescaleView(mouse_down_x, mouse_down_y, mouse_up_x, mouse_up_y);
		onRepaint();
		renderFractal();
	}
}
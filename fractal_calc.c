#include "fractal_calc.h"

int max_iterations = DEFAULT_MAXITERATIONS;
uint32_t Memory[WINDOW_HEIGHT][WINDOW_WIDTH];

bool invertColors = false;
bool juliaMode = false;

bool isColorRotationActive = false;
bool isButtonPressed = false;
bool isImageLoaded = false;
bool isAutomaticZoomOn = false;

int mouse_move_x;
int mouse_move_y;

int mouse_down_x;
int mouse_down_y;
int mouse_up_x;
int mouse_up_y;

int color_offset = 0;
int global_pixel_size;

double current_zoom = 1.0;
double absolute_zoom = 1.0; // Track total zoom from initial state
double complex_step_x;
double complex_step_y;
double complex_origin_x;
double complex_origin_y;

const double INITIALFRACTALSIZE = 2.9;
char s[255]; // Status bar buffer

int getOptimalThreadCount(void)
{
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  int cores = (int)sysinfo.dwNumberOfProcessors;

  if (cores <= 0)
    return 1;					  // Fallback for detection failure
  return (cores > 16) ? 16 : cores; // Cap at 16 to avoid thread overhead
}

void onClearMemory()
{
  // All bytes 0xFF means every uint32_t entry is ITERATION_UNKNOWN.
  memset(Memory, 0xFF, sizeof(Memory));
}

#ifdef useUniformBlockOptimization
static void fillMemorySquare(int x, int y, int w, int h, uint32_t value)
{
  if (x < 0)
    x = 0;
  if (y < 0)
    y = 0;
  if (x >= WINDOW_WIDTH || y >= WINDOW_HEIGHT)
    return;
  if (x + w > WINDOW_WIDTH)
    w = WINDOW_WIDTH - x;
  if (y + h > WINDOW_HEIGHT)
    h = WINDOW_HEIGHT - y;
  for (int a = 0; a < h; a++) {
    uint32_t *row = &Memory[y + a][x];
    for (int b = 0; b < w; b++)
      row[b] = value;
  }
}

static bool isDeepInsideSet(int xc, int yc, int blockSize)
{
  int dx[] = { 2, 2, 2, 1, 0 };
  int dy[] = { 0, 1, 2, 2, 2 };
  for (int i = 0; i < 5; i++)
  {
    int x = xc - blockSize * dx[i];
    int y = yc - blockSize * dy[i];
    if (x < 0 || y < 0 || x >= WINDOW_WIDTH || y >= WINDOW_HEIGHT)
      continue;
    if (Memory[y][x] != MANDELBROTPOINT_VALUE)
      return false;
  }
  return true;
}

static void optimizeWhenCornersEqual(int xc, int yc, int blockSize, uint32_t current)
{
  if (blockSize > 1 && xc >= blockSize && yc >= blockSize && blockSize <= BLOCK_OPTIMIZATION_SIZE)
  {
    uint32_t topLeft = Memory[yc - blockSize][xc - blockSize];
    uint32_t topRight = Memory[yc - blockSize][xc];
    uint32_t bottomLeft = Memory[yc][xc - blockSize];

    if (current == ITERATION_UNKNOWN)
      return;
    if (!(topLeft == topRight && topRight == bottomLeft && bottomLeft == current))
      return;

    if (current == MANDELBROTPOINT_VALUE && absolute_zoom == 1.0)
      return;
    if (current == MANDELBROTPOINT_VALUE && !isDeepInsideSet(xc, yc, blockSize))
      return;
    fillMemorySquare(xc - blockSize, yc - blockSize, blockSize, blockSize, current);
  }
}
#endif

/**
 * Calculate if a point belongs to the Mandelbrot set
 * @param column Pixel x coordinate
 * @param row Pixel y coordinate
 * @param j Julia Comp * , calculates julia set.
 * @return MANDELBROTPOINT_VALUE if in set, otherwise iteration count when diverged
 */
static uint32_t calculateMandelbrotPoint(int column, int row, Comp* j)
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

    if (j != NULL) {
      Comp tempComp = *j;
      squareAddAssign(current, tempComp);
    }
    else {
      squareAddAssign(current, c);
    }
  }
#endif

  if (iterations >= max_iterations)
    return (MANDELBROTPOINT_VALUE);
  return (iterations);
}

static void calculateAndDraw(Point* p)
{
  uint32_t iterations;
  int xc = p->x;
  int yc = p->y;
  int blockSize = p->blockSize;

  iterations = Memory[yc][xc];
  if (iterations == ITERATION_UNKNOWN)
  {

    if (juliaMode) {
      Comp julia;
      julia.x = complex_origin_x+ complex_step_x * (double)mouse_move_x;
      julia.y = complex_origin_y + complex_step_y * (double)mouse_move_y;
      iterations = calculateMandelbrotPoint(xc, yc, &julia);
    }
    else {
      iterations = calculateMandelbrotPoint(xc, yc, NULL);
    }

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
static void renderFractalInternal(RenderFractalInternalParams* rp)
{
  int pcurrent_block_size = rp->current_block_size;
  int threadId = rp->threadId;

  Point p;
  int threadCount = getOptimalThreadCount();
  for (int i = threadId; i * pcurrent_block_size < WINDOW_WIDTH; i += threadCount)
  {
    for (int j = 0; j * pcurrent_block_size < WINDOW_HEIGHT; j += 1)
    {
      if (pcurrent_block_size == 1 && Memory[j][i] != ITERATION_UNKNOWN)
      {
        drawSquare(i, j, 1, Memory[j][i]);
      }
      else
      {
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
  if (hDC == NULL || !isImageLoaded || !isButtonPressed)
    return 0;

  int x0 = (mouse_down_x < mouse_up_x) ? mouse_down_x : mouse_up_x;
  int y0 = (mouse_down_y < mouse_up_y) ? mouse_down_y : mouse_up_y;
  int x1 = (mouse_down_x > mouse_up_x) ? mouse_down_x : mouse_up_x;
  int y1 = (mouse_down_y > mouse_up_y) ? mouse_down_y : mouse_up_y;

  HPEN pen1 = CreatePen(PS_DASH, 1, RGB(50, 255, 50));
  HBRUSH br = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
  HPEN oldpen = (HPEN)SelectObject(hDC, pen1);
  HBRUSH oldbr = (HBRUSH)SelectObject(hDC, br);
  int rs = Rectangle(hDC, x0, y0, x1, y1);
  SelectObject(hDC, oldpen);
  SelectObject(hDC, oldbr);
  DeleteObject(pen1);
  return rs;
}

static void expandMemory(int startX, int startY, int newWidth, int newHeight, double scaleX, double scaleY)
{
  if (startX < 0 || startY < 0 || newWidth <= 0 || newHeight <= 0 ||
    startX >= WINDOW_WIDTH || startY >= WINDOW_HEIGHT ||
    scaleX <= 0.0 || scaleY <= 0.0)
  {
    return;
  }

  if (startX + newWidth > WINDOW_WIDTH)
    newWidth = WINDOW_WIDTH - startX;
  if (startY + newHeight > WINDOW_HEIGHT)
    newHeight = WINDOW_HEIGHT - startY;
  if (newWidth <= 0 || newHeight <= 0)
    return;

  size_t count = (size_t)newWidth * (size_t)newHeight;
  if (count > SIZE_MAX / sizeof(uint32_t))
    return;

  uint32_t* oldMemoryToExpand = (uint32_t*)malloc(count * sizeof(uint32_t));
  if (oldMemoryToExpand == NULL)
    return;

  // Start from "unknown" so any uncovered position is recalculated later.
  memset(oldMemoryToExpand, 0xFF, count * sizeof(uint32_t));

  for (int i = 0; i < newHeight; i++)
  {
    memcpy(&oldMemoryToExpand[(size_t)i * newWidth],
           &Memory[startY + i][startX],
           (size_t)newWidth * sizeof(uint32_t));
  }
  onClearMemory();

  for (int i = 0; i < newHeight; i++)
  {
    for (int j = 0; j < newWidth; j++)
    {
      uint32_t value = oldMemoryToExpand[(size_t)i * newWidth + j];
      if (value == ITERATION_UNKNOWN)
        continue;

      int posy = (int)((double)i / scaleY);
      int posx = (int)((double)j / scaleX);

      if (posy >= 0 && posy < WINDOW_HEIGHT && posx >= 0 && posx < WINDOW_WIDTH)
      {
        Memory[posy][posx] = value;
        drawSquare(posx, posy, global_pixel_size, value);
      }
    }
  }
  drawFractalBitmap(main_window_handle);
  free(oldMemoryToExpand);
}

// A cached "inside the set" value is only valid for the max_iterations used to
// compute it. If the limit grows, those pixels must be recomputed.
static void invalidateSetPoints(void)
{
  for (int y = 0; y < WINDOW_HEIGHT; y++)
  {
    for (int x = 0; x < WINDOW_WIDTH; x++)
    {
      if (Memory[y][x] == MANDELBROTPOINT_VALUE)
        Memory[y][x] = ITERATION_UNKNOWN;
    }
  }
}

static bool rescaleView(int px0, int py0, int px1, int py1)
{
  int x0 = (px0 < px1) ? px0 : px1;
  int y0 = (py0 < py1) ? py0 : py1;
  int x1 = (px0 > px1) ? px0 : px1;
  int y1 = (py0 > py1) ? py0 : py1;

  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 >= WINDOW_WIDTH) x1 = WINDOW_WIDTH - 1;
  if (y1 >= WINDOW_HEIGHT) y1 = WINDOW_HEIGHT - 1;

  int newWidth = (x1 - x0 + 1);
  int newHeight = (y1 - y0 + 1);

  if (newWidth < MIN_ZOOM_SELECTION || newHeight < MIN_ZOOM_SELECTION)
    return false;

  double factorX = (double)newWidth / (double)(WINDOW_WIDTH);
  double factorY = (double)newHeight / (double)(WINDOW_HEIGHT);

  if (factorX <= 0.0 || factorY <= 0.0 || factorX > 1.0 || factorY > 1.0)
    return false;

  complex_origin_x += complex_step_x * (double)x0;
  complex_origin_y += complex_step_y * (double)y0;

  global_pixel_size = (int)(1.0 / factorX) + 1;
  if (global_pixel_size < 1)
    global_pixel_size = 1;

  if (isAutomaticZoomOn)
  {
    onClearMemory();
  }
  else
  {
    expandMemory(x0, y0, newWidth, newHeight, factorX, factorY);
  }

  complex_step_x *= factorX;
  complex_step_y *= factorY;

  double zoom_factor = 1.0 / ((factorX + factorY) / 2.0);
  absolute_zoom *= zoom_factor;

  int old_max = max_iterations;
  int new_max = old_max;
  if (absolute_zoom > 0.0 && absolute_zoom < 1e300)
  {
    double computed = 50.0 + log2(absolute_zoom) * 50.0;
    if (computed < 50.0)
      computed = 50.0;
    if (computed > (double)MAX_ITERATIONS_LIMIT)
      computed = (double)MAX_ITERATIONS_LIMIT;
    new_max = (int)computed;
  }
  max_iterations = new_max;

  if (new_max > old_max)
    invalidateSetPoints();

  if (complex_step_x < 1e-6)
  {
    sprintf_s(s, sizeof(s), "Complex Plane: (%.8f,%.8f) : (%.8f,%.8f)", complex_origin_x, complex_origin_y, complex_origin_x + complex_step_x * WINDOW_WIDTH, complex_origin_y + complex_step_y * WINDOW_HEIGHT);
  }
  else
  {
    sprintf_s(s, sizeof(s), "Complex Plane: (%.2f,%.2f) : (%.2f,%.2f)", complex_origin_x, complex_origin_y, complex_origin_x + complex_step_x * WINDOW_WIDTH, complex_origin_y + complex_step_y * WINDOW_HEIGHT);
  }
  updateStatusBar(s, 1, 0);
  return true;
}

static void animateColorRotation(void)
{
  while ((isColorRotationActive) && (main_window_handle != 0))
  {
    color_offset = color_offset + 1;
    if (color_offset >= COLOR_COUNT)
      color_offset = 0;
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
  for (int r = juliaMode ? 8 : 0; r < 9; r++)
  {
    int current_block_size = initial_block_size >> r; // Bit shift is faster than pow(2,r)
    if (current_block_size < 1)
      break;
    if (current_block_size >= global_pixel_size)
      continue;

    int threadCount = getOptimalThreadCount();
    for (int k = 0; k < threadCount; k++)
    {
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
  sprintf_s(s, sizeof(s), "Mouse (%d,%d) Max iterations: %d", mouse_move_x, mouse_move_y, max_iterations);
  updateStatusBar(s, 0, 0);
  isImageLoaded = TRUE;
}

static void animateJulia(void)
{
  while ((juliaMode) && (main_window_handle != 0))
  {

    if (main_window_handle != 0) {
      onClearMemory();
      onRepaint();
      renderFractal();
    }
    Sleep(100);
    onClearMessageQueue();
  }
}

// Initialize fractal to default view (full Mandelbrot set)
void onInitializeFractal(void)
{
  // Stop any animation/interaction mode before resetting the view.
  juliaMode = false;
  isAutomaticZoomOn = false;
  isButtonPressed = false;
  isImageLoaded = false;
  isColorRotationActive = false;
  ReleaseCapture();

  mouse_down_x = 0;
  mouse_down_y = 0;
  mouse_up_x = WINDOW_WIDTH - 1;
  mouse_up_y = WINDOW_HEIGHT - 1;
  mouse_move_x = WINDOW_WIDTH / 2;
  mouse_move_y = WINDOW_HEIGHT / 2;

  // Set initial complex plane coordinates (standard Mandelbrot view)
  complex_origin_x = -2.2;
  complex_origin_y = -1.5;

  color_offset = 0;
  invertColors = false;
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

  if (isImageLoaded)
  {
    sprintf_s(s, sizeof(s), "Mouse (%d,%d) Max iterations: %d", mouse_move_x, mouse_move_y, max_iterations);
  }
  else
  {
    sprintf_s(s, sizeof(s), "Mouse (%d,%d) Max iterations: %d - RENDERING...", mouse_move_x, mouse_move_y, max_iterations);
  }
  updateStatusBar(s, 0, 0);


  mouse_up_x = x;
  mouse_up_y = y;

  if (juliaMode) {
    complexEndX = (complex_origin_x + complex_step_x * (double)mouse_up_x);
    complexEndY = (complex_origin_y + complex_step_y * (double)mouse_up_y);
    sprintf_s(s, sizeof(s), "Complex Plane: (%.2f,%.2f)", complexEndX, complexEndY);
    updateStatusBar(s, 1, 0);
  }

  if (isImageLoaded && isButtonPressed)
  {
    int dx = x - mouse_down_x;
    int dy = y - mouse_down_y;
    int side = (dx < 0) ? -dx : dx;
    int dyAbs = (dy < 0) ? -dy : dy;
    if (dyAbs > side)
      side = dyAbs;
    if (side < 1)
      side = 1;

    // Keep the selection square and anchored at the mouse-down point.
    int upx = mouse_down_x + ((dx < 0) ? -side : side);
    int upy = mouse_down_y + ((dy < 0) ? -side : side);

    // Never let the selection leave the window.
    if (upx < 0) upx = 0;
    if (upx >= WINDOW_WIDTH) upx = WINDOW_WIDTH - 1;
    if (upy < 0) upy = 0;
    if (upy >= WINDOW_HEIGHT) upy = WINDOW_HEIGHT - 1;

    mouse_up_x = upx;
    mouse_up_y = upy;

    drawFractalBitmap(hwnd);
    hDC = GetDC(hwnd);
    drawSelectionRectangle(hDC);

    int x0 = (mouse_down_x < mouse_up_x) ? mouse_down_x : mouse_up_x;
    int y0 = (mouse_down_y < mouse_up_y) ? mouse_down_y : mouse_up_y;
    int x1 = (mouse_down_x > mouse_up_x) ? mouse_down_x : mouse_up_x;
    int y1 = (mouse_down_y > mouse_up_y) ? mouse_down_y : mouse_up_y;

    complexStartX = (complex_origin_x + complex_step_x * (double)x0);
    complexStartY = (complex_origin_y + complex_step_y * (double)y0);

    complexEndX = (complex_origin_x + complex_step_x * (double)(x1 + 1));
    complexEndY = (complex_origin_y + complex_step_y * (double)(y1 + 1));

    if (complex_step_x < 1e-6)
    {
      sprintf_s(s, sizeof(s), "Complex Plane: (%.8f,%.8f) : (%.8f,%.8f)", complexStartX, complexStartY, complexEndX, complexEndY);
    }
    else
    {
      sprintf_s(s, sizeof(s), "Complex Plane: (%.2f,%.2f) : (%.2f,%.2f)", complexStartX, complexStartY, complexEndX, complexEndY);
    }
    updateStatusBar(s, 1, 0);

    if (hDC != NULL)
      ReleaseDC(hwnd, hDC);
  }

}

char* generateSaveFilename(void)
{
  double txi = (complex_origin_x) * 100.0;
  double tyi = (complex_origin_y) * 100.0;

  double txf = (complex_step_x * ((double)(WINDOW_WIDTH)) + complex_origin_x) * 100.0;
  double tyf = (complex_step_y * ((double)(WINDOW_HEIGHT)) + complex_origin_y) * 100.0;

  sprintf_s(s, sizeof(s), "Fractal%2.0f_%2.0f_%2.0f_%2.0f.bmp", txi, tyi, txf, tyf);
  return (s);
}

static void automaticZoom(void)
{

  int reduceddWith = (WINDOW_WIDTH)-AUTOMATIC_ZOOM_PIXELS * 2;
  int reducedHeight = (WINDOW_HEIGHT)-AUTOMATIC_ZOOM_PIXELS * 2;
  int pmouse_move_x, pmouse_move_y;

  while (isAutomaticZoomOn)
  {

    pmouse_move_x = mouse_move_x * (AUTOMATIC_ZOOM_PIXELS * 2) / WINDOW_WIDTH;
    pmouse_move_y = mouse_move_y * (AUTOMATIC_ZOOM_PIXELS * 2) / WINDOW_HEIGHT;

    if (rescaleView(pmouse_move_x, pmouse_move_y, pmouse_move_x + reduceddWith, pmouse_move_y + reducedHeight))
    {
      onRepaint();
      renderFractal();
    }

    onClearMessageQueue();

    // check if the windows exists
    if (main_window_handle == 0)
    {
      isAutomaticZoomOn = FALSE;
      break;
    }
  }
}

// Cancel zoom selection and redraw fractal without selection rectangle
void onFractalCancelSelection(void)
{
  if (!isButtonPressed)
    return;

  isButtonPressed = FALSE;
  if (main_window_handle == NULL)
    return;

  ReleaseCapture();
  onRepaint();

  // Restore current view coordinates in status bar
  sprintf_s(s, sizeof(s), "Complex Plane: (%.2f,%.2f) : (%.2f,%.2f)",
    complex_origin_x, complex_origin_y,
    complex_origin_x + complex_step_x * WINDOW_WIDTH,
    complex_origin_y + complex_step_y * WINDOW_HEIGHT);
  updateStatusBar(s, 1, 0);
}

void onFractalKeyPress(BYTE key)
{
  if (key == 't' || key == 'T')
  {
    isColorRotationActive = !isColorRotationActive;
    animateColorRotation();
  }
  if (key == 'i' || key == 'I')
  {
    invertColors = !invertColors;
    fillColors();
    if (main_window_handle != 0)
      drawFractalBitmap(main_window_handle);
  }
  if (key == 'z' || key == 'Z')
  {
    isAutomaticZoomOn = !isAutomaticZoomOn;
    automaticZoom();
  }
  if (key == 'j' || key == 'J')
  {
    juliaMode = !juliaMode;
    if (juliaMode)
      animateJulia();
    else
    {
      onClearMemory();
      onRepaint();
      renderFractal();
    }
  }
}

void onFractalMouseMove(int X, int Y, HWND hwnd)
{
  if (X < 0 || X >= WINDOW_WIDTH || Y < 0 || Y >= WINDOW_HEIGHT)
  {
    return;
  }
  handleMouseMove(X, Y, hwnd);
}

void onFractalMouseDown(int x, int y)
{
  if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT)
  {
    return;
  }
  if (isImageLoaded && !juliaMode)
  {
    mouse_down_x = x;
    mouse_down_y = y;
    mouse_up_x = x;
    mouse_up_y = y;
    isButtonPressed = TRUE;
    SetCapture(main_window_handle);
  }
}

void onFractalMouseUp(void)
{
  if (!isImageLoaded || !isButtonPressed)
    return;

  int x0 = (mouse_down_x < mouse_up_x) ? mouse_down_x : mouse_up_x;
  int y0 = (mouse_down_y < mouse_up_y) ? mouse_down_y : mouse_up_y;
  int x1 = (mouse_down_x > mouse_up_x) ? mouse_down_x : mouse_up_x;
  int y1 = (mouse_down_y > mouse_up_y) ? mouse_down_y : mouse_up_y;

  // A click (or a tiny drag) must not zoom to a 1-pixel region.
  if ((x1 - x0 + 1) < MIN_ZOOM_SELECTION || (y1 - y0 + 1) < MIN_ZOOM_SELECTION)
  {
    onFractalCancelSelection();
    return;
  }

  isButtonPressed = FALSE;
  ReleaseCapture();

  if (rescaleView(x0, y0, x1, y1))
  {
    onRepaint();
    renderFractal();
  }
  else
  {
    onRepaint();
  }
}
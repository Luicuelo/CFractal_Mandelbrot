#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define AUTOMATIC_ZOOM_PIXELS 40

#define COLOR_DEPTH 8

// Iteration storage is 32-bit so it never collides with the palette size.
// ITERATION_UNKNOWN marks pixels that have not been calculated yet.
#define ITERATION_UNKNOWN 0xFFFFFFFFu
#define MANDELBROTPOINT_VALUE 0xFFFFFFFEu

#define COLOR_COUNT 256
#define DEFAULT_MAXITERATIONS 50
#define MAX_ITERATIONS_LIMIT 2000
#define MIN_ZOOM_SELECTION 8

#define DEFAULT_QUEUE_SIZE 100

//#define DEBUG_PAUSE_ITERATIONS // Debug symbol for conditional compilation
#define useUniformBlockOptimization // Uncomment to enable uniform block optimization for faster rendering
#ifdef useUniformBlockOptimization
  #define BLOCK_OPTIMIZATION_SIZE 8
#endif
//#define useConvergenceThreshold // Uncomment to enable convergence checking, this should accelerate the calculations
#ifdef useConvergenceThreshold
  #define CONVERGENCE_THRESHOLD 12.0
#endif

#endif // CONSTANTS_H

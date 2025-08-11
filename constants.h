#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define AUTOMATIC_ZOOM_PIXELS 40

#define COLOR_DEPTH 8

#define MANDELBROTPOINT_VALUE 255
#define COLOR_COUNT 255
#define DEFAULT_MAXITERATIONS 50

#define DEFAULT_QUEUE_SIZE 100

//#define DEBUG_PAUSE_ITERATIONS // Debug symbol for conditional compilation
#define useUniformBlockOptimization // Uncomment to enable uniform block optimization for faster rendering
#ifdef useUniformBlockOptimization
  #define  BLOCK_OPTIMIZATION_SIZE 8.0
#endif
//#define useConvergenceThreshold // Uncomment to enable convergence checking, this should accelerate the calculations
#ifdef useConvergenceThreshold
  #define  CONVERGENCE_THRESHOLD 12.0
#endif

/**
 * Detects optimal thread count for CPU-intensive tasks
 * @return Number of CPU cores, capped at 16 threads maximum
 */


#endif // CONSTANTS_H

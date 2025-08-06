#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define COLOR_DEPTH 8

#define MANDELBROTPOINT_VALUE 255
#define COLOR_COUNT 255
#define DEFAULT_MAXITERATIONS 50

#define DEFAULT_THREAD_COUNT getOptimalThreadCount()
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

/*
#ifndef _bool
    #define _bool
    typedef unsigned char bool;
    #define true 255
    #define false 0
#endif
*/
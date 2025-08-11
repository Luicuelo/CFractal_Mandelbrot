#ifndef TYPES_H
#define TYPES_H

// Common type definitions used across multiple modules

typedef struct _renderFractalInternalParams{
  int current_block_size;
  int threadId;
} RenderFractalInternalParams;

typedef struct _point {
  int x;
  int y;
  int blockSize;
} Point;

#endif // TYPES_H
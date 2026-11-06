#ifndef MAIN_H
#define MAIN_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "common.h"

void onRepaint(void);
void onClearMessageQueue(void);

struct threadpool_t;

extern HWND main_window_handle;
extern struct threadpool_t *thread_pool;

#endif // MAIN_H

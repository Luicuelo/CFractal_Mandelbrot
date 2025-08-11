#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "common.h"

void onRepaint(void);
void onClearMessageQueue(void);

HWND main_window_handle;
struct threadpool_t *thread_pool;
HINSTANCE instance;

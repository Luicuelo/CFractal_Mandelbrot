#include "constants.h"
#include <windows.h>

void onRepaint(void);
void onClearMessageQueue(void);
void updateStatusBar(LPSTR statusText, WORD partNumber, WORD displayFlags);

HWND main_window_handle;
struct threadpool_t *thread_pool;
HINSTANCE instance;

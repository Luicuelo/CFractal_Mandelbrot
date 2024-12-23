#include "constantes.h"
#include <windows.h>

void onRepaint(void); // Cambiado de repinta
void onClearMessageQueue(void); // Cambiado de vaciaCola
extern HWND main_window_handle;  // Main window handle
extern struct threadpool_t *thread_pool;

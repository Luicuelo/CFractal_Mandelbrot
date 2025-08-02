#include "constantes.h"
#include <windows.h>

void onRepaint(void);
void onClearMessageQueue(void);
void updateStatusBar(LPSTR lpszStatusString, WORD partNumber, WORD displayFlags);

extern HWND main_window_handle;
extern struct threadpool_t *thread_pool;
extern HINSTANCE instancia;

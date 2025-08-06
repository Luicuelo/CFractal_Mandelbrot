#include "utils.h"

/**
 * Prints debug messages to Windows debug console (visible in debugger/DebugView)
 * @param format Printf-style format string
 * @param ... Variable arguments for format string
 */
void DebugPrint(const char *format, ...) {
    char debug_message_buffer[1024];
    va_list args;

    if (format == NULL) return;

    va_start(args, format);
    int result = vsnprintf(debug_message_buffer, sizeof(debug_message_buffer) - 1, format, args);
    va_end(args);

    debug_message_buffer[sizeof(debug_message_buffer) - 1] = '\0';

    if (result >= 0) {
        OutputDebugString(debug_message_buffer);
    }
}

/**
 * Detects optimal thread count for CPU-intensive tasks
 * @return Number of CPU cores, capped at 16 threads maximum
 */
int getOptimalThreadCount(void) {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int cores = (int)sysinfo.dwNumberOfProcessors;
    
    if (cores <= 0) return 4;  // Fallback for detection failure
    return (cores > 16) ? 16 : cores;  // Cap at 16 to avoid thread overhead
}
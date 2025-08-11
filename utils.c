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

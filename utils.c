#include "utils.h"

void DebugPrint (const char *format, ...) {
  char buffer[1024];
  va_list args;
  va_start (args, format);
  vsnprintf (buffer, sizeof (buffer), format, args);
  va_end (args);
  OutputDebugString (buffer);
}
#include "utils.h"

// Función para imprimir mensajes de depuración en la consola de depuración de Windows.
void DebugPrint(const char *format, ...) {
    char debug_message_buffer[1024]; // Búfer para almacenar el mensaje de depuración.
    va_list args;

    // Inicializar la lista de argumentos variables.
    va_start(args, format);

    // Formatear el mensaje con los argumentos proporcionados.
    vsnprintf(debug_message_buffer, sizeof(debug_message_buffer), format, args);

    // Finalizar el uso de la lista de argumentos variables.
    va_end(args);

    // Enviar el mensaje a la consola de depuración de Windows.
    OutputDebugString(debug_message_buffer);
}
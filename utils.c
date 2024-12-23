#include "utils.h"

// Función para imprimir mensajes de depuración en la consola de depuración de Windows.
void DebugPrint(const char *format, ...) {
    char debug_message_buffer[1024]; // Búfer para almacenar el mensaje de depuración.
    va_list args;

    if (format == NULL) return;

    // Inicializar la lista de argumentos variables.
    va_start(args, format);

    // Formatear el mensaje con los argumentos proporcionados con límite de tamaño.
    int result = vsnprintf(debug_message_buffer, sizeof(debug_message_buffer) - 1, format, args);
    
    // Finalizar el uso de la lista de argumentos variables.
    va_end(args);

    // Asegurar terminación nula
    debug_message_buffer[sizeof(debug_message_buffer) - 1] = '\0';

    // Verificar si el formateo fue exitoso
    if (result >= 0) {
        // Enviar el mensaje a la consola de depuración de Windows.
        OutputDebugString(debug_message_buffer);
    }
}
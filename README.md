# Pfractal_2

Explorador de los conjuntos de Mandelbrot y Julia para Windows, escrito en C con la API Win32 y sin frameworks externos.

## Características

- Ventana de 1000×1000 con framebuffer DIB de 8 bits.
- Render progresivo por bloques (de 256×256 hasta 1×1 píxel).
- Cálculo multihilo mediante un pool de hilos (`threadpool.c` + `tinycthread`).
- Zoom por arrastre reutilizando las iteraciones ya calculadas.
- Iteraciones adaptativas según la profundidad del zoom.
- Optimización de bloques uniformes.
- Guardado de la vista actual como BMP.

## Controles

| Acción | Control |
| --- | --- |
| Zoom a una zona | Arrastrar con el botón izquierdo |
| Cancelar la selección | Botón derecho o `Esc` |
| Ciclo de color | `T` |
| Invertir colores | `I` |
| Zoom automático siguiendo el ratón | `Z` |
| Modo Julia siguiendo el ratón | `J` |
| Nueva vista (Mandelbrot) | Menú `Archivo > Nuevo` |
| Guardar BMP | Menú `Archivo > Guardar` |

## Compilar

### Visual Studio

Abrir `Pfractal_2.sln` y compilar `Release|x64` (o `Debug|x64`). También están disponibles las configuraciones `Win32`.

### Línea de comandos

Desde un **Developer Command Prompt** de Visual Studio:

```bat
Compila.bat
```

El script compila `menu.rc` y todos los `.c` en `intermediate\`, y genera `Pfractal.exe`.

## Estructura

- `main.c` — ventana, bucle de mensajes, menú y teclado.
- `fractal_calc.c` — cálculo del fractal, zoom, animaciones y estado de interacción.
- `dib.c` — DIB, paleta, pintado y guardado BMP.
- `threadpool.c`, `source/tinycthread.c` — pool de hilos.
- `stsbar.c`, `utils.c` — barra de estado y utilidades.
- `menu.rc` — menú, diálogo About, iconos y bitmap.

## Licencia

MIT. `tinycthread` y `threadpool` conservan sus licencias BSD originales.
